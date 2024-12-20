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

#include "mdns_socket_listener.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <iostream>
#include <algorithm>

#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

namespace {
constexpr uint32_t MDNS_MULTICAST_INADDR = (224U << 24) | 251U;
constexpr in6_addr MDNS_MULTICAST_IN6ADDR = {
    {{0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFB}}};

constexpr const char *CONTROL_TAG_REFRESH = "R";

constexpr uint16_t MDNS_PORT = 5353;
constexpr size_t RECV_BUFFER = 2000;
constexpr int WAIT_THREAD_MS = 5;
constexpr int SOCKET_INIT_INTERVAL_MS = 1000;
constexpr size_t MDNS_MAX_SOCKET = 16;
constexpr size_t REFRESH_BUFFER_LEN = 2;
constexpr uint32_t BOOL_VALUE_FALSE = 0;
constexpr uint32_t BOOL_VALUE_TRUE = 1;

inline bool IfaceIsSupported(ifaddrs *ifa)
{
    return ifa->ifa_addr && ((ifa->ifa_flags & IFF_UP) && (ifa->ifa_flags & IFF_MULTICAST)) &&
           (!(ifa->ifa_flags & IFF_LOOPBACK) && !(ifa->ifa_flags & IFF_POINTOPOINT));
}

int InitFdFlags(int sock)
{
    const int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    if (fcntl(sock, F_SETFL, static_cast<uint32_t>(flags) | O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}

int InitReusedSocket(int sock)
{
    const int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&enable), sizeof(enable)) != 0) {
        return -1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const char *>(&enable), sizeof(enable)) != 0) {
        return -1;
    }
    return sock;
}

int InitSocketV4(int sock, ifaddrs *ifa, int port)
{
    const int one = 1;
    const int maxtll = 255;

    if (sock < 0) {
        NETMGR_EXT_LOG_E("mdns_log sock [%{public}d] error", sock);
        return -1;
    }
    if (port != 0 && InitReusedSocket(sock) < 0) {
        NETMGR_EXT_LOG_E("mdns_log InitReusedSocket error");
        return -1;
    }

    bool allOK =
        (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, reinterpret_cast<const char *>(&one), sizeof(one)) == 0) &&
        (setsockopt(sock, IPPROTO_IP, IP_PKTINFO, reinterpret_cast<const char *>(&one), sizeof(one)) == 0) &&
        (setsockopt(sock, IPPROTO_IP, IP_TTL, reinterpret_cast<const char *>(&maxtll), sizeof(maxtll)) == 0) &&
        (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<const char *>(&maxtll), sizeof(maxtll)) == 0);
    if (!allOK) {
        NETMGR_EXT_LOG_E("mdns_log setsockopt IP_MULTICAST_LOOP|IP_PKTINFO|IP_TTL|IP_MULTICAST_TTL error");
        return -1;
    }

    sockaddr_in sockAddr{};

    ip_mreq mreq{};
    mreq.imr_multiaddr.s_addr = htonl(MDNS_MULTICAST_INADDR);
    mreq.imr_interface = ifa ? reinterpret_cast<sockaddr_in *>(ifa->ifa_addr)->sin_addr : in_addr{INADDR_ANY};
    allOK =
        (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) == 0) &&
        (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<const char *>(&mreq.imr_interface),
                    sizeof(in_addr)) == 0);
    if (!allOK) {
        NETMGR_EXT_LOG_E("mdns_log setsockopt IP_ADD_MEMBERSHIP|IP_MULTICAST_IF error");
        return -1;
    }

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = htons(port);

    if (bind(sock, reinterpret_cast<sockaddr *>(&sockAddr), sizeof(sockaddr_in)) != 0) {
        NETMGR_EXT_LOG_E("mdns_log bind failed, errno:[%{public}d]", errno);
        return -1;
    }

    return InitFdFlags(sock);
}

