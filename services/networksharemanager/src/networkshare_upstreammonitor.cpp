/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "networkshare_upstreammonitor.h"

#include "netmgr_ext_log_wrapper.h"
#include "networkshare_constants.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr const char *ERROR_MSG_HAS_NOT_UPSTREAM = "Has not Upstream Network";
static constexpr const char *ERROR_MSG_UPSTREAM_ERROR = "Get Upstream Network is Error";
static constexpr const int32_t INVALID_NETID = -1;

NetworkShareUpstreamMonitor::NetConnectionCallback::NetConnectionCallback(
    const std::shared_ptr<NetworkShareUpstreamMonitor> &networkmonitor, int32_t callbackType)
    : NetworkMonitor_(networkmonitor)
{
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetAvailable(sptr<NetHandle> &netHandle)
{
    if (NetworkMonitor_) {
        NetworkMonitor_->HandleNetAvailable(netHandle);
    }
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetCapabilitiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetAllCapabilities> &netAllCap)
{
    if (NetworkMonitor_) {
        NetworkMonitor_->HandleNetCapabilitiesChange(netHandle, netAllCap);
    }
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetLinkInfo> &info)
{
    if (NetworkMonitor_) {
        NetworkMonitor_->HandleConnectionPropertiesChange(netHandle, info);
    }
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetLost(sptr<NetHandle> &netHandle)
{
    if (NetworkMonitor_) {
        NetworkMonitor_->HandleNetLost(netHandle);
    }
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetUnavailable()
{
    return NETWORKSHARE_SUCCESS;
}

int32_t NetworkShareUpstreamMonitor::NetConnectionCallback::NetBlockStatusChange(sptr<NetHandle> &netHandle,
                                                                                 bool blocked)
{
    return NETWORKSHARE_SUCCESS;
}

NetworkShareUpstreamMonitor::NetworkShareUpstreamMonitor() : defaultNetworkId_(INVALID_NETID) {}

NetworkShareUpstreamMonitor::~NetworkShareUpstreamMonitor()
{
    {
        std::lock_guard lock(networkMapMutex_);
        networkMaps_.clear();
    }
    auto netManager = DelayedSingleton<NetConnClient>::GetInstance();
    if (netManager != nullptr) {
        netManager->UnregisterNetConnCallback(defaultNetworkCallback_);
    }
}

void NetworkShareUpstreamMonitor::SetOptionData(int what, std::weak_ptr<MonitorEventHandler> &handler)
{
    eventId_ = what;
    eventHandler_ = handler;
}

void NetworkShareUpstreamMonitor::ListenDefaultNetwork()
{
    auto netManager = DelayedSingleton<NetConnClient>::GetInstance();
    if (netManager == nullptr) {
        NETMGR_EXT_LOG_E("NetConnClient is null.");
        return;
    }
    std::shared_ptr<NetworkShareUpstreamMonitor> networkmonitor = shared_from_this();
    defaultNetworkCallback_ =
        std::make_unique<NetConnectionCallback>(networkmonitor, CALLBACK_DEFAULT_INTERNET_NETWORK).release();
    int32_t result = netManager->RegisterNetConnCallback(defaultNetworkCallback_);
    if (result == NETWORKSHARE_SUCCESS) {
        NETMGR_EXT_LOG_I("Register defaultNetworkCallback_ successful");
    } else {
        NETMGR_EXT_LOG_E("Register defaultNetworkCallback_ failed");
    }
}

void NetworkShareUpstreamMonitor::RegisterUpstreamChangedCallback(
    const std::shared_ptr<NotifyUpstreamCallback> &callback)
{
    notifyUpstreamCallback_ = callback;
}

bool NetworkShareUpstreamMonitor::GetCurrentGoodUpstream(std::shared_ptr<UpstreamNetworkInfo> &upstreamNetwork)
{
    auto netManager = DelayedSingleton<NetConnClient>::GetInstance();
    if (upstreamNetwork == nullptr || netManager == nullptr) {
        NETMGR_EXT_LOG_E("NetConnClient or upstreamNetInfo is null.");
        return false;
    }
    bool hasDefaultNet = true;
    int32_t result = netManager->HasDefaultNet(hasDefaultNet);
    if (result != ERR_NONE || !hasDefaultNet) {
        NetworkShareHisysEvent::GetInstance().SendFaultEvent(
            NetworkShareEventOperator::OPERATION_GET_UPSTREAM, NetworkShareEventErrorType::ERROR_GET_UPSTREAM,
            ERROR_MSG_HAS_NOT_UPSTREAM, NetworkShareEventType::SETUP_EVENT);
        NETMGR_EXT_LOG_E("NetConn hasDefaultNet error[%{public}d].", result);
        return false;
    }

    netManager->GetDefaultNet(*(upstreamNetwork->netHandle_));
    int32_t currentNetId = upstreamNetwork->netHandle_->GetNetId();
    NETMGR_EXT_LOG_I("NetConn get defaultNet id[%{public}d].", currentNetId);
    if (currentNetId <= INVALID_NETID) {
        NetworkShareHisysEvent::GetInstance().SendFaultEvent(
            NetworkShareEventOperator::OPERATION_GET_UPSTREAM, NetworkShareEventErrorType::ERROR_GET_UPSTREAM,
            ERROR_MSG_UPSTREAM_ERROR, NetworkShareEventType::SETUP_EVENT);
        NETMGR_EXT_LOG_E("NetConn get defaultNet id is error.");
        return false;
    }

    {
        std::lock_guard lock(networkMapMutex_);
        auto iter = networkMaps_.find(currentNetId);
        if (iter == networkMaps_.end()) {
            return false;
        }
        upstreamNetwork = iter->second;
    }
    defaultNetworkId_ = currentNetId;
    return true;
}

void NetworkShareUpstreamMonitor::NotifyMainStateMachine(int which, const std::shared_ptr<UpstreamNetworkInfo> &obj)
{
    if (notifyUpstreamCallback_ == nullptr) {
        NETMGR_EXT_LOG_E("notifyUpstreamCallback is null.");
    } else {
        notifyUpstreamCallback_->OnUpstreamStateChanged(eventId_, which, 0, obj);
    }
}

void NetworkShareUpstreamMonitor::NotifyMainStateMachine(int which)
{
    if (notifyUpstreamCallback_ == nullptr) {
        NETMGR_EXT_LOG_E("notifyUpstreamCallback is null.");
    } else {
        notifyUpstreamCallback_->OnUpstreamStateChanged(eventId_, which);
    }
}

void NetworkShareUpstreamMonitor::HandleNetAvailable(sptr<NetHandle> &netHandle)
{
    if (netHandle == nullptr) {
        return;
    }
    std::lock_guard lock(networkMapMutex_);
    auto iter = networkMaps_.find(netHandle->GetNetId());
    if (iter == networkMaps_.end()) {
        NETMGR_EXT_LOG_I("netHandle[%{public}d] is new.", netHandle->GetNetId());
        sptr<NetAllCapabilities> netCap = new NetAllCapabilities();
        sptr<NetLinkInfo> linkInfo = new NetLinkInfo();
        std::shared_ptr<UpstreamNetworkInfo> network =
            std::make_shared<UpstreamNetworkInfo>(netHandle, netCap, linkInfo);
        networkMaps_.insert(std::make_pair(netHandle->GetNetId(), network));
    }
}

void NetworkShareUpstreamMonitor::HandleNetCapabilitiesChange(sptr<NetHandle> &netHandle,
                                                              const sptr<NetAllCapabilities> &newNetAllCap)
{
    if (netHandle == nullptr) {
        return;
    }
    std::lock_guard lock(networkMapMutex_);
    auto iter = networkMaps_.find(netHandle->GetNetId());
    if (iter != networkMaps_.end()) {
        if ((iter->second)->netAllCap_ != newNetAllCap) {
            NETMGR_EXT_LOG_I("netHandle[%{public}d] Capabilities Changed.", netHandle->GetNetId());
            *((iter->second)->netAllCap_) = *(newNetAllCap);
        }
    }
}

void NetworkShareUpstreamMonitor::HandleConnectionPropertiesChange(sptr<NetHandle> &netHandle,
                                                                   const sptr<NetLinkInfo> &newNetLinkInfo)
{
    if (netHandle == nullptr || newNetLinkInfo == nullptr) {
        return;
    }
    std::shared_ptr<UpstreamNetworkInfo> currentNetwork = nullptr;
    {
        std::lock_guard lock(networkMapMutex_);
        auto iter = networkMaps_.find(netHandle->GetNetId());
        if (iter != networkMaps_.end()) {
            if ((iter->second)->netLinkPro_ != newNetLinkInfo) {
                currentNetwork = (iter->second);
                NETMGR_EXT_LOG_I("netHandle[%{public}d] ConnectionProperties Changed.", netHandle->GetNetId());
                currentNetwork->netLinkPro_->ifaceName_ = newNetLinkInfo->ifaceName_;
            }
        }
    }

    if (currentNetwork != nullptr && defaultNetworkId_ != netHandle->GetNetId()) {
        if (defaultNetworkId_ == INVALID_NETID) {
            NETMGR_EXT_LOG_I("Send MainSM ON_LINKPROPERTY event with netHandle[%{public}d].", netHandle->GetNetId());
            NotifyMainStateMachine(EVENT_UPSTREAM_CALLBACK_ON_LINKPROPERTIES, currentNetwork);
        } else {
            NETMGR_EXT_LOG_I("Send MainSM ON_SWITCH event with netHandle[%{public}d].", netHandle->GetNetId());
            NotifyMainStateMachine(EVENT_UPSTREAM_CALLBACK_DEFAULT_SWITCHED, currentNetwork);
        }
        defaultNetworkId_ = netHandle->GetNetId();
    }
}

void NetworkShareUpstreamMonitor::HandleNetLost(sptr<NetHandle> &netHandle)
{
    if (netHandle == nullptr) {
        return;
    }
    std::shared_ptr<UpstreamNetworkInfo> currentNetInfo = nullptr;
    {
        std::lock_guard lock(networkMapMutex_);
        auto iter = networkMaps_.find(netHandle->GetNetId());
        if (iter != networkMaps_.end()) {
            NETMGR_EXT_LOG_I("netHandle[%{public}d] is lost, defaultNetId[%{public}d].",
                             netHandle->GetNetId(), defaultNetworkId_);
            currentNetInfo = iter->second;
        }
    }

    if (currentNetInfo != nullptr && defaultNetworkId_ == netHandle->GetNetId()) {
        NETMGR_EXT_LOG_I("Send MainSM ON_LOST event with netHandle[%{public}d].", defaultNetworkId_);
        NotifyMainStateMachine(EVENT_UPSTREAM_CALLBACK_ON_LOST, currentNetInfo);
        defaultNetworkId_ = INVALID_NETID;
    }
}

NetworkShareUpstreamMonitor::MonitorEventHandler::MonitorEventHandler(
    const std::shared_ptr<NetworkShareUpstreamMonitor> &networkmonitor,
    const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner), networkMonitor_(networkmonitor)
{
}
} // namespace NetManagerStandard
} // namespace OHOS
