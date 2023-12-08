/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "networkshare_service.h"

#include "net_event_report.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "netmanager_base_permission.h"
#include "netmgr_ext_log_wrapper.h"
#include "networkshare_constants.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT_NETSHARE =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetworkShareService>::GetInstance().get());

NetworkShareService::NetworkShareService() : SystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, true) {}

NetworkShareService::~NetworkShareService(){};

void NetworkShareService::OnStart()
{
    if (state_ == STATE_RUNNING) {
        NETMGR_EXT_LOG_D("OnStart Service state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_EXT_LOG_E("OnStart init failed");
        EventInfo eventInfo;
        eventInfo.operatorType = static_cast<int32_t>(NetworkShareEventOperator::OPERATION_START_SA);
        eventInfo.errorType = static_cast<int32_t>(NetworkShareEventErrorType::ERROR_START_SA);
        eventInfo.errorMsg = "Start Network Share Service failed";
        NetEventReport::SendSetupFaultEvent(eventInfo);
        return;
    }
    state_ = STATE_RUNNING;
    NETMGR_EXT_LOG_I("OnStart successful");
}

void NetworkShareService::OnStop()
{
    NetworkShareTracker::GetInstance().Uninit();
    state_ = STATE_STOPPED;
    registerToService_ = false;
    NETMGR_EXT_LOG_I("OnStop successful");
}

int32_t NetworkShareService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETMGR_EXT_LOG_I("Start Dump, fd: %{public}d", fd);
    std::string result;
    GetDumpMessage(result);
    NETMGR_EXT_LOG_I("Dump content: %{public}s", result.c_str());
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    return ret < 0 ? NETWORKSHARE_ERROR_INTERNAL_ERROR : NETMANAGER_EXT_SUCCESS;
}

bool NetworkShareService::Init()
{
    if (!REGISTER_LOCAL_RESULT_NETSHARE) {
        NETMGR_EXT_LOG_E("Register to local sa manager failed");
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<NetworkShareService>::GetInstance().get())) {
            NETMGR_EXT_LOG_E("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }

    AddSystemAbilityListener(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);

    return NetworkShareTracker::GetInstance().Init();
}

void NetworkShareService::GetDumpMessage(std::string &message)
{
    message.append("Net Sharing Info:\n");
    int32_t supported = NETWORKSHARE_IS_UNSUPPORTED;
    NetworkShareTracker::GetInstance().IsNetworkSharingSupported(supported);
    std::string surpportContent = supported == NETWORKSHARE_IS_SUPPORTED ? "surpported" : "not surpported";
    message.append("\tIs Sharing Supported: " + surpportContent + "\n");
    int32_t sharingStatus = NETWORKSHARE_IS_UNSHARING;
    NetworkShareTracker::GetInstance().IsSharing(sharingStatus);
    std::string sharingState = sharingStatus ? "is sharing" : "not sharing";
    message.append("\tSharing State: " + sharingState + "\n");
    if (sharingStatus) {
        std::string sharingType;
        GetSharingType(SharingIfaceType::SHARING_WIFI, "wifi;", sharingType);
        GetSharingType(SharingIfaceType::SHARING_USB, "usb;", sharingType);
        GetSharingType(SharingIfaceType::SHARING_BLUETOOTH, "bluetooth;", sharingType);
        message.append("\tSharing Types: " + sharingType + "\n");
    }

    std::string wifiShareRegexs;
    GetShareRegexsContent(SharingIfaceType::SHARING_WIFI, wifiShareRegexs);
    message.append("\tUsb Regexs: " + wifiShareRegexs + "\n");
    std::string usbShareRegexs;
    GetShareRegexsContent(SharingIfaceType::SHARING_USB, usbShareRegexs);
    message.append("\tWifi Regexs: " + usbShareRegexs + "\n");
    std::string btpanShareRegexs;
    GetShareRegexsContent(SharingIfaceType::SHARING_BLUETOOTH, btpanShareRegexs);
    message.append("\tBluetooth Regexs: " + btpanShareRegexs + "\n");
}

void NetworkShareService::GetSharingType(const SharingIfaceType &type, const std::string &typeContent,
                                         std::string &sharingType)
{
    SharingIfaceState state;
    NetworkShareTracker::GetInstance().GetSharingState(type, state);
    if (state == SharingIfaceState::SHARING_NIC_SERVING) {
        sharingType += typeContent;
    }
}

void NetworkShareService::GetShareRegexsContent(const SharingIfaceType &type, std::string &shareRegexsContent)
{
    std::vector<std::string> regexs;
    NetworkShareTracker::GetInstance().GetSharableRegexs(type, regexs);
    for_each(regexs.begin(), regexs.end(),
             [&shareRegexsContent](const std::string &regex) { shareRegexsContent += regex + ";"; });
}

int32_t NetworkShareService::IsNetworkSharingSupported(int32_t &supported)
{
    NETMGR_EXT_LOG_I("NetworkSharing IsNetworkSharingSupported");
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().IsNetworkSharingSupported(supported);
}

int32_t NetworkShareService::IsSharing(int32_t &sharingStatus)
{
    NETMGR_EXT_LOG_I("NetworkSharing IsSharing");
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().IsSharing(sharingStatus);
}

int32_t NetworkShareService::UpdateDataSharingType(const SharingIfaceType &type, bool isOpen)
{
    int32_t ret = NETMANAGER_ERROR;
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    switch (type) {
        case SharingIfaceType::SHARING_WIFI:
            {
                Uri uri(SHARING_WIFI_URI);
                ret = dataShareHelperUtils->Update(uri, KEY_SHARING_WIFI, std::to_string(isOpen));
            }
            break;
        case SharingIfaceType::SHARING_USB:
            {
                Uri uri(SHARING_USB_URI);
                ret = dataShareHelperUtils->Update(uri, KEY_SHARING_USB, std::to_string(isOpen));
            }
            break;
        case SharingIfaceType::SHARING_BLUETOOTH:
            {
                Uri uri(SHARING_BLUETOOTH_URI);
                ret = dataShareHelperUtils->Update(uri, KEY_SHARING_BLUETOOTH, std::to_string(isOpen));
            }
            break;
        default:
            break;
    }
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("Update sharetype:[%{public}d]to datashare failed", static_cast<int32_t>(type));
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    return ret;
}

int32_t NetworkShareService::StartNetworkSharing(const SharingIfaceType &type)
{
    NETMGR_EXT_LOG_I("NetworkSharing start sharing,type is %{public}d", type);
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    int32_t ret = NetworkShareTracker::GetInstance().StartNetworkSharing(type);
    if (ret == NETMANAGER_EXT_SUCCESS) {
        ret = UpdateDataSharingType(type, true);
    }
    return ret;
}

int32_t NetworkShareService::StopNetworkSharing(const SharingIfaceType &type)
{
    NETMGR_EXT_LOG_I("NetworkSharing start sharing,type is %{public}d", type);
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    int32_t ret = NetworkShareTracker::GetInstance().StopNetworkSharing(type);
    if (ret == NETMANAGER_EXT_SUCCESS) {
        ret = UpdateDataSharingType(type, false);
    }

    return ret;
}

int32_t NetworkShareService::RegisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    NETMGR_EXT_LOG_I("NetworkSharing Register Sharing Event.");
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().RegisterSharingEvent(callback);
}