int InitSocketV6(int sock, ifaddrs *ifa, int port)
{
    const int one = 1;
    const int max = 255;

    if (sock < 0) {
        return -1;
    }
    if (port != 0 && InitReusedSocket(sock) < 0) {
        return -1;
    }

    bool allOK =
        (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, reinterpret_cast<const char *>(&one), sizeof(one)) == 0) &&
        (setsockopt(sock, IPPROTO_IPV6, IPV6_2292PKTINFO, reinterpret_cast<const char *>(&one), sizeof(one)) == 0) &&
        (setsockopt(sock, IPPROTO_IPV6, IPV6_2292HOPLIMIT, reinterpret_cast<const char *>(&one), sizeof(one)) == 0) &&
        (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char *>(&one), sizeof(one)) == 0) &&
        (setsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, reinterpret_cast<const char *>(&max), sizeof(max)) == 0) &&
        (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, reinterpret_cast<const char *>(&max), sizeof(max)) == 0);
    if (!allOK) {
        return -1;
    }

    sockaddr_in6 sockAddr{};

    unsigned int ifaceIndex = ifa ? if_nametoindex(ifa->ifa_name) : 0;
    ipv6_mreq mreq{};
    mreq.ipv6mr_multiaddr = MDNS_MULTICAST_IN6ADDR;
    mreq.ipv6mr_interface = ifaceIndex;
    allOK =
        (setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) == 0) &&
        (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, reinterpret_cast<const char *>(&ifaceIndex),
                    sizeof(ifaceIndex)) == 0);
    if (!allOK) {
        return -1;
    }

    sockAddr.sin6_family = AF_INET6;
    sockAddr.sin6_addr = in6addr_any;
    sockAddr.sin6_port = htons(port);
    sockAddr.sin6_flowinfo = 0;
    sockAddr.sin6_scope_id = 0;

    if (bind(sock, reinterpret_cast<sockaddr *>(&sockAddr), sizeof(sockaddr_in6)) != 0) {
        NETMGR_EXT_LOG_E("mdns_log bind failed, errno:[%{public}d]", errno);
        return -1;
    }

    return InitFdFlags(sock);
}

} // namespace

MDnsSocketListener::MDnsSocketListener()
{
    if (socketpair(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0, ctrlPair_) != 0) {
        NETMGR_EXT_LOG_F("mdns_log bind failed, errno:[%{public}d]", errno);
    }
}

MDnsSocketListener::~MDnsSocketListener()
{
    Stop();
}

void MDnsSocketListener::Start()
{
    if (std::this_thread::get_id() == thread_.get_id()) {
        return;
    }
    if (!runningFlag_) {
        runningFlag_ = true;
        thread_ = std::thread([this]() { Run(); });
        std::string threadName = "MDnsSockListen";
        pthread_setname_np(thread_.native_handle(), threadName.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_THREAD_MS));
    }
}

void MDnsSocketListener::Stop()
{
    if (std::this_thread::get_id() == thread_.get_id()) {
        return;
    }
    if (runningFlag_) {
        runningFlag_ = false;
        TriggerRefresh();
        if (thread_.joinable()) {
            thread_.join();
        }
    }
}

bool MDnsSocketListener::Ifaceverification(ifaddrs *ifa, ifaddrs *loaddr)
{
    if (ifa->ifa_addr == nullptr || (ifa->ifa_addr->sa_family != AF_INET && ifa->ifa_addr->sa_family != AF_INET6)) {
        return false;
    }

    std::string ifName(ifa->ifa_name);
    std::transform(ifName.begin(), ifName.end(), ifName.begin(), ::tolower);
    if (ifName.find("p2p") != std::string::npos) {
        return false;
    }

    if ((ifa->ifa_flags & IFF_LOOPBACK) && ifa->ifa_addr->sa_family == AF_INET) {
        loaddr = ifa;
        return false;
    }
    if (!IfaceIsSupported(ifa)) {
        NETMGR_EXT_LOG_I("mdns_log iface [%{public}s] is mismatch", ifa->ifa_name);
        return false;
    }
    return true;
}

