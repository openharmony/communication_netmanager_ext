/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <memory>

#include "networkvpn_service_stub.h"
#include "net_manager_constants.h"
#include "netmanager_base_permission.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

NetworkVpnServiceStub::NetworkVpnServiceStub()
{
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_PREPARE] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplyPrepare};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_START_VPN] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplySetUpVpn};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_PROTECT] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplyProtect};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_STOP_VPN] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplyDestroyVpn};
#ifdef SUPPORT_SYSVPN
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_SETUP_SYS_VPN] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplySetUpSysVpn};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_ADD_SYS_VPN_CONFIG] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplyAddSysVpnConfig};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_DELETE_SYS_VPN_CONFIG] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplyDeleteSysVpnConfig};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_GET_SYS_VPN_CONFIG_LIST] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplyGetSysVpnConfigList};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_GET_SYS_VPN_CONFIG] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplyGetSysVpnConfig};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_GET_CONNECTED_SYS_VPN_CONFIG] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplyGetConnectedSysVpnConfig};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_NOTIFY_CONNECT_STAGE] = {
        "", &NetworkVpnServiceStub::ReplyNotifyConnectStage};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_GET_SYS_VPN_CERT_URI] = {
        "", &NetworkVpnServiceStub::ReplyGetSysVpnCertUri};
#endif // SUPPORT_SYSVPN
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_REGISTER_EVENT_CALLBACK] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplyRegisterVpnEvent};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_UNREGISTER_EVENT_CALLBACK] = {
        Permission::MANAGE_VPN, &NetworkVpnServiceStub::ReplyUnregisterVpnEvent};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_CREATE_VPN_CONNECTION] = {
        "", &NetworkVpnServiceStub::ReplyCreateVpnConnection};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_FACTORYRESET_VPN] = {
        "", &NetworkVpnServiceStub::ReplyFactoryResetVpn};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_CREATE_VPN_CONNECTION_EXT] = {
        "", &NetworkVpnServiceStub::ReplyCreateVpnConnection};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_START_VPN_EXT] = {
        "", &NetworkVpnServiceStub::ReplySetUpVpn};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_PROTECT_EXT] = {
        "", &NetworkVpnServiceStub::ReplyProtect};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_STOP_VPN_EXT] = {
        "", &NetworkVpnServiceStub::ReplyDestroyVpn};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_REGISTER_BUNDLENAME] = {
        "", &NetworkVpnServiceStub::ReplyRegisterBundleName};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_GET_SELF_APP_NAME] = {
        "", &NetworkVpnServiceStub::ReplyGetSelfAppName};
    permissionAndFuncMap_[INetworkVpnService::MessageCode::CMD_SET_SELF_VPN_PID] = {
        "", &NetworkVpnServiceStub::ReplySetSelfVpnPid};
}

