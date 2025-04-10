/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef NETWORKSHARE_CLIENT_H
#define NETWORKSHARE_CLIENT_H

#include <string>

#include "i_netshare_result_callback.h"
#include "inetwork_share_service.h"
#include "isharing_event_callback.h"
#include "parcel.h"
#include "singleton.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class NetworkShareLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    bool IsFailed();
    const sptr<IRemoteObject> &GetRemoteObject() const;

private:
    bool loadSAFailed_ = false;
    sptr<IRemoteObject> remoteObject_ = nullptr;
};
class NetworkShareClient {
    DECLARE_DELAYED_SINGLETON(NetworkShareClient)

public:
    /**
     * check if the sharing is supported
     *
     * @param supported NETWORKSHARE_IS_SUPPORTED(1) if supported, other is NETWORKSHARE_IS_UNSUPPORTED(0)
     * @return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t IsSharingSupported(int32_t &supported);

    /**
     * get the sharing running state, WiFi, Bluetooth, USB, as long as one of them is shared, it will return true
     *
     * @param sharingStatus NETWORKSHARE_IS_SHARING(1) if sharing running, others is NETWORKSHARE_IS_UNSHARING(0)
     * @return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t IsSharing(int32_t &sharingStatus);

    /**
     * start network by type
     *
     * @param type network sharing type, including Wifi, Bluetooth, USB
     * @return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t StartSharing(const SharingIfaceType &type);

    /**
     * stop network by type
     *
     * @param type network sharing type, including Wifi, Bluetooth, USB
     * @return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t StopSharing(const SharingIfaceType &type);

    /**
     * register the sharing state callback
     *
     * @param callback if this fuction return NETMANAGER_EXT_SUCCESS, this callback will be called by service
     * @return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterSharingEvent(sptr<ISharingEventCallback> callback);

    /**
     * unregister the sharing state callback
     *
     * @param callback if this fuction return NETMANAGER_EXT_SUCCESS, this callback will not be called by
     * service
     * @return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UnregisterSharingEvent(sptr<ISharingEventCallback> callback);

    /**
     * get the regexs data of the type.
     * like these "usb\d" "wlan\d" "bt-pan"
     *
     * @param type the network sharing type, including Wifi, Bluetooth, USB
     * @param ifaceRegexs get list of interface sharable regex
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetSharableRegexs(const SharingIfaceType &type, std::vector<std::string> &ifaceRegexs);

    /**
     * get sharing state by type
     *
     * @param type the network sharing type, including Wifi, Bluetooth, USB
     * @param state the network sharing state, includes services, can services, errors
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetSharingState(const SharingIfaceType &type, SharingIfaceState &state);

    /**
     * get interface name by sharing state, like these "usb0" "wlan0" "bt-pan"
     *
     * @param state the network sharing state, includes services, can services, errors
     * @param ifaces interface name vector
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetSharingIfaces(const SharingIfaceState &state, std::vector<std::string> &ifaces);

    /**
     * Obtains the number of downlink data bytes of the sharing network interfaces.
     *
     * @param bytes network traffic data unit is KB
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetStatsRxBytes(int32_t &bytes);

    /**
     * Obtains the number of uplink data bytes of the sharing network interfaces.
     *
     * @param bytes network traffic data unit is KB
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetStatsTxBytes(int32_t &bytes);

    /**
     * Obtains the number of total data bytes of the sharing network interfaces.
     *
     * @param bytes network traffic data unit is KB
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetStatsTotalBytes(int32_t &bytes);

    /**
     * Set sysctl prop when start or stop sharing network.
     *
     * @param enabled set sharing network enabled or not
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetConfigureForShare(bool enabled);

private:
    void RestartNetTetheringManagerSysAbility();

private:
    class NetshareDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit NetshareDeathRecipient(NetworkShareClient &client) : client_(client) {}
        ~NetshareDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetworkShareClient &client_;
    };

private:
    sptr<INetworkShareService> GetProxy();
    void RecoverCallback();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<INetworkShareService> networkShareService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    sptr<ISharingEventCallback> callback_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKSHARE_CLIENT_H