void MDnsSocketListener::OpenSocketForEachIface(bool ipv6Support, bool lo)
{
    ifaddrs *ifaddr = nullptr;
    ifaddrs *loaddr = nullptr;
    uint32_t ret = BOOL_VALUE_FALSE;

    do {
        if (getifaddrs(&ifaddr) < 0) {
            NETMGR_EXT_LOG_F("mdns_log getifaddrs failed, errno=[%{public}d]", errno);
            std::this_thread::sleep_for(std::chrono::microseconds(SOCKET_INIT_INTERVAL_MS));
            continue;
        } else {
            for (ifaddrs *ifa = ifaddr; ifa != nullptr && socks_.size() < MDNS_MAX_SOCKET; ifa = ifa->ifa_next) {
                if (!Ifaceverification(ifa, loaddr)) {
                    continue;
                }
                if (ifa->ifa_addr->sa_family == AF_INET) {
                    ret |= OpenSocketV4(ifa);
                } else if (ifa->ifa_addr->sa_family == AF_INET6) {
                    ret |= OpenSocketV6(ifa, ipv6Support);
                }
            }

            if (lo && socks_.size() == 0 && loaddr && loaddr->ifa_addr) {
                ret |= OpenSocketV4(loaddr);
            }

            freeifaddrs(ifaddr);

            if (socks_.size() == 0 || ret == BOOL_VALUE_FALSE) {
                NETMGR_EXT_LOG_W("mdns_log no available iface found");
                std::this_thread::sleep_for(std::chrono::milliseconds(SOCKET_INIT_INTERVAL_MS));
                continue;
            }
        }
    } while (false);
}

uint32_t MDnsSocketListener::OpenSocketV4(ifaddrs *ifa)
{
    sockaddr_in *saddr = reinterpret_cast<sockaddr_in *>(ifa->ifa_addr);
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        NETMGR_EXT_LOG_E("mdns_log socket create failed, errno:[%{public}d]", errno);
        return BOOL_VALUE_FALSE;
    }
    if (InitSocketV4(sock, ifa, MDNS_PORT)) {
        NETMGR_EXT_LOG_E("mdns_log InitSocketV4 failed, errno=[%{public}d]", errno);
        close(sock);
        return BOOL_VALUE_FALSE;
    } else {
        socks_.emplace_back(sock);
        iface_[sock] = ifa->ifa_name;
        reinterpret_cast<sockaddr_in *>(&saddr_[sock])->sin_family = AF_INET;
        reinterpret_cast<sockaddr_in *>(&saddr_[sock])->sin_addr = saddr->sin_addr;
    }
    NETMGR_EXT_LOG_I("mdns_log iface found, ifa_name=[%{public}s]", ifa->ifa_name);
    return BOOL_VALUE_TRUE;
}

inline bool InetAddrV6IsLoopback(const in6_addr *addr6)
{
    return IN6_IS_ADDR_LOOPBACK(addr6);
}

uint32_t MDnsSocketListener::OpenSocketV6(ifaddrs *ifa, bool ipv6Support)
{
    if (!ipv6Support || IN6_IS_ADDR_LOOPBACK(&reinterpret_cast<sockaddr_in6 *>(ifa->ifa_addr)->sin6_addr) ||
        (reinterpret_cast<sockaddr_in6 *>(ifa->ifa_addr)->sin6_scope_id != 0)) {
        NETMGR_EXT_LOG_D("mdns_log ipv6 not supported");
        return BOOL_VALUE_FALSE;
    }
    sockaddr_in6 *saddr = reinterpret_cast<sockaddr_in6 *>(ifa->ifa_addr);
    int sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        NETMGR_EXT_LOG_E("mdns_log socket create failed, errno:[%{public}d]", errno);
        return BOOL_VALUE_FALSE;
    }
    if (InitSocketV6(sock, ifa, MDNS_PORT)) {
        NETMGR_EXT_LOG_E("mdns_log InitSocketV6 failed, errno=[%{public}d]", errno);
        close(sock);
        return BOOL_VALUE_FALSE;
    } else {
        socks_.emplace_back(sock);
        iface_[sock] = ifa->ifa_name;
        reinterpret_cast<sockaddr_in6 *>(&saddr_[sock])->sin6_family = AF_INET6;
        reinterpret_cast<sockaddr_in6 *>(&saddr_[sock])->sin6_addr = saddr->sin6_addr;
    }
    NETMGR_EXT_LOG_I("mdns_log iface found, ifa_name=[%{public}s]", ifa->ifa_name);
    return BOOL_VALUE_TRUE;
}

