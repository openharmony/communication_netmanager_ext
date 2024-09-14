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

#include "net_event_report.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
// event name
constexpr const char *NET_SHARING_SETUP_FAULT = "NET_SHARING_SETUP_FAULT";
constexpr const char *NET_SHARING_CANCEL_FAULT = "NET_SHARING_CANCEL_FAULT";
constexpr const char *NET_SHARING_TIME_STAT = "NET_SHARING_TIME_STAT";
constexpr const char *NET_VPN_CONNECT_FAULT = "NET_VPN_CONNECT_FAULT";
// event params
constexpr const char *EVENT_KEY_SHARING_TYPE = "SHARING_TYPE";
constexpr const char *EVENT_KEY_OPERATION_TYPE = "OPERATION_TYPE";
constexpr const char *EVENT_KEY_SHARING_COUNT = "SHARING_COUNT";
constexpr const char *EVENT_KEY_ERROR_TYPE = "ERROR_TYPE";
constexpr const char *EVENT_KEY_ERROR_MSG = "ERROR_MSG";
constexpr const char *EVENT_KEY_VPN_LEGACY = "VPN_LEGACY";
constexpr const char *EVENT_KEY_VPN_ERROR_TYPE = "VPN_ERROR_TYPE";
constexpr const char *EVENT_KEY_VPN_ERROR_MSG = "VPN_ERROR_MSG";
constexpr int32_t NETMANAGER_EXT_SUCCESS = 0;
} // namespace

int32_t NetEventReport::SendSetupFaultEvent(const EventInfo &eventInfo)
{
    HiSysEventWrite(HiSysEvent::Domain::NETMANAGER_STANDARD, NET_SHARING_SETUP_FAULT, HiSysEvent::EventType::FAULT,
                    EVENT_KEY_SHARING_TYPE, eventInfo.sharingType, EVENT_KEY_OPERATION_TYPE, eventInfo.operatorType,
                    EVENT_KEY_ERROR_TYPE, eventInfo.errorType, EVENT_KEY_ERROR_MSG, eventInfo.errorMsg);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetEventReport::SendCancleFaultEvent(const EventInfo &eventInfo)
{
    HiSysEventWrite(HiSysEvent::Domain::NETMANAGER_STANDARD, NET_SHARING_CANCEL_FAULT, HiSysEvent::EventType::FAULT,
                    EVENT_KEY_SHARING_TYPE, eventInfo.sharingType, EVENT_KEY_OPERATION_TYPE, eventInfo.operatorType,
                    EVENT_KEY_ERROR_TYPE, eventInfo.errorType, EVENT_KEY_ERROR_MSG, eventInfo.errorMsg);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetEventReport::SendTimeBehaviorEvent(const EventInfo &eventInfo)
{
    HiSysEventWrite(HiSysEvent::Domain::NETMANAGER_STANDARD, NET_SHARING_TIME_STAT, HiSysEvent::EventType::BEHAVIOR,
                    EVENT_KEY_SHARING_COUNT, eventInfo.sharingCount, EVENT_KEY_SHARING_TYPE, eventInfo.sharingType);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetEventReport::SendVpnConnectEvent(const VpnEventInfo &eventInfo)
{
    HiSysEventWrite(HiSysEvent::Domain::NETMANAGER_STANDARD, NET_VPN_CONNECT_FAULT, HiSysEvent::EventType::FAULT,
                    EVENT_KEY_VPN_LEGACY, eventInfo.legacy, EVENT_KEY_OPERATION_TYPE, eventInfo.operatorType,
                    EVENT_KEY_VPN_ERROR_TYPE, eventInfo.errorType, EVENT_KEY_VPN_ERROR_MSG, eventInfo.errorMsg);
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS