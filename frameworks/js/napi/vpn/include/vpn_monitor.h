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

#ifndef NET_EXT_NAPI_VPN_MONITOR_H
#define NET_EXT_NAPI_VPN_MONITOR_H

#include <napi/native_api.h>
#include <refbase.h>

#include "event_manager.h"
#include "vpn_event_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class VpnEventCallback : public VpnEventCallbackStub {
public:
    int32_t OnVpnStateChanged(bool isConnected) override;
    int32_t OnMultiVpnStateChanged(bool isConnected, const std::string &bundleName, const std::string &vpnId) override;
    int32_t OnVpnMultiUserSetUp() override{ return ERR_OK; };
};

class VpnMonitor {
private:
    VpnMonitor();
    ~VpnMonitor();
    VpnMonitor(const VpnMonitor &) = delete;
    VpnMonitor &operator=(const VpnMonitor &) = delete;

public:
    static VpnMonitor &GetInstance();

public:
    napi_value On(napi_env env, napi_callback_info info);
    napi_value Off(napi_env env, napi_callback_info info);

    inline std::shared_ptr<EventManager> GetManager() const
    {
        return manager_;
    }

private:
    sptr<VpnEventCallback> eventCallback_ = nullptr;
#ifdef SUPPORT_SYSVPN
    sptr<VpnEventCallback> multiEventCallback_ = nullptr;
#endif // SUPPORT_SYSVPN
    napi_value callback_ = nullptr;
    std::shared_ptr<EventManager> manager_ = nullptr;

private:
    bool ParseParams(napi_env env, napi_callback_info info);
    bool UnwrapManager(napi_env env, napi_value jsObject);
    void Register(napi_env env);
    void Unregister(napi_env env);
#ifdef SUPPORT_SYSVPN
    bool ParseParams(napi_env env, napi_callback_info info, std::string &event);
    void Register(napi_env env, const std::string &event);
    void Unregister(napi_env env, const std::string &event);
#endif // SUPPORT_SYSVPN
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_EXT_NAPI_VPN_MONITOR_H