void MDnsSocketListener::OpenSocketForDefault(bool ipv6Support)
{
    do {
        if (!ipv6Support) {
            int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (socks_.size() < MDNS_MAX_SOCKET && InitSocketV4(sock, nullptr, MDNS_PORT)) {
                close(sock);
                NETMGR_EXT_LOG_E("mdns_log OpenSocjetForDefault ipv4 failed");
                std::this_thread::sleep_for(std::chrono::microseconds(SOCKET_INIT_INTERVAL_MS));
                continue;
            } else {
                socks_.emplace_back(sock);
            }
        } else {
            int sock6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
            if (socks_.size() < MDNS_MAX_SOCKET && InitSocketV6(sock6, nullptr, MDNS_PORT)) {
                close(sock6);
                NETMGR_EXT_LOG_E("mdns_log OpenSocjetForDefault ipv6 failed");
                std::this_thread::sleep_for(std::chrono::milliseconds(SOCKET_INIT_INTERVAL_MS));
                continue;
            } else {
                socks_.emplace_back(sock6);
            }
        }
    } while (false);
}

void MDnsSocketListener::CloseAllSocket()
{
    for (size_t i = 0; i < socks_.size() && i < MDNS_MAX_SOCKET; ++i) {
        close(socks_[i]);
    }
    socks_.clear();
    iface_.clear();
}

void MDnsSocketListener::Run()
{
    while (runningFlag_) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(ctrlPair_[0], &rfds);
        int nfds = ctrlPair_[0] + 1;
        for (size_t i = 0; i < socks_.size(); ++i) {
            FD_SET(socks_[i], &rfds);
            nfds = std::max(nfds, socks_[i] + 1);
        }
        timeval timeout{.tv_sec = 1, .tv_usec = 0};
        int res = select(nfds, &rfds, 0, 0, &timeout);
        if (res < 0) {
            continue;
        }
        if (FD_ISSET(ctrlPair_[0], &rfds)) {
            CanRefresh();
        }
        for (size_t i = 0; i < socks_.size() && i < MDNS_MAX_SOCKET; ++i) {
            if (FD_ISSET(socks_[i], &rfds)) {
                ReceiveInSock(socks_[i]);
            }
        }
        if (static_cast<bool>(finished_)) {
            finished_(ctrlPair_[0]);
        }
    }
    NETMGR_EXT_LOG_I("mdns_log listener stopped");
}

void MDnsSocketListener::ReceiveInSock(int sock)
{
    sockaddr_storage addr{};
    sockaddr *saddr = (sockaddr *)&addr;
    socklen_t addrlen = sizeof(addr);
    MDnsPayload payload(RECV_BUFFER);
    msghdr msg{};
    iovec iov[1];
    cmsghdr *cmptr;
    union {
        cmsghdr cm;
        char control[CMSG_SPACE(sizeof(in6_pktinfo))];
    } control_un;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    msg.msg_flags = 0;
    msg.msg_name = saddr;
    msg.msg_namelen = addrlen;
    iov[0].iov_base = payload.data();
    iov[0].iov_len = payload.size();
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    ssize_t recvLen = recvmsg(sock, &msg, 0);
    if (recvLen <= 0) {
        NETMGR_EXT_LOG_E("mdns_log recvmsg return: [%{public}zd], errno:[%{public}d]", recvLen, errno);
        return;
    }

    int ifIndex = -1;
    for (cmptr = CMSG_FIRSTHDR(&msg); cmptr != nullptr; cmptr = CMSG_NXTHDR(&msg, cmptr)) {
        if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_PKTINFO) {
            ifIndex = reinterpret_cast<in_pktinfo *>(CMSG_DATA(cmptr))->ipi_ifindex;
        }
        if (cmptr->cmsg_level == IPPROTO_IPV6 && cmptr->cmsg_type == IPV6_2292PKTINFO) {
            ifIndex = static_cast<int>(reinterpret_cast<in6_pktinfo *>(CMSG_DATA(cmptr))->ipi6_ifindex);
        }
    }

    char ifName[IFNAMSIZ] = {0};
    if (if_indextoname(static_cast<unsigned>(ifIndex), ifName) == nullptr) {
        NETMGR_EXT_LOG_E("mdns_log if_indextoname failed, errno:[%{public}d]", errno);
    }
    if (ifName == iface_[sock] && recvLen > 0 && recv_) {
        payload.resize(static_cast<size_t>(recvLen));
        recv_(sock, payload);
    }
}

