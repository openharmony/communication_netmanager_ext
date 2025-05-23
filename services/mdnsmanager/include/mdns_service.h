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

#ifndef MDNS_SERVICE_H
#define MDNS_SERVICE_H

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "refbase.h"
#include "singleton.h"
#include "system_ability.h"

#include "imdns_service.h"
#include "mdns_service_stub.h"
#include "mdns_manager.h"
#include "net_interface_callback.h"

namespace OHOS {
namespace NetManagerStandard {

class MDnsService : public SystemAbility, public MdnsServiceStub {
    DECLARE_DELAYED_SINGLETON(MDnsService)
    DECLARE_SYSTEM_ABILITY(MDnsService)

public:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    int32_t RegisterService(const MDnsServiceInfo &serviceInfo, const sptr<IRegistrationCallback> &cb) override;
    int32_t UnRegisterService(const sptr<IRegistrationCallback> &cb) override;

    int32_t StartDiscoverService(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb) override;
    int32_t StopDiscoverService(const sptr<IDiscoveryCallback> &cb) override;

    int32_t ResolveService(const MDnsServiceInfo &serviceInfo, const sptr<IResolveCallback> &cb) override;

    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

protected:
    void OnStart() override;
    void OnStop() override;
    int32_t OnIdle(const SystemAbilityOnDemandReason &idleReason) override;

private:
    bool Init();

    bool isRegistered_;
    ServiceRunningState state_;
    sptr<INetInterfaceStateCallback> netStateCallback_;

private:
    class MdnsCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit MdnsCallbackDeathRecipient(MDnsService &client) : client_(client) {}
        ~MdnsCallbackDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        MDnsService &client_;
    };
    void OnRemoteDied(const wptr<IRemoteObject> &remoteObject);
    void AddClientDeathRecipient(const sptr<IDiscoveryCallback> &cb);
    void RemoveClientDeathRecipient(const sptr<IDiscoveryCallback> &cb);
    void UnloadSystemAbility();
    void RemoveALLClientDeathRecipient();
    std::mutex remoteMutex_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::vector<sptr<IDiscoveryCallback>> remoteCallback_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // MDNS_SERVICE_H