int32_t NetworkShareService::UnregisterSharingEvent(sptr<ISharingEventCallback> callback)
{
    NETMGR_EXT_LOG_I("NetworkSharing UnRegister Sharing Event.");
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().UnregisterSharingEvent(callback);
}

int32_t NetworkShareService::GetSharableRegexs(SharingIfaceType type, std::vector<std::string> &ifaceRegexs)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetSharableRegexs(type, ifaceRegexs);
}

int32_t NetworkShareService::GetSharingState(SharingIfaceType type, SharingIfaceState &state)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetSharingState(type, state);
}

int32_t NetworkShareService::GetNetSharingIfaces(const SharingIfaceState &state, std::vector<std::string> &ifaces)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetNetSharingIfaces(state, ifaces);
}

int32_t NetworkShareService::GetStatsRxBytes(int32_t &bytes)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(TrafficType::TRAFFIC_RX, bytes);
}

int32_t NetworkShareService::GetStatsTxBytes(int32_t &bytes)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(TrafficType::TRAFFIC_TX, bytes);
}

int32_t NetworkShareService::GetStatsTotalBytes(int32_t &bytes)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    }
    return NetworkShareTracker::GetInstance().GetSharedSubSMTraffic(TrafficType::TRAFFIC_ALL, bytes);
}

void NetworkShareService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_D("NetworkShareService::OnAddSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        if (hasSARemoved_) {
            OnNetSysRestart();
            hasSARemoved_ = false;
        }
    }
}

void NetworkShareService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_EXT_LOG_D("NetworkShareService::OnRemoveSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        hasSARemoved_ = true;
    }
}

void NetworkShareService::OnNetSysRestart()
{
    NETMGR_EXT_LOG_I("NetworkShareService::OnNetSysRestart");
    NetworkShareTracker::GetInstance().RestartResume();
}
} // namespace NetManagerStandard
} // namespace OHOS
