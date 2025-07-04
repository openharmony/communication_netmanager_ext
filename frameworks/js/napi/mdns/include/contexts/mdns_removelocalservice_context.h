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

#ifndef NETMANAGER_BASE_REMOVELOCALSERVICE_CONTEXT_H
#define NETMANAGER_BASE_REMOVELOCALSERVICE_CONTEXT_H

#include <cstddef>
#include <cstdint>

#include "mdns_callback_observer.h"
#include "mdns_service_info.h"

namespace OHOS::NetManagerStandard {
class MDnsRemoveLocalServiceContext final : public MDnsBaseContext {
public:
    DISALLOW_COPY_AND_MOVE(MDnsRemoveLocalServiceContext);
    MDnsRemoveLocalServiceContext() = delete;
    explicit MDnsRemoveLocalServiceContext(napi_env env, std::shared_ptr<EventManager>& manager);

    void ParseParams(napi_value *params, size_t paramsCount);
    sptr<IRegistrationCallback> GetObserver();

private:
    bool CheckParamsType(napi_value *params, size_t paramsCount);
    sptr<IRegistrationCallback> regObserver_;
};
} // namespace OHOS::NetManagerStandard
#endif /* NETMANAGER_BASE_REMOVELOCALSERVICE_CONTEXT_H */
