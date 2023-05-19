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

#ifndef NETWORKVPN_CLIENT_H
#define NETWORKVPN_CLIENT_H

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include <parcel.h>
#include <refbase.h>
#include <unistd.h>

#include "i_networkvpn_service.h"
#include "i_vpn_event_callback.h"
#include "vpn_interface.h"

namespace OHOS {
namespace NetManagerStandard {
class NetworkVpnClient {
public:
    NetworkVpnClient() = default;
    ~NetworkVpnClient() = default;

public:
    /**
     * start internal vpn
     *
     * @param isExistVpn check whether exist vpn connection
     * @param isRun if isExistVpn=true, check the vpn is running or not
     * @param pkg Indicates which application the current vpn belongs to
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     */
    int32_t Prepare(bool &isExistVpn, bool &isRun, std::string &pkg);

    /**
     * extended vpn need always communication with remote vpn server, the data is send/receive by default network but
     * not vpn network.
     *
     * @param socketFd extended vpn opened soecket fd
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     */
    int32_t Protect(uint32_t socketFd);

    /**
     * after extended vpn's negotiation over, need system create a VPN interface using the config parameters.
     *
     * @param config VPN interface parameters
     * @param tunFd the virtual interface fd(out param)
     * @return the interface node's file descriptor(>0) if process normal, others is error
     */
    int32_t SetUp(sptr<VpnConfig> config, int32_t &tunFd);

    /**
     * stop the vpn connection, system will destroy the vpn network.
     *
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     */
    int32_t DestroyVpn();

    /**
     * register the vpn state callback
     *
     * @param callback if this fuction return NETMANAGER_EXT_SUCCESS(0), this callback will be called by service
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     */
    int32_t RegisterVpnEvent(sptr<IVpnEventCallback> callback);

    /**
     * unregister the vpn state callback
     *
     * @param callback if this fuction return NETMANAGER_EXT_SUCCESS(0), this callback will not be called by service
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     */
    int32_t UnregisterVpnEvent(sptr<IVpnEventCallback> callback);

private:
    class MonitorVpnServiceDead : public IRemoteObject::DeathRecipient {
    public:
        explicit MonitorVpnServiceDead(NetworkVpnClient &client) : client_(client) {}
        ~MonitorVpnServiceDead() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetworkVpnClient &client_;
    };

    sptr<INetworkVpnService> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    std::unique_ptr<VpnInterface> vpnInterface_;
    sptr<INetworkVpnService> networkVpnService_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKVPN_CLIENT_H