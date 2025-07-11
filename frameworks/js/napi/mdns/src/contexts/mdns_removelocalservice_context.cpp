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

#include "napi_utils.h"
#include "netmgr_ext_log_wrapper.h"

#include "constant.h"
#include "mdns_addlocalservice_context.h"
#include "mdns_removelocalservice_context.h"

namespace OHOS::NetManagerStandard {
std::mutex g_mDNSUnregisterMutex;
MDnsRemoveLocalServiceContext::MDnsRemoveLocalServiceContext(napi_env env, std::shared_ptr<EventManager>& manager)
    : MDnsBaseContext(env, manager)
{
}

void MDnsRemoveLocalServiceContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(params, paramsCount)) {
        SetNeedThrowException(true);
        SetErrorCode(NET_MDNS_ERR_ILLEGAL_ARGUMENT);
        return;
    }

    std::string bundleName = GetContextIdString(GetEnv(), params[ARG_NUM_0]);
    ParseServiceInfo(GetEnv(), params[ARG_NUM_1]);
    std::string key = bundleName + MDNS_HOSTPORT_SPLITER_STR + GetServiceInfo().name + MDNS_DOMAIN_SPLITER_STR +
                      GetServiceInfo().type;

    {
        std::lock_guard<std::mutex> lock(g_mDNSUnregisterMutex);
        regObserver_ = MDnsAddLocalServiceContext::registerCallbackMap_[key];
        MDnsAddLocalServiceContext::registerCallbackMap_.erase(key);
    }

    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[ARG_NUM_2]) == napi_ok);
        return;
    }
    SetParseOK(true);
}

bool MDnsRemoveLocalServiceContext::CheckParamsType(napi_value *params, size_t paramsCount)
{
    bool bRet = false;
    if (paramsCount == PARAM_JUST_OPTIONS) {
        bRet = NapiUtils::GetValueType(GetEnv(), params[ARG_NUM_0]) == napi_object &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_NUM_1]) == napi_object;
    } else if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        bRet = NapiUtils::GetValueType(GetEnv(), params[ARG_NUM_0]) == napi_object &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_NUM_1]) == napi_object &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_NUM_2]) == napi_function;
    }
    return bRet;
}

sptr<IRegistrationCallback> MDnsRemoveLocalServiceContext::GetObserver()
{
    return regObserver_;
}
} // namespace OHOS::NetManagerStandard
