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

#ifndef NETMANAGER_BASE_STARTSEARCHINGMDNS_CONTEXT_H
#define NETMANAGER_BASE_STARTSEARCHINGMDNS_CONTEXT_H

#include <cstddef>
#include <cstdint>

#include "base_context.h"
#include "mdns_instances.h"

namespace OHOS::NetManagerStandard {
class MDnsStartSearchingContext final : public BaseContext {
public:
    DISALLOW_COPY_AND_MOVE(MDnsStartSearchingContext);
    MDnsStartSearchingContext() = delete;
    explicit MDnsStartSearchingContext(napi_env env, std::shared_ptr<EventManager>& manager);
    void ParseParams(napi_value *params, size_t paramsCount);
    wptr<MDnsDiscoveryObserver> GetObserver();
    MDnsDiscoveryInstance GetDiscover();

private:
    bool CheckParamsType(napi_value *params, size_t paramsCount);
    std::string serviceType_;
    wptr<MDnsDiscoveryObserver> observer_;
    MDnsDiscoveryInstance discover_;
};
using MDnsStopSearchingContext = MDnsStartSearchingContext;
} // namespace OHOS::NetManagerStandard
#endif /* NETMANAGER_BASE_STARTSEARCHINGMDNS_CONTEXT_H */
