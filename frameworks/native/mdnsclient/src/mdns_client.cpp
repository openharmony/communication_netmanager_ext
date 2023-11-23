/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "mdns_client.h"

#include <condition_variable>
#include <mutex>
#include <unistd.h>
#include <thread>

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "netmgr_ext_log_wrapper.h"
#include "system_ability_definition.h"

#include "mdns_common.h"
#include "mdns_client_resume.h"

namespace OHOS {
namespace NetManagerStandard {

std::mutex g_loadMutex;
std::condition_variable g_cv;

static constexpr uint32_t MAX_GET_SERVICE_COUNT = 30;
constexpr uint32_t WAIT_FOR_SERVICE_TIME_S = 1;
constexpr uint32_t WAIT_FOR_SERVICE_READY_TIME_S = 20;

void OnDemandLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    NETMGR_EXT_LOG_D("OnLoadSystemAbilitySuccess systemAbilityId: [%{public}d]", systemAbilityId);
    g_loadMutex.lock();
    remoteObject_ = remoteObject;
    g_loadMutex.unlock();
    g_cv.notify_one();
}

void OnDemandLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    NETMGR_EXT_LOG_D("OnLoadSystemAbilityFail: [%{public}d]", systemAbilityId);
    g_cv.notify_one();
}

const sptr<IRemoteObject> &OnDemandLoadCallback::GetRemoteObject() const
{
    return remoteObject_;
}

MDnsClient::MDnsClient() : mdnsService_(nullptr), loadCallback_(nullptr) {}

MDnsClient::~MDnsClient() = default;

int32_t MDnsClient::RegisterService(const MDnsServiceInfo &serviceInfo, const sptr<IRegistrationCallback> &cb)
{
    if (!(IsNameValid(serviceInfo.name) && IsTypeValid(serviceInfo.type) && IsPortValid(serviceInfo.port))) {
        NETMGR_EXT_LOG_E("RegisterService arguments are not valid");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("callback is null");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    sptr<IMDnsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("proxy is nullptr");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    int32_t ret = proxy->RegisterService(serviceInfo, cb);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("RegisterService return code: [%{public}d]", ret);
    }
    return ret;
}

int32_t MDnsClient::UnRegisterService(const sptr<IRegistrationCallback> &cb)
{
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("callback is null");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    sptr<IMDnsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("MDnsClient::RemoveLocalService proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    int32_t ret = proxy->UnRegisterService(cb);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("UnRegisterService return code: [%{public}d]", ret);
    }
    return ret;
}

