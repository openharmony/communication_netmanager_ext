/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef I_NETSHARE_RESULT_CALLBACK_H
#define I_NETSHARE_RESULT_CALLBACK_H

#include <string>
#include <iremote_broker.h>

#include "tethering_ipc_interface_code.h"

namespace OHOS {
namespace NetManagerStandard {
class INetShareResultCallback : public IRemoteBroker {
public:
    virtual void OnResult(const int32_t &status) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetworkShareService.INetShareResultCallback");
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NETSHARE_RESULT_CALLBACK_H
