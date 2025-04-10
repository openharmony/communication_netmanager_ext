/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef NETMANAGER_EXT_CONSTANTS_H
#define NETMANAGER_EXT_CONSTANTS_H

#include <errors.h>

namespace OHOS {
namespace NetManagerStandard {

enum {
    NETMANAGER_EXT_COMMON = 0x00,
    EXTERNALVPN_MANAGER = 0x01,
    NETMANAGER_ETHERNET_MANAGER = 0x02,
    NETMANAGER_MDNS_MANAGER = 0x03,
};

// for network sharing
enum class SharingIfaceType {
    SHARING_NONE = -1,
    SHARING_WIFI,
    SHARING_USB,
    SHARING_BLUETOOTH,
};
enum class SharingIfaceState {
    SHARING_NIC_SERVING = 1,
    SHARING_NIC_CAN_SERVER,
    SHARING_NIC_ERROR,
};

enum class VpnConnectState {
    VPN_CONNECTED = 1,
    VPN_DISCONNECTED,
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMANAGER_EXT_CONSTANTS_H