int32_t MDnsClient::StartDiscoverService(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb)
{
    if (!IsTypeValid(serviceType)) {
        NETMGR_EXT_LOG_E("arguments are not valid, [%{public}s]", serviceType.c_str());
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("callback is null");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    sptr<IMDnsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("MDnsClient::StartDiscoverService proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    int32_t ret = proxy->StartDiscoverService(serviceType, cb);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("StartDiscoverService return code: [%{public}d]", ret);
    }
    return ret;
}

int32_t MDnsClient::StopDiscoverService(const sptr<IDiscoveryCallback> &cb)
{
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("callback is null");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    sptr<IMDnsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("MDnsClient::StopSearchingMDNS proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    int32_t ret = proxy->StopDiscoverService(cb);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("StopDiscoverService return code: [%{public}d]", ret);
    }
    return ret;
}

int32_t MDnsClient::ResolveService(const MDnsServiceInfo &serviceInfo, const sptr<IResolveCallback> &cb)
{
    if (!(IsNameValid(serviceInfo.name) && IsTypeValid(serviceInfo.type))) {
        NETMGR_EXT_LOG_E("arguments are not valid");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    if (cb == nullptr) {
        NETMGR_EXT_LOG_E("callback is null");
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    sptr<IMDnsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_EXT_LOG_E("MDnsClient::ResolveService proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    int32_t ret = proxy->ResolveService(serviceInfo, cb);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("ResolveService return code: [%{public}d]", ret);
    }
    return ret;
}

sptr<IRemoteObject> MDnsClient::LoadSaOnDemand()
{
    if (loadCallback_->GetRemoteObject() == nullptr) {
        sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            NETMGR_EXT_LOG_E("GetSystemAbilityManager failed");
            return nullptr;
        }
        int32_t result = sam->LoadSystemAbility(COMM_MDNS_MANAGER_SYS_ABILITY_ID, loadCallback_);
        if (result != ERR_OK) {
            NETMGR_EXT_LOG_E("LoadSystemAbility failed : [%{public}d]", result);
            return nullptr;
        }
        std::unique_lock<std::mutex> lk(g_loadMutex);
        if (!g_cv.wait_for(lk, std::chrono::seconds(LOAD_SA_TIMEOUT),
                           [this]() { return loadCallback_->GetRemoteObject() != nullptr; })) {
            NETMGR_EXT_LOG_E("LoadSystemAbility timeout");
            lk.unlock();
            return nullptr;
        }
        lk.unlock();
    }
    return loadCallback_->GetRemoteObject();
}

sptr<IMDnsService> MDnsClient::GetProxy()
{
    std::lock_guard lock(mutex_);
    if (mdnsService_ != nullptr) {
        NETMGR_EXT_LOG_D("get proxy is ok");
        return mdnsService_;
    }
    loadCallback_ = new (std::nothrow) OnDemandLoadCallback();
    if (loadCallback_ == nullptr) {
        NETMGR_EXT_LOG_E("loadCallback_ is nullptr");
        return nullptr;
    }
    sptr<IRemoteObject> remote = LoadSaOnDemand();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service failed");
        return nullptr;
    }
    deathRecipient_ = new (std::nothrow) MDnsDeathRecipient(*this);
    if (deathRecipient_ == nullptr) {
        NETMGR_EXT_LOG_E("deathRecipient_ is nullptr");
        return nullptr;
    }
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_EXT_LOG_E("add death recipient failed");
        return nullptr;
    }
    mdnsService_ = iface_cast<IMDnsService>(remote);
    if (mdnsService_ == nullptr) {
        NETMGR_EXT_LOG_E("get Remote service proxy failed");
        return nullptr;
    }
    return mdnsService_;
}

void MDnsClient::RestartResume()
{
    NETMGR_EXT_LOG_I("MDnsClient::RestartResume");
    std::thread t([this]() {
        uint32_t count = 0;
        while (GetProxy() == nullptr && count < MAX_GET_SERVICE_COUNT) {
            std::this_thread::sleep_for(std::chrono::seconds(WAIT_FOR_SERVICE_TIME_S));
            count++;
        }
        auto proxy = GetProxy();
        NETMGR_EXT_LOG_W("Get proxy %{public}s, count: %{public}u", proxy == nullptr ? "failed" : "success", count);
        if (proxy != nullptr) {
            std::this_thread::sleep_for(std::chrono::seconds(WAIT_FOR_SERVICE_READY_TIME_S));
            NETMGR_EXT_LOG_I("resume RegisterService");
            for (const auto& [key, value]: *MDnsClientResume::GetInstance().GetRegisterServiceMap()) {
                RegisterService(value, key);
            }
            NETMGR_EXT_LOG_I("resume RegisterService ok");

            NETMGR_EXT_LOG_D("resume StartDiscoverService");
            for (const auto& [key, value]: *MDnsClientResume::GetInstance().GetStartDiscoverServiceMap()) {
                StartDiscoverService(value, key);
            }

            NETMGR_EXT_LOG_I("resume StartDiscoverService ok");
        }
    });
    std::string threadName = "mdnsGetProxy";
    pthread_setname_np(t.native_handle(), threadName.c_str());
    t.detach();
}

void MDnsClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_EXT_LOG_D("on remote died");
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("remote object is nullptr");
        return;
    }
    std::lock_guard lock(mutex_);
    if (mdnsService_ == nullptr) {
        NETMGR_EXT_LOG_E("mdnsService_ is nullptr");
        return;
    }
    sptr<IRemoteObject> local = mdnsService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_EXT_LOG_E("proxy and stub is not same remote object");
        return;
    }
    local->RemoveDeathRecipient(deathRecipient_);
    mdnsService_ = nullptr;

    RestartResume();
}
} // namespace NetManagerStandard
} // namespace OHOS