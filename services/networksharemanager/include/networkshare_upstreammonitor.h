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

#ifndef NETWORKSHARE_NETWORK_MONITOR_H
#define NETWORKSHARE_NETWORK_MONITOR_H

#include <any>
#include <map>

#include "event_handler.h"
#include "ffrt.h"
#include "net_conn_callback_stub.h"
#include "net_conn_client.h"
#include "networkshare_hisysevent.h"
#include "networkshare_state_common.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t INVALID_NETID = -1;
class NetworkShareUpstreamMonitor : public std::enable_shared_from_this<NetworkShareUpstreamMonitor> {
public:
    static std::shared_ptr<NetworkShareUpstreamMonitor> &GetInstance()
    {
        static std::shared_ptr<NetworkShareUpstreamMonitor> instance = std::make_shared<NetworkShareUpstreamMonitor>();
        return instance;
    }
    NetworkShareUpstreamMonitor();
    virtual ~NetworkShareUpstreamMonitor();
    class NetConnectionCallback : public NetConnCallbackStub {
    public:
        NetConnectionCallback(const std::shared_ptr<NetworkShareUpstreamMonitor> &networkmonitor, int32_t callbackType);
        ~NetConnectionCallback() = default;

        int32_t NetAvailable(sptr<NetHandle> &netHandle) override;
        int32_t NetCapabilitiesChange(sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap) override;
        int32_t NetConnectionPropertiesChange(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info) override;
        int32_t NetLost(sptr<NetHandle> &netHandle) override;
        int32_t NetUnavailable() override;
        int32_t NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked) override;

    private:
        std::shared_ptr<NetworkShareUpstreamMonitor> NetworkMonitor_;
        ffrt::queue ffrtQueue{"NetworkShareUpstreamMonitorCallback"};
    };

public:
    class NotifyUpstreamCallback {
    public:
        virtual void OnUpstreamStateChanged(int32_t msgName, int32_t param1) = 0;
        virtual void OnUpstreamStateChanged(int32_t msgName, int32_t param1, int32_t param2,
                                            const std::any &messageObj) = 0;
    };

    class MonitorEventHandler : public AppExecFwk::EventHandler {
    public:
        MonitorEventHandler(const std::shared_ptr<NetworkShareUpstreamMonitor> &networkmonitor,
                            const std::shared_ptr<AppExecFwk::EventRunner> &runner);
        ~MonitorEventHandler() = default;

    private:
        std::shared_ptr<NetworkShareUpstreamMonitor> networkMonitor_;
    };

    /**
     * set eventhandler
     */
    void SetOptionData(int what);

    /**
     * set callback to listen default network modify
     */
    void ListenDefaultNetwork();
    void UnregisterListenDefaultNetwork();
    /**
     * get current upstream networ (default network now)
     */
    bool GetCurrentGoodUpstream(std::shared_ptr<UpstreamNetworkInfo> &upstreamNetInfo);

    /**
     * register main state machine callback
     */
    void RegisterUpstreamChangedCallback(const std::shared_ptr<NotifyUpstreamCallback> &callback);

private:
    void NotifyMainStateMachine(int32_t which, const std::shared_ptr<UpstreamNetworkInfo> &obj);
    void NotifyMainStateMachine(int32_t which);
    void HandleNetAvailable(sptr<NetHandle> &netHandle);
    void HandleNetCapabilitiesChange(sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &newNetAllCap);
    void HandleConnectionPropertiesChange(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &newNetLinkInfo);
    void HandleNetLost(sptr<NetHandle> &netHandle);

private:
    int32_t eventId_ = 0;
    std::mutex networkCallbackMutex_;
    sptr<NetConnectionCallback> defaultNetworkCallback_ = nullptr;
    std::map<int32_t, std::shared_ptr<UpstreamNetworkInfo>> networkMaps_;
    std::mutex networkMapMutex_;
    int32_t defaultNetworkId_ = INVALID_NETID;
    std::shared_ptr<NotifyUpstreamCallback> notifyUpstreamCallback_ = nullptr;
    sptr<NetSpecifier> netSpecifier_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKSHARE_NETWORK_MONITOR_H
