/*
 * Copyright (C) 2022-2024 Huawei Device Co., Ltd.
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

#include "ethernet_async_work.h"

#include "base_async_work.h"
#include "ethernet_exec.h"
#include "get_all_active_ifaces_context.h"
#include "get_device_infomation.h"
#include "get_mac_address_context.h"
#include "get_iface_config_context.h"
#include "is_iface_active_context.h"
#include "set_iface_config_context.h"

namespace OHOS {
namespace NetManagerStandard {
namespace EthernetAsyncWork {
void ExecGetMacAddress(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetMacAddressContext, EthernetExec::ExecGetMacAddress>(env, data);
}

void GetMacAddressCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetMacAddressContext, EthernetExec::GetMacAddressCallback>(env, status, data);
}

void ExecGetIfaceConfig(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetIfaceConfigContext, EthernetExec::ExecGetIfaceConfig>(env, data);
}

void GetIfaceConfigCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetIfaceConfigContext, EthernetExec::GetIfaceConfigCallback>(env, status, data);
}

void ExecSetIfaceConfig(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<SetIfaceConfigContext, EthernetExec::ExecSetIfaceConfig>(env, data);
}

void SetIfaceConfigCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<SetIfaceConfigContext, EthernetExec::SetIfaceConfigCallback>(env, status, data);
}

void ExecIsIfaceActive(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<IsIfaceActiveContext, EthernetExec::ExecIsIfaceActive>(env, data);
}

void IsIfaceActiveCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<IsIfaceActiveContext, EthernetExec::IsIfaceActiveCallback>(env, status, data);
}

void ExecGetAllActiveIfaces(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetAllActiveIfacesContext, EthernetExec::ExecGetAllActiveIfaces>(env, data);
}

void GetAllActiveIfacesCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetAllActiveIfacesContext, EthernetExec::GetAllActiveIfacesCallback>(env, status,
                                                                                                          data);
}

void ExecGetDeviceInformation(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetDeviceInformationContext, EthernetExec::ExecGetDeviceInformation>(env, data);
}
 
void GetDeviceInformationCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetDeviceInformationContext, EthernetExec::GetDeviceInformationCallback>(env,
        status, data);
}
} // namespace EthernetAsyncWork
} // namespace NetManagerStandard
} // namespace OHOS
