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

#include <arpa/inet.h>
#include "net_interface_callback.h"
#include "mdns_manager.h"
#include "netmgr_ext_log_wrapper.h"

#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#include <thread>

namespace OHOS {
namespace NetManagerStandard {
constexpr int WAITING_TIME_MS = 1000;

NetInterfaceStateCallback::NetInterfaceStateCallback() {}

int32_t NetInterfaceStateCallback::OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName,
                                                             int32_t flags, int32_t scope)
{
    NETMGR_EXT_LOG_I("OnInterfaceAddressUpdated, iface:[%{public}s], scope:[%{public}d]",
                     ifName.c_str(), scope);
    if (ifName.empty()) {
        NETMGR_EXT_LOG_E("mdns_log Invalid interface name");
        return NETMANAGER_SUCCESS;
    }

    std::string ifrName = ifName;
    std::transform(ifrName.begin(), ifrName.end(), ifrName.begin(), ::tolower);
    if (ifrName.find("p2p") != std::string::npos) {
        NETMGR_EXT_LOG_D("mdns_log Not p2p netcard handle");
        return NETMANAGER_SUCCESS;
    }

    size_t pos = addr.find("/");
    if (pos == std::string::npos) {
        pos = addr.length();
    }
    std::string tmpAddr = addr.substr(0, pos);
    if (tmpAddr.empty()) {
        NETMGR_EXT_LOG_E("mdns_log Invalid IP address");
        return NETMANAGER_SUCCESS;
    }
    in6_addr ipAddr;
    int32_t ret = inet_pton(AF_INET6, tmpAddr.c_str(), &ipAddr);
    if (ret > 0 && !MDnsManager::GetInstance().IsSupportIpV6()) {
        NETMGR_EXT_LOG_D("mdns_log Not support IpV6");
        return NETMANAGER_SUCCESS;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(WAITING_TIME_MS));
    MDnsManager::GetInstance().RestartMDnsProtocolImpl();
    return NETMANAGER_SUCCESS;
}

int32_t NetInterfaceStateCallback::OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName,
                                                             int32_t flags, int32_t scope)
{
    NETMGR_EXT_LOG_D("OnInterfaceAddressRemoved, iface:[%{public}s], scope:[%{public}d]",
                     ifName.c_str(), scope);
    return NETMANAGER_SUCCESS;
}

int32_t NetInterfaceStateCallback::OnInterfaceAdded(const std::string &ifName)
{
    NETMGR_EXT_LOG_D("OnInterfaceAdded, iface:[%{public}s]", ifName.c_str());
    return NETMANAGER_SUCCESS;
}

int32_t NetInterfaceStateCallback::OnInterfaceRemoved(const std::string &ifName)
{
    NETMGR_EXT_LOG_D("OnInterfaceRemoved, iface:[%{public}s]", ifName.c_str());
    return NETMANAGER_SUCCESS;
}

int32_t NetInterfaceStateCallback::OnInterfaceChanged(const std::string &ifName, bool up)
{
    NETMGR_EXT_LOG_D("OnInterfaceChanged, iface:[%{public}s]->Up:[%{public}d]", ifName.c_str(), up);
    return NETMANAGER_SUCCESS;
}

int32_t NetInterfaceStateCallback::OnInterfaceLinkStateChanged(const std::string &ifName, bool up)
{
    NETMGR_EXT_LOG_D("OnInterfaceLinkStateChanged, iface:[%{public}s]->Up:[%{public}d]", ifName.c_str(), up);
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
