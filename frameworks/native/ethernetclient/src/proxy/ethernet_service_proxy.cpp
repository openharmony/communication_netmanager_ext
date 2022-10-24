/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ethernet_service_proxy.h"

#include "i_ethernet_service.h"
#include "interface_configuration.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "message_option.h"
#include "message_parcel.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
EthernetServiceProxy::EthernetServiceProxy(const sptr<IRemoteObject> &impl)
    :IRemoteProxy<IEthernetService>(impl)
{
}

EthernetServiceProxy::~EthernetServiceProxy() {}

bool EthernetServiceProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(EthernetServiceProxy::GetDescriptor())) {
        NETMGR_EXT_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}

int32_t EthernetServiceProxy::SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(iface)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!ic->Marshalling(data)) {
        NETMGR_EXT_LOG_E("proxy Marshalling failed");
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_SET_IF_CFG, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

sptr<InterfaceConfiguration> EthernetServiceProxy::GetIfaceConfig(const std::string &iface)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!data.WriteString(iface)) {
        return nullptr;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return nullptr;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_GET_IF_CFG, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return nullptr;
    }
    int32_t res = 0;
    if (!reply.ReadInt32(res)) {
        return nullptr;
    }
    if (res != GET_CFG_SUC) {
        return nullptr;
    }
    return InterfaceConfiguration::Unmarshalling(reply);
}

int32_t EthernetServiceProxy::IsIfaceActive(const std::string &iface)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(iface)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_IS_ACTIVATE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }

    return reply.ReadInt32();
}

std::vector<std::string> EthernetServiceProxy::GetAllActiveIfaces()
{
    MessageParcel data;
    std::vector<std::string> ifaces;
    if (!WriteInterfaceToken(data)) {
        return ifaces;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return ifaces;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_GET_ACTIVATE_INTERFACE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return ifaces;
    }

    int32_t size = reply.ReadInt32();
    for (int i = 0; i < size; i++) {
        ifaces.push_back(reply.ReadString());
    }
    return ifaces;
}

int32_t EthernetServiceProxy::ResetFactory()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_RESET_FACTORY, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

int32_t EthernetServiceProxy::SetInterfaceUp(const std::string &iface)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(iface)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_SET_INTERFACE_UP, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

int32_t EthernetServiceProxy::SetInterfaceDown(const std::string &iface)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(iface)) {
        return NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_SET_INTERFACE_DOWN, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

bool EthernetServiceProxy::GetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    NETMGR_EXT_LOG_I("Begin to GetInterfaceConfig iface:[%{public}s]", iface.c_str());
    MessageParcel data;
    int32_t vSize;
    if (!WriteInterfaceToken(data)) {
        return false;
    }
    if (!data.WriteString(iface)) {
        return false;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("Remote is null");
        return false;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_GET_INTERFACE_CONFIG, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_EXT_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return false;
    }
    reply.ReadString(cfg.ifName);
    reply.ReadString(cfg.hwAddr);
    reply.ReadString(cfg.ipv4Addr);
    reply.ReadInt32(cfg.prefixLength);
    vSize = reply.ReadInt32();
    std::vector<std::string> vecString;
    for (int i = 0; i < vSize; i++) {
        vecString.push_back(reply.ReadString());
    }
    if (vSize > 0) {
        cfg.flags.assign(vecString.begin(), vecString.end());
    }
    NETMGR_EXT_LOG_I("End to GetInterfaceConfig");
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS