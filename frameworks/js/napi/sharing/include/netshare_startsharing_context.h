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

#ifndef NETMANAGER_EXT_TSTATSGETUIDRXBYTES_CONTEXT_H
#define NETMANAGER_EXT_TSTATSGETUIDRXBYTES_CONTEXT_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <napi/native_api.h>

#include "base_context.h"
#include "nocopyable.h"

namespace OHOS {
namespace NetManagerStandard {
class NetShareStartSharingContext final : public BaseContext {
public:
    DISALLOW_COPY_AND_MOVE(NetShareStartSharingContext);

    NetShareStartSharingContext() = delete;
    explicit NetShareStartSharingContext(napi_env env, EventManager *manager);

    void ParseParams(napi_value *params, size_t paramsCount);
    int32_t GetParam();
    int32_t GetBytes64();
    std::vector<std::string> GetIfaces();
    void SetParam(int32_t param);
    void SetIface(std::vector<std::string> ifaces);
    void SetBytes64(int64_t bytes64);

private:
    int32_t param_ = 0;
    std::vector<std::string> ifaces_;
    int64_t bytes64_ = 0;

private:
    bool CheckParamsType(napi_value *params, size_t paramsCount);
};

using StopSharingContext = NetShareStartSharingContext;
using GetSharingIfacesContext = NetShareStartSharingContext;
using GetSharingStateContext = NetShareStartSharingContext;
using GetSharableRegexesContext = NetShareStartSharingContext;
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMANAGER_EXT_TSTATSGETUIDRXBYTES_CONTEXT_H