int32_t NetworkVpnServiceStub::CheckVpnPermission(std::string &strPermission)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_EXT_LOG_E("is not system call");
        return NETMANAGER_ERR_NOT_SYSTEM_CALL;
    }
    if (!strPermission.empty() && !NetManagerPermission::CheckPermission(strPermission)) {
        NETMGR_EXT_LOG_E("Permission denied permission: %{public}s", strPermission.c_str());
        return NETMANAGER_ERR_PERMISSION_DENIED;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetworkVpnServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                               MessageOption &option)
{
    if (NetworkVpnServiceStub::GetDescriptor() != data.ReadInterfaceToken()) {
        NETMGR_EXT_LOG_E("descriptor checked failed");
        return NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH;
    }
    auto itr = permissionAndFuncMap_.find(static_cast<INetworkVpnService::MessageCode>(code));
    if (itr != permissionAndFuncMap_.end()) {
        if (itr->first >= INetworkVpnService::MessageCode::CMD_START_VPN_EXT &&
                itr->first <= INetworkVpnService::MessageCode::CMD_GET_SELF_APP_NAME) {
            NETMGR_EXT_LOG_I("enter OnRemoteRequest code %{public}d:", code);
            auto serviceFunc = itr->second.serviceFunc;
            if (serviceFunc != nullptr) {
                return (this->*serviceFunc)(data, reply);
            }
        } else {
            NETMGR_EXT_LOG_I("enter OnRemoteRequest code %{public}d:", code);
            int32_t checkResult = CheckVpnPermission(itr->second.strPermission);
            if (checkResult != NETMANAGER_SUCCESS) {
                return checkResult;
            }
            auto serviceFunc = itr->second.serviceFunc;
            if (serviceFunc != nullptr) {
                return (this->*serviceFunc)(data, reply);
            }
        }
    }

    NETMGR_EXT_LOG_I("stub default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NetworkVpnServiceStub::ReplyPrepare(MessageParcel &data, MessageParcel &reply)
{
    bool isExist = false;
    bool isRun = false;
    std::string pkg;
    int32_t ret = Prepare(isExist, isRun, pkg);
    bool allOK = reply.WriteInt32(ret) && reply.WriteBool(isExist) && reply.WriteBool(isRun) && reply.WriteString(pkg);
    return allOK ? NETMANAGER_EXT_SUCCESS : NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
}

int32_t NetworkVpnServiceStub::ReplySetUpVpn(MessageParcel &data, MessageParcel &reply)
{
    sptr<VpnConfig> config = VpnConfig::Unmarshalling(data);
    if (config == nullptr) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }

    int32_t result = SetUpVpn(config);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyProtect(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = Protect();
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyDestroyVpn(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = DestroyVpn();
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

#ifdef SUPPORT_SYSVPN
int32_t NetworkVpnServiceStub::ReplySetUpSysVpn(MessageParcel &data, MessageParcel &reply)
{
    sptr<SysVpnConfig> config = new (std::nothrow) SysVpnConfig();
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("ReplySetUpSysVpn failed, config is null");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (!(data.ReadString(config->vpnId_) && data.ReadInt32(config->vpnType_))) {
        NETMGR_EXT_LOG_E("ReplySetUpSysVpn read data failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    NETMGR_EXT_LOG_I("ReplySetUpSysVpn id=%{public}s", config->vpnId_.c_str());
    int32_t result = SetUpVpn(config);
    if (!reply.WriteInt32(result)) {
        NETMGR_EXT_LOG_E("ReplySetUpSysVpn write reply failed");
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyAddSysVpnConfig(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkVpnServiceStub ReplyAddSysVpnConfig");
    sptr<SysVpnConfig> config = SysVpnConfig::Unmarshalling(data);
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("ReplyAddSysVpnConfig read data failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    int32_t result = AddSysVpnConfig(config);
    if (!reply.WriteInt32(result)) {
        NETMGR_EXT_LOG_E("ReplyAddSysVpnConfig write reply failed");
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyDeleteSysVpnConfig(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkVpnServiceStub ReplyDeleteSysVpnConfig");
    std::string vpnId;
    if (!data.ReadString(vpnId)) {
        NETMGR_EXT_LOG_E("ReplyDeleteSysVpnConfig read data failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    int32_t result = DeleteSysVpnConfig(vpnId);
    if (!reply.WriteInt32(result)) {
        NETMGR_EXT_LOG_E("ReplyDeleteSysVpnConfig write reply failed");
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyGetSysVpnConfigList(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkVpnServiceStub ReplyGetSysVpnConfigList");
    std::vector<SysVpnConfig> vpnList;
    int32_t result = GetSysVpnConfigList(vpnList);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("ReplyGetSysVpnConfigList failed, result=%{public}d", result);
        return result;
    }
    int32_t vpnListSize = static_cast<int32_t>(vpnList.size());
    if (!reply.WriteInt32(vpnListSize)) {
        NETMGR_EXT_LOG_E("ReplyGetSysVpnConfigList write reply failed");
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    for (auto &config : vpnList) {
        if (!(reply.WriteString(config.vpnId_) && reply.WriteString(config.vpnName_) &&
            reply.WriteInt32(config.vpnType_))) {
            NETMGR_EXT_LOG_E("ReplyGetSysVpnConfigList write reply failed");
            return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyGetSysVpnConfig(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkVpnServiceStub ReplyGetSysVpnConfig");
    std::string vpnId;
    if (!data.ReadString(vpnId)) {
        NETMGR_EXT_LOG_E("ReplyGetSysVpnConfig read data failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    sptr<SysVpnConfig> config = nullptr;
    int32_t result = GetSysVpnConfig(config, vpnId);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("ReplyGetSysVpnConfig failed, result=%{public}d", result);
        return result;
    }
    if (config != nullptr && !config->Marshalling(reply)) {
        NETMGR_EXT_LOG_E("ReplyGetSysVpnConfig write reply failed");
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyGetConnectedSysVpnConfig(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkVpnServiceStub ReplyGetConnectedSysVpnConfig");
    sptr<SysVpnConfig> config = nullptr;
    int32_t result = GetConnectedSysVpnConfig(config);
    if (result != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("ReplyGetConnectedSysVpnConfig failed, result=%{public}d", result);
        return result;
    }
    if (config != nullptr && !config->Marshalling(reply)) {
        NETMGR_EXT_LOG_E("ReplyGetConnectedSysVpnConfig write reply failed");
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyNotifyConnectStage(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkVpnServiceStub ReplyNotifyConnectStage");
    std::string stage;
    if (!data.ReadString(stage)) {
        NETMGR_EXT_LOG_E("ReplyNotifyConnectStage read data failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    int32_t result;
    if (!data.ReadInt32(result)) {
        NETMGR_EXT_LOG_E("ReplyNotifyConnectStage read data failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    int32_t ret = NotifyConnectStage(stage, result);
    if (!reply.WriteInt32(ret)) {
        NETMGR_EXT_LOG_E("ReplyNotifyConnectStage write reply failed");
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyGetSysVpnCertUri(MessageParcel &data, MessageParcel &reply)
{
    int32_t certType;
    if (!data.ReadInt32(certType)) {
        NETMGR_EXT_LOG_E("ReplyGetSysVpnCertUri read data failed");
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    std::string certUri;
    int32_t ret = GetSysVpnCertUri(certType, certUri);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_E("ReplyGetSysVpnCertUri failed, ret=%{public}d", ret);
        return ret;
    }
    if (!reply.WriteString(certUri)) {
        NETMGR_EXT_LOG_E("ReplyGetSysVpnCertUri write reply failed");
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}
#endif // SUPPORT_SYSVPN

int32_t NetworkVpnServiceStub::ReplyRegisterVpnEvent(MessageParcel &data, MessageParcel &reply)
{
    sptr<IVpnEventCallback> callback = iface_cast<IVpnEventCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("ReplyRegisterVpnEvent callback is null.");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }

    int32_t result = RegisterVpnEvent(callback);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyUnregisterVpnEvent(MessageParcel &data, MessageParcel &reply)
{
    sptr<IVpnEventCallback> callback = iface_cast<IVpnEventCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        NETMGR_EXT_LOG_E("ReplyUnregisterVpnEvent callback is null.");
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }

    int32_t result = UnregisterVpnEvent(callback);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyCreateVpnConnection(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = CreateVpnConnection();
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyFactoryResetVpn(MessageParcel &data, MessageParcel &reply)
{
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyRegisterBundleName(MessageParcel &data, MessageParcel &reply)
{
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplyGetSelfAppName(MessageParcel &data, MessageParcel &reply)
{
    std::string selfAppName;
    int32_t result = GetSelfAppName(selfAppName);
    if (result != ERR_NONE) {
        NETMGR_EXT_LOG_E("GetSelfAppName failed on service");
        return result;
    }
    if (!reply.WriteString(selfAppName)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkVpnServiceStub::ReplySetSelfVpnPid(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = SetSelfVpnPid();
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
