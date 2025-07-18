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

#include "vpn_exec.h"

#include <cstdint>
#include <securec.h>

#include "napi_utils.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"
#include "networkvpn_client.h"
#ifdef SUPPORT_SYSVPN
#include "vpn_config_utils.h"
#endif // SUPPORT_SYSVPN

namespace OHOS {
namespace NetManagerStandard {
namespace VpnExec {
template <typename ContextT> static inline NetworkVpnClient *GetVpnConnectionInstance(ContextT *context)
{
    if (context == nullptr) {
        return nullptr;
    }
    auto manager = context->GetManager();
    return (manager == nullptr) ? nullptr : reinterpret_cast<NetworkVpnClient *>(manager->GetData());
}

bool ExecPrepare(PrepareContext *context)
{
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }
    int32_t result = vpnClient->Prepare(context->isExistVpn_, context->isRun_, context->package_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecSetUp(SetUpContext *context)
{
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }
    int32_t result = NETMANAGER_EXT_SUCCESS;
#ifdef SUPPORT_SYSVPN
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return false;
    }
    if (context->sysVpnConfig_ != nullptr) {
        // is system vpn
        result = vpnClient->SetUpVpn(context->sysVpnConfig_);
    } else {
        result = vpnClient->SetUpVpn(context->vpnConfig_, context->fd_);
    }
#else
    result = vpnClient->SetUpVpn(context->vpnConfig_, context->fd_);
#endif // SUPPORT_SYSVPN
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecProtect(ProtectContext *context)
{
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }
    int32_t result = vpnClient->Protect(context->socketFd_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecDestroy(DestroyContext *context)
{
    auto vpnClient = GetVpnConnectionInstance(context);
    if (vpnClient == nullptr) {
        NETMANAGER_EXT_LOGE("vpnClient is nullptr");
        return false;
    }
    int32_t result = vpnClient->DestroyVpn();
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

#ifdef SUPPORT_SYSVPN
bool ExecAddSysVpnConfig(AddContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return false;
    }
    int32_t result = NetworkVpnClient::GetInstance().AddSysVpnConfig(context->vpnConfig_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecDeleteSysVpnConfig(DeleteContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return false;
    }
    int32_t result = NetworkVpnClient::GetInstance().DeleteSysVpnConfig(context->vpnId_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecGetSysVpnConfigList(GetListContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return false;
    }
    int32_t result = NetworkVpnClient::GetInstance().GetSysVpnConfigList(context->vpnList_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecGetSysVpnConfig(GetContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return false;
    }
    int32_t result = NetworkVpnClient::GetInstance().GetSysVpnConfig(context->vpnConfig_, context->vpnId_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecGetConnectedSysVpnConfig(GetConnectedContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return false;
    }
    int32_t result = NetworkVpnClient::GetInstance().GetConnectedSysVpnConfig(context->vpnConfig_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}

bool ExecGetConnectedVpnAppInfo(GetAppInfoContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return false;
    }
    int32_t result = NetworkVpnClient::GetInstance().GetConnectedVpnAppInfo(context->bundleNameList_);
    if (result != NETMANAGER_EXT_SUCCESS) {
        context->SetErrorCode(result);
        return false;
    }
    return true;
}
#endif // SUPPORT_SYSVPN

napi_value PrepareCallback(PrepareContext *context)
{
    napi_value obj = NapiUtils::CreateObject(context->GetEnv());
    NapiUtils::SetBooleanProperty(context->GetEnv(), obj, "isExistVpn", context->isExistVpn_);
    NapiUtils::SetBooleanProperty(context->GetEnv(), obj, "isRun", context->isRun_);
    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), obj, "package", context->package_);
    return obj;
}

napi_value SetUpCallback(SetUpContext *context)
{
    return NapiUtils::CreateInt32(context->GetEnv(), context->fd_);
}

napi_value ProtectCallback(ProtectContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

napi_value DestroyCallback(DestroyContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

#ifdef SUPPORT_SYSVPN
napi_value AddSysVpnConfigCallback(AddContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return nullptr;
    }
    return NapiUtils::GetUndefined(context->GetEnv());
}

napi_value DeleteSysVpnConfigCallback(DeleteContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return nullptr;
    }
    return NapiUtils::GetUndefined(context->GetEnv());
}

napi_value GetSysVpnConfigCallback(GetContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return nullptr;
    }
    return VpnConfigUtils::CreateNapiVpnConfig(context->GetEnv(), context->vpnConfig_);
}

napi_value GetSysVpnConfigListCallback(GetListContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return nullptr;
    }
    int32_t index = 0;
    auto len = context->vpnList_.size();
    napi_value array = NapiUtils::CreateArray(context->GetEnv(), len);
    for (const auto &info : context->vpnList_) {
        napi_value config = NapiUtils::CreateObject(context->GetEnv());
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), config, VpnConfigUtils::CONFIG_VPN_ID, info->vpnId_);
        NapiUtils::SetStringPropertyUtf8(context->GetEnv(), config, VpnConfigUtils::CONFIG_VPN_NAME, info->vpnName_);
        NapiUtils::SetArrayElement(context->GetEnv(), array, index, config);
        ++index;
    }
    return array;
}

napi_value GetConnectedSysVpnConfigCallback(GetConnectedContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return nullptr;
    }
    return VpnConfigUtils::CreateNapiVpnConfig(context->GetEnv(), context->vpnConfig_);
}

napi_value GetConnectedVpnAppInfoCallback(GetAppInfoContext *context)
{
    if (context == nullptr) {
        NETMANAGER_EXT_LOGE("context is nullptr");
        return nullptr;
    }
    int32_t index = 0;
    auto len = context->bundleNameList_.size();
    napi_value array = NapiUtils::CreateArray(context->GetEnv(), len);
    for (const auto &info : context->bundleNameList_) {
        napi_value bundleName = NapiUtils::CreateStringUtf8(context->GetEnv(), info);
        NapiUtils::SetArrayElement(context->GetEnv(), array, index, bundleName);
        ++index;
    }
    return array;
}
#endif // SUPPORT_SYSVPN
} // namespace VpnExec
} // namespace NetManagerStandard
} // namespace OHOS