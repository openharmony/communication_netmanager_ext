/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "net_manager_constants.h"
#include "router_advertisement_daemon.h"
#include <csignal>
#include <net/if.h>
#include <sys/time.h>

namespace OHOS {
namespace NetManagerStandard {
namespace {
/** https://www.rfc-editor.org/rfc/rfc4861#section-6.2.1
 * MaxRtrAdvInterval: MUST be no less than 4 seconds and no greater than 1800 seconds.
 *                 Default: 600 seconds
 * MinRtrAdvInterval: MUST be no less than 3seconds and no greater than 0.75 * MaxRtrAdvInterval.
 *                 Default: 0.33 * MaxRtrAdvInterval If
 *                 MaxRtrAdvInterval >= 9 seconds; otherwise, the
 *                 Default is MaxRtrAdvInterval.
 */
constexpr uint32_t MAX_URGENT_RTR_ADVERTISEMENTS = 10;
constexpr uint32_t RECV_RS_TIMEOUT = 1;
constexpr uint32_t SEND_RA_INTERVAL = 3000;
constexpr size_t RA_HEADER_SIZE = 16;

/**
 * https://www.rfc-editor.org/rfc/rfc4861.html#section-6.1.2-4.2
 * Note: If neither M nor O flags are set, this indicates that no
 * information is available via DHCPv6.
 */
constexpr uint8_t DEFAULT_ROUTER_PRE = 0x08;
constexpr uint8_t PREFIX_INFO_FLAGS = 0xc0;
constexpr uint32_t DEFAULT_HOP_LIMIT = 255;

/**
 * https://tools.ietf.org/html/rfc4861#section-2.3
 * all-nodes multicast address
 *          - the link-local scope address to reach all nodes,
 *            FF02::1.
 */
constexpr const char *DST_IPV6 = "ff02::1";
} // namespace

RouterAdvertisementDaemon::RouterAdvertisementDaemon()
{
    raParams_ = std::make_shared<RaParams>();
}

bool RouterAdvertisementDaemon::IsSocketValid()
{
    return socket_ > 0;
}

void RouterAdvertisementDaemon::HupRaThread()
{
    stopRaThread_ = true;
}

int32_t RouterAdvertisementDaemon::Init(const std::string &ifaceName)
{
    sendRaTimes_ = 0;
    raParams_->name_ = ifaceName;
    raParams_->index_ = if_nametoindex(ifaceName.c_str());
    if (memset_s(&dstIpv6Addr_, sizeof(dstIpv6Addr_), 0, sizeof(dstIpv6Addr_)) != EOK) {
        return NETMANAGER_EXT_ERR_MEMSET_FAIL;
    }
    dstIpv6Addr_.sin6_port = 0;
    dstIpv6Addr_.sin6_family = AF_INET6;
    dstIpv6Addr_.sin6_scope_id = 0;
    inet_pton(AF_INET6, DST_IPV6, &dstIpv6Addr_.sin6_addr);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t RouterAdvertisementDaemon::StartRa()
{
    NETMGR_EXT_LOG_I("StartRa");
    if (!CreateRASocket()) {
        NETMGR_EXT_LOG_E("StartRa fail due to socket");
        return NETMANAGER_EXT_ERR_PARAMETER_ERROR;
    }
    stopRaThread_ = false;
    auto sp = shared_from_this();
    recvRsThread_ = std::thread([sp]() { sp->RunRecvRsThread(); });
    pthread_setname_np(recvRsThread_.native_handle(), "OH_Net_RecvRs");
    recvRsThread_.detach();
    ffrtTimer_.Start(SEND_RA_INTERVAL, [sp]() { sp->ProcessSendRaPacket(); });
    return NETMANAGER_EXT_SUCCESS;
}

void RouterAdvertisementDaemon::StopRa()
{
    NETMGR_EXT_LOG_I("StopRa");
    // close timer
    ffrtTimer_.Stop();

    HupRaThread();
}

bool RouterAdvertisementDaemon::CreateRASocket()
{
    NETMGR_EXT_LOG_I("CreateRASocket Start");
    socket_ = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (socket_ < 0) {
        NETMGR_EXT_LOG_E("CreateRASocket fail, errno[%{public}d]", errno);
        return false;
    }
    timeval timeout = {};
    timeout.tv_sec = RECV_RS_TIMEOUT;
    if (setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        NETMGR_EXT_LOG_E("CreateRASocket setsockopt SO_RCVTIMEO fail");
        close(socket_);
        socket_ = -1;
        return false;
    }
    ifreq ifr = {};
    if (strncpy_s(ifr.ifr_name, IFNAMSIZ - 1, raParams_->name_.c_str(), raParams_->name_.size()) != EOK) {
        NETMGR_EXT_LOG_E("CreateRASocket strncopy fail");
        close(socket_);
        socket_ = -1;
        return false;
    }
    if (setsockopt(socket_, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        NETMGR_EXT_LOG_E("CreateRASocket setsockopt SO_BINDTODEVICE fail");
        close(socket_);
        socket_ = -1;
        return false;
    }
    uint32_t hoplimitNew = DEFAULT_HOP_LIMIT;
    if (setsockopt(socket_, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (void *)&hoplimitNew, sizeof(hoplimitNew)) == -1) {
        NETMGR_EXT_LOG_E(" setsockopt IPV6_UNICAST_HOPS fail");
    }
    return true;
}

void RouterAdvertisementDaemon::CloseRaSocket()
{
    NETMGR_EXT_LOG_I("CloseRaSocket Start");
    if (socket_ > 0) {
        close(socket_);
    }
    socket_ = -1;
}

bool RouterAdvertisementDaemon::MaybeSendRa(sockaddr_in6 &dest)
{
    NETMGR_EXT_LOG_D("Send Ra Enter, socket_[%{public}d], raPacketLength_[%{public}hu]", socket_, raPacketLength_);
    if (raPacketLength_ < RA_HEADER_SIZE) {
        NETMGR_EXT_LOG_E("Send Ra failed due to Ra packet length less than RA header size");
        return false;
    }
    if (!IsSocketValid()) {
        NETMGR_EXT_LOG_E("Send Ra failed due to socket invalid");
        return false;
    }
    int ret = sendto(socket_, raPacket_, raPacketLength_, 0, (sockaddr *)&dest, sizeof(dest));
    if (ret < 0) {
        NETMGR_EXT_LOG_E("Send Ra error, ret[%{public}d], errno[%{public}d]", ret, errno);
        return false;
    }
    return true;
}

void RouterAdvertisementDaemon::ProcessSendRaPacket()
{
    if (!IsSocketValid() || stopRaThread_) {
        NETMGR_EXT_LOG_E("socket closed or stopRaThread!");
        return;
    }
    if (AssembleRaLocked()) {
        MaybeSendRa(dstIpv6Addr_);
    }
    ResetRaRetryInterval();
}

void RouterAdvertisementDaemon::RunRecvRsThread()
{
    NETMGR_EXT_LOG_I("Start to receive Rs thread, socket[%{public}d]", socket_);
    sockaddr_in6 solicitor = {};
    uint8_t solicitation[IPV6_MIN_MTU] = {};
    socklen_t sendLen = sizeof(solicitation);
    while (IsSocketValid() && !stopRaThread_) {
        if (memset_s(solicitation, sizeof(solicitation), 0, sizeof(solicitation)) != EOK) {
            break;
        }
        auto rval =
            recvfrom(socket_, solicitation, IPV6_MIN_MTU, 0, reinterpret_cast<sockaddr *>(&solicitor), &sendLen);
        if (rval <= 0 && errno != EAGAIN && errno != EINTR) {
            NETMGR_EXT_LOG_E("recvfrom failed, rval[%{public}zd], errno[%{public}d]", rval, errno);
            break;
        }
        if (solicitation[0] != ICMPV6_ND_ROUTER_SOLICIT_TYPE) {
            continue;
        }
        if (AssembleRaLocked()) {
            MaybeSendRa(solicitor);
        }
    }
    CloseRaSocket();
    raParams_ = nullptr;
}

RaParams RouterAdvertisementDaemon::GetDeprecatedRaParams(RaParams &oldRa, RaParams &newRa)
{
    RaParams deprecateRa = {};
    for (auto ipp : newRa.prefixes_) {
        if (oldRa.ContainsPrefix(ipp)) {
            deprecateRa.prefixes_.emplace_back(ipp);
        }
    }
    for (auto dns : newRa.dnses_) {
        if (oldRa.ContainsDns(dns)) {
            deprecateRa.dnses_.emplace_back(dns);
        }
    }
    return deprecateRa;
}

void RouterAdvertisementDaemon::BuildNewRa(const RaParams &newRa)
{
    raParams_->Set(newRa);
}

void RouterAdvertisementDaemon::ResetRaRetryInterval()
{
    if (sendRaTimes_ < MAX_URGENT_RTR_ADVERTISEMENTS) {
        sendRaTimes_++;
        return;
    }
    if (sendRaTimes_ == MAX_URGENT_RTR_ADVERTISEMENTS) {
        itimerval setvalue = {};
        itimerval oldvalue = {};
        setvalue.it_interval.tv_sec = DEFAULT_RTR_INTERVAL_SEC;
        setvalue.it_value.tv_sec = 1;
        sendRaTimes_++;
        return;
    }
}

bool RouterAdvertisementDaemon::AssembleRaLocked()
{
    NETMGR_EXT_LOG_D("Generate Ra package start");
    uint8_t raBuf[IPV6_MIN_MTU] = {};
    uint8_t *ptr = raBuf;
    uint16_t raHeadLen = PutRaHeader(ptr);
    ptr += raHeadLen;
    uint16_t raSllLen = PutRaSlla(ptr, raParams_->macAddr_);
    ptr += raSllLen;
    uint16_t raMtuLen = PutRaMtu(ptr, raParams_->mtu_);
    ptr += raMtuLen;
    uint16_t raPrefixLens = 0;
    for (IpPrefix ipp : raParams_->prefixes_) {
        uint16_t raPrefixLen = PutRaPio(ptr, ipp);
        ptr += raPrefixLen;
        raPrefixLens += raPrefixLen;
    }
    uint16_t raRdnsLen = PutRaRdnss(ptr);
    ptr += raRdnsLen;
    raPacketLength_ = raHeadLen + raSllLen + raMtuLen + raPrefixLens + raRdnsLen;
    if (memset_s(&raPacket_, sizeof(raPacket_), 0, sizeof(raPacket_)) != EOK) {
        return false;
    }
    if (memcpy_s(raPacket_, sizeof(raPacket_), raBuf, raPacketLength_) != EOK) {
        return false;
    }
    NETMGR_EXT_LOG_D("Generate Ra package end, raPacketLength_: %{public}hu", raPacketLength_);
    return true;
}

uint16_t RouterAdvertisementDaemon::PutRaHeader(uint8_t *raBuf)
{
    NETMGR_EXT_LOG_D("Append Ra header");
    // https://datatracker.ietf.org/doc/html/rfc4861#section-4.2
    // 0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |     Type      |     Code      |          Checksum             |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | Cur Hop Limit |M|O|  Reserved |       Router Lifetime         |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                         Reachable Time                        |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                          Retrans Timer                        |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |   Options ...
    // +-+-+-+-+-+-+-+-+-+-+-+-
    Icmpv6HeadSt raHeadSt;
    raHeadSt.type = ICMPV6_ND_ROUTER_ADVERT_TYPE;
    raHeadSt.curHopLimit = DEFAULT_HOPLIMIT;
    raHeadSt.flags = DEFAULT_ROUTER_PRE;
    raHeadSt.routerLifetime = htons(DEFAULT_LIFETIME);
    if (memcpy_s(raBuf, sizeof(Icmpv6HeadSt), &raHeadSt, sizeof(Icmpv6HeadSt)) != EOK) {
        return 0;
    }
    return static_cast<uint16_t>(sizeof(Icmpv6HeadSt));
}

uint16_t RouterAdvertisementDaemon::PutRaSlla(uint8_t *raBuf, const std::string &mac)
{
    NETMGR_EXT_LOG_D("Append Ra source link lay address");
    if (mac.size() < HW_MAC_STR_LENGTH) {
        return 0;
    }
    // https://datatracker.ietf.org/doc/html/rfc4861#section-4.6.1
    //  0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |     Type      |    Length     |    Link-Layer Address ...
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    Icmpv6SllOpt srcLinkAddrSt;
    srcLinkAddrSt.type = ND_OPTION_SLLA_TYPE;
    srcLinkAddrSt.len = sizeof(Icmpv6SllOpt) / UNITS_OF_OCTETS;
    std::istringstream iss(mac);
    int value = 0;
    for (uint32_t i = 0; i < HW_MAC_LENGTH; i++) {
        iss >> std::hex >> value;
        srcLinkAddrSt.linkAddress[i] = static_cast<uint8_t>(value);
        iss.ignore(1, ':');
    }
    if (memcpy_s(raBuf, sizeof(Icmpv6SllOpt), &srcLinkAddrSt, sizeof(Icmpv6SllOpt)) != EOK) {
        return 0;
    }
    return static_cast<uint16_t>(sizeof(Icmpv6SllOpt));
}

uint16_t RouterAdvertisementDaemon::PutRaMtu(uint8_t *raBuf, int32_t mtu)
{
    NETMGR_EXT_LOG_D("Append Ra Mtu option");
    // https://datatracker.ietf.org/doc/html/rfc4861#section-4.6.4
    //    0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |     Type      |    Length     |           Reserved            |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                              MTU                              |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    Icmpv6MtuOpt mtuSt;
    mtuSt.type = ND_OPTION_MTU_TYPE;
    mtuSt.len = sizeof(Icmpv6MtuOpt) / UNITS_OF_OCTETS;
    mtuSt.mtu = static_cast<uint32_t>(htonl(mtu));
    if (memcpy_s(raBuf, sizeof(Icmpv6MtuOpt), &mtuSt, sizeof(Icmpv6MtuOpt)) != EOK) {
        return 0;
    }
    return static_cast<uint16_t>(sizeof(Icmpv6MtuOpt));
}

uint16_t RouterAdvertisementDaemon::PutRaPio(uint8_t *raBuf, IpPrefix &ipp)
{
    NETMGR_EXT_LOG_D("Append Ra prefix information option");
    // refer to https://tools.ietf.org/html/rfc4861#section-4.6.2
    //    0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |     Type      |    Length     | Prefix Length |L|A| Reserved1 |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                         Valid Lifetime                        |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                       Preferred Lifetime                      |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                           Reserved2                           |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                                                               |
    // +                                                               +
    // |                                                               |
    // +                            Prefix                             +
    // |                                                               |
    // +                                                               +
    // |                                                               |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    Icmpv6PrefixInfoOpt prefixInfoSt;
    prefixInfoSt.type = ND_OPTION_PIO_TYPE;
    prefixInfoSt.len = sizeof(Icmpv6PrefixInfoOpt) / UNITS_OF_OCTETS;
    prefixInfoSt.prefixLen = ipp.prefixesLength;
    prefixInfoSt.flag = PREFIX_INFO_FLAGS;
    prefixInfoSt.validLifetime = htonl(DEFAULT_LIFETIME);
    prefixInfoSt.prefLifetime = htonl(DEFAULT_LIFETIME);
    prefixInfoSt.type = ND_OPTION_PIO_TYPE;
    if (memcpy_s(prefixInfoSt.prefix, IPV6_ADDR_LEN, ipp.prefix.s6_addr, IPV6_ADDR_LEN) != EOK) {
        return 0;
    }
    if (memcpy_s(raBuf, sizeof(Icmpv6PrefixInfoOpt), &prefixInfoSt, sizeof(Icmpv6PrefixInfoOpt)) != EOK) {
        return 0;
    }
    return static_cast<uint16_t>(sizeof(Icmpv6PrefixInfoOpt));
}

uint16_t RouterAdvertisementDaemon::PutRaRdnss(uint8_t *raBuf)
{
    NETMGR_EXT_LOG_D("Make RA DNS server option");
    // https://datatracker.ietf.org/doc/rfc8106/
    //   0                   1                   2                   3
    //   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |     Type      |     Length    |           Reserved            |
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |                           Lifetime                            |
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |                                                               |
    //  :            Addresses of IPv6 Recursive DNS Servers            :
    //  |                                                               |
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    Icmpv6RdnsOpt rdnsInfoSt;
    size_t raRdnsNum = raParams_->prefixes_.size();
    rdnsInfoSt.type = ND_OPTION_RDNSS_TYPE;
    rdnsInfoSt.len = (sizeof(Icmpv6RdnsOpt) + raRdnsNum * IPV6_ADDR_LEN) / UNITS_OF_OCTETS;
    rdnsInfoSt.lifetime = htonl(DEFAULT_LIFETIME);
    if (memcpy_s(raBuf, sizeof(Icmpv6RdnsOpt), &rdnsInfoSt, sizeof(Icmpv6RdnsOpt)) != EOK) {
        return 0;
    }
    raBuf += sizeof(Icmpv6RdnsOpt);
    uint32_t index = 0;
    for (IpPrefix ipp : raParams_->prefixes_) {
        if (memcpy_s(raBuf + index * IPV6_ADDR_LEN, IPV6_ADDR_LEN, ipp.address.s6_addr, IPV6_ADDR_LEN) != EOK) {
            return 0;
        }
        index++;
    }
    return static_cast<uint16_t>(sizeof(Icmpv6RdnsOpt) + raRdnsNum * IPV6_ADDR_LEN);
}
} // namespace NetManagerStandard
} // namespace OHOS
