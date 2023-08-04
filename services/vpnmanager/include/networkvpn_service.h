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

#ifndef NETWORK_VPN_SERVICE_H
#define NETWORK_VPN_SERVICE_H

#include "event_handler.h"
#include "i_vpn_conn_state_cb.h"
#include "net_vpn_impl.h"
#include "networkvpn_service_stub.h"
#include "os_account_manager.h"
#include "singleton.h"
#include "system_ability.h"

namespace OHOS {
namespace NetManagerStandard {
class NetworkVpnService : public SystemAbility, public NetworkVpnServiceStub {
    DECLARE_SINGLETON(NetworkVpnService)
    DECLARE_SYSTEM_ABILITY(NetworkVpnService)

    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    class VpnConnStateCb : public IVpnConnStateCb {
    public:
        explicit VpnConnStateCb(const NetworkVpnService &vpnService) : vpnService_(vpnService){};
        virtual ~VpnConnStateCb() = default;
        void OnVpnConnStateChanged(const VpnConnectState &state) override;

    private:
        const NetworkVpnService &vpnService_;
    };

public:
    /**
     * service start
     */
    void OnStart() override;

    /**
     * service stop
     */
    void OnStop() override;

    /**
     * check current whether has vpn is running
     */
    int32_t Prepare(bool &isExistVpn, bool &isRun, std::string &pkg) override;

    /**
     * This function is called when the three-party vpn application negotiation ends
     */
    int32_t SetUpVpn(const sptr<VpnConfig> &config) override;

    /**
     * protect vpn tunnel
     */
    int32_t Protect() override;

    /**
     * stop the vpn connection
     */
    int32_t DestroyVpn() override;

    /**
     * register callback
     */
    int32_t RegisterVpnEvent(const sptr<IVpnEventCallback> callback) override;

    /**
     * unregister callback
     */
    int32_t UnregisterVpnEvent(const sptr<IVpnEventCallback> callback) override;

    /**
     * dump function
     */
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

private:
    bool Init();
    void GetDumpMessage(std::string &message);
    int32_t CheckCurrentAccountType(int32_t &userId);

    void OnVpnMultiUserSetUp();
    int32_t SyncRegisterVpnEvent(const sptr<IVpnEventCallback> callback);
    int32_t SyncUnregisterVpnEvent(const sptr<IVpnEventCallback> callback);

private:
    ServiceRunningState state_ = ServiceRunningState::STATE_STOPPED;
    bool isServicePublished_ = false;
    std::shared_ptr<IVpnConnStateCb> vpnConnCallback_;
    std::shared_ptr<NetVpnImpl> vpnObj_;

    std::vector<sptr<IVpnEventCallback>> vpnEventCallbacks_;
    std::shared_ptr<AppExecFwk::EventRunner> policyCallRunner_;
    std::shared_ptr<AppExecFwk::EventHandler> policyCallHandler_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORK_VPN_SERVICE_H
