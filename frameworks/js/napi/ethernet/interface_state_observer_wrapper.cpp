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

#include "interface_state_observer_wrapper.h"

#include "constant.h"
#include "ethernet_client.h"
#include "module_template.h"
#include "net_manager_constants.h"
#include "netmanager_ext_log.h"

namespace OHOS {
namespace NetManagerStandard {
InterfaceStateObserverWrapper::InterfaceStateObserverWrapper()
    : observer_(new InterfaceStateObserver()), manager_(std::make_shared<EventManager>()), registered_(false)
{
}

InterfaceStateObserverWrapper::~InterfaceStateObserverWrapper() = default;

InterfaceStateObserverWrapper &InterfaceStateObserverWrapper::GetInstance()
{
    static InterfaceStateObserverWrapper instance;
    return instance;
}

napi_value InterfaceStateObserverWrapper::On(napi_env env, napi_callback_info info,
                                             const std::initializer_list<std::string> &events, bool asyncCallback)
{
    if (manager_ == nullptr || observer_ == nullptr) {
        return NapiUtils::GetUndefined(env);
    }
    size_t paramsCount = MAX_PARAM_NUM;
    napi_value params[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramsCount, params, nullptr, nullptr));
    if (paramsCount != PARAM_OPTIONS_AND_CALLBACK || NapiUtils::GetValueType(env, params[ARG_INDEX_0]) != napi_string ||
        NapiUtils::GetValueType(env, params[ARG_INDEX_1]) != napi_function) {
        NETMANAGER_EXT_LOGE("on off once interface para: [string, function]");
        napi_throw_error(env, std::to_string(NETMANAGER_ERR_PARAMETER_ERROR).c_str(), "Parameter error");
        return NapiUtils::GetUndefined(env);
    }

    const auto event = NapiUtils::GetStringFromValueUtf8(env, params[ARG_INDEX_0]);
    if (std::find(events.begin(), events.end(), event) == events.end()) {
        return NapiUtils::GetUndefined(env);
    }
    if (!registered_) {
        int32_t ret = DelayedSingleton<EthernetClient>::GetInstance()->RegisterIfacesStateChanged(observer_);
        NETMANAGER_EXT_LOGI("ret = [%{public}d]", ret);
        registered_ = (ret == NETMANAGER_SUCCESS);
        if (!registered_) {
            NETMANAGER_EXT_LOGE("register callback error");
            NetBaseErrorCodeConvertor convertor;
            std::string errorMsg = convertor.ConvertErrorCode(ret);
            napi_throw_error(env, std::to_string(ret).c_str(), errorMsg.c_str());
        }
    }
    if (registered_) {
        manager_->AddListener(env, event, params[ARG_INDEX_1], false, asyncCallback);
    }
    return NapiUtils::GetUndefined(env);
}

napi_value InterfaceStateObserverWrapper::Off(napi_env env, napi_callback_info info,
                                              const std::initializer_list<std::string> &events, bool asyncCallback)
{
    if (manager_ == nullptr || observer_ == nullptr) {
        return NapiUtils::GetUndefined(env);
    }

    napi_value thisVal = nullptr;
    size_t paramsCount = MAX_PARAM_NUM;
    napi_value params[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramsCount, params, &thisVal, nullptr));

    if (!(paramsCount == PARAM_JUST_OPTIONS || paramsCount == PARAM_OPTIONS_AND_CALLBACK) ||
        NapiUtils::GetValueType(env, params[ARG_INDEX_0]) != napi_string) {
        NETMANAGER_EXT_LOGE("on off once interface para: [string, function?]");
        napi_throw_error(env, std::to_string(NETMANAGER_ERR_PARAMETER_ERROR).c_str(), "Parameter error");
        return NapiUtils::GetUndefined(env);
    }

    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK &&
        NapiUtils::GetValueType(env, params[ARG_INDEX_1]) != napi_function) {
        NETMANAGER_EXT_LOGE("on off once interface para: [string, function]");
        return NapiUtils::GetUndefined(env);
    }

    std::string event = NapiUtils::GetStringFromValueUtf8(env, params[ARG_INDEX_0]);
    if (std::find(events.begin(), events.end(), event) == events.end()) {
        return NapiUtils::GetUndefined(env);
    }

    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        manager_->DeleteListener(event, params[ARG_INDEX_1]);
    } else {
        manager_->DeleteListener(event);
    }

    if (manager_->IsListenerListEmpty()) {
        auto ret = DelayedSingleton<EthernetClient>::GetInstance()->UnregisterIfacesStateChanged(observer_);
        if (ret != NETMANAGER_SUCCESS) {
            NETMANAGER_EXT_LOGE("unregister ret = %{public}d", ret);
            NetBaseErrorCodeConvertor convertor;
            std::string errorMsg = convertor.ConvertErrorCode(ret);
            napi_throw_error(env, std::to_string(ret).c_str(), errorMsg.c_str());
        }
        registered_ = false;
    }
    return NapiUtils::GetUndefined(env);
}
} // namespace NetManagerStandard
} // namespace OHOS