void MDnsSocketListener::TriggerRefresh()
{
    write(ctrlPair_[1], CONTROL_TAG_REFRESH, 1);
}

bool MDnsSocketListener::CanRefresh()
{
    char buf[REFRESH_BUFFER_LEN] = {};
    read(ctrlPair_[0], buf, 1);
    return (std::string_view(buf) == CONTROL_TAG_REFRESH);
}

ssize_t MDnsSocketListener::Multicast(int sock, const MDnsPayload &payload)
{
    const sockaddr *saddrIf = GetSockAddr(sock);
    if (saddrIf == nullptr) {
        NETMGR_EXT_LOG_E("mdns_log GetSockAddr failed");
        return -1;
    }
    NETMGR_EXT_LOG_I("mdns_log Multicast, sock=%{public}d, family=%{public}d", sock, saddrIf->sa_family);
    int ret = -1;
    if (saddrIf->sa_family == AF_INET) {
        in_addr addr;
        addr.s_addr = htonl(MDNS_MULTICAST_INADDR);
        sockaddr_in saddr{.sin_family = AF_INET, .sin_port = htons(MDNS_PORT), .sin_addr = addr};
        ret = sendto(sock, payload.data(), payload.size(), 0, reinterpret_cast<const sockaddr *>(&saddr),
                     sizeof(saddr));
    }
    if (saddrIf->sa_family == AF_INET6) {
        sockaddr_in6 saddr{.sin6_family = AF_INET6, .sin6_port = htons(MDNS_PORT), .sin6_addr = MDNS_MULTICAST_IN6ADDR};
        ret = sendto(sock, payload.data(), payload.size(), 0, reinterpret_cast<const sockaddr *>(&saddr),
                     sizeof(saddr));
    }
    if (ret < 0) {
        NETMGR_EXT_LOG_I("mdns_log sendto error[%{public}d] error info:[%{public}s]", errno, strerror(errno));
    }
    return ret;
}

ssize_t MDnsSocketListener::Unicast(int sock, sockaddr *saddr, const MDnsPayload &payload)
{
    socklen_t saddrLen = 0;
    if (saddr->sa_family == AF_INET) {
        saddrLen = sizeof(sockaddr_in);
    } else if (saddr->sa_family == AF_INET6) {
        saddrLen = sizeof(sockaddr_in6);
    } else {
        return -1;
    }
    return sendto(sock, payload.data(), payload.size(), 0, saddr, saddrLen);
}

ssize_t MDnsSocketListener::MulticastAll(const MDnsPayload &payload)
{
    ssize_t total = 0;
    for (size_t i = 0; i < socks_.size() && i < MDNS_MAX_SOCKET; ++i) {
        ssize_t sendLen = Multicast(socks_[i], payload);
        if (sendLen == -1) {
            return sendLen;
        }
        total += sendLen;
    }
    return total;
}

const std::vector<int> &MDnsSocketListener::GetSockets() const
{
    return socks_;
}

void MDnsSocketListener::SetReceiveHandler(const ReceiveHandler &callback)
{
    recv_ = callback;
}

void MDnsSocketListener::SetFinishedHandler(const FinishedHandler &callback)
{
    finished_ = callback;
}

std::string_view MDnsSocketListener::GetIface(int sock) const
{
    auto i = iface_.find(sock);
    return i == iface_.end() ? std::string_view{} : i->second;
}

const sockaddr *MDnsSocketListener::GetSockAddr(int sock) const
{
    auto i = saddr_.find(sock);
    return i == saddr_.end() ? nullptr : reinterpret_cast<const sockaddr *>(&(i->second));
}
} // namespace NetManagerStandard
} // namespace OHOS
