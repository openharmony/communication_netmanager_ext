/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NET_FIREWALL_ASYNC_WORK_H
#define NET_FIREWALL_ASYNC_WORK_H

#include <napi/native_api.h>

namespace OHOS {
namespace NetManagerStandard {
namespace NetFirewallAsyncWork {
void ExecSetNetFirewallPolicy(napi_env env, void *data);

void SetNetFirewallPolicyCallback(napi_env env, napi_status status, void *data);

void ExecGetNetFirewallPolicy(napi_env env, void *data);

void GetNetFirewallPolicyCallback(napi_env env, napi_status status, void *data);

void ExecAddNetFirewallRule(napi_env env, void *data);

void AddNetFirewallRuleCallback(napi_env env, napi_status status, void *data);

void ExecUpdateNetFirewallRule(napi_env env, void *data);

void UpdateNetFirewallRuleCallback(napi_env env, napi_status status, void *data);

void ExecDeleteNetFirewallRule(napi_env env, void *data);

void DeleteNetFirewallRuleCallback(napi_env env, napi_status status, void *data);

void ExecGetNetFirewallRules(napi_env env, void *data);

void GetNetFirewallRulesCallback(napi_env env, napi_status status, void *data);

void ExecGetNetFirewallRule(napi_env env, void *data);

void GetNetFirewallRuleCallback(napi_env env, napi_status status, void *data);

void ExecGetInterceptRecords(napi_env env, void *data);

void GetInterceptRecordCallbacks(napi_env env, napi_status status, void *data);
} // namespace NetFirewallAsyncWork
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_FIREWALL_ASYNC_WORK_H