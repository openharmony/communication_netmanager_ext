/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef NETMGR_EXT_LOG_WRAPPER_H
#define NETMGR_EXT_LOG_WRAPPER_H

#include "hilog/log.h"
#include <string>
#include <cstring>

namespace OHOS {
namespace NetManagerStandard {
enum class NetMgrExtLogLevel {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

class NetMgrExtLogWrapper {
public:
    static bool JudgeLevel(const NetMgrExtLogLevel &level);

    static void SetLogLevel(const NetMgrExtLogLevel &level)
    {
        level_ = level;
    }

    static const NetMgrExtLogLevel &GetLogLevel()
    {
        return level_;
    }

    static std::string GetBriefFileName(const std::string &file);

private:
    static NetMgrExtLogLevel level_;
};

#ifndef NETMGR_EXT_LOG_TAG
#define NETMGR_EXT_LOG_TAG "NetMgrExtPart"
#endif

#undef LOG_TAG
#define LOG_TAG NETMGR_EXT_LOG_TAG

#define NETMANAGER_EXT_LOG_DOMAIN 0xD0015B0

#undef LOG_DOMAIN
#define LOG_DOMAIN NETMANAGER_EXT_LOG_DOMAIN

#define NETMGR_EXT_DEBUG 1

#define MAKE_FILE_NAME (strrchr(__FILE__, '/') + 1)

#ifdef NETMGR_EXT_DEBUG
#define PRINT_LOG(op, fmt, ...)                                                                               \
    (void)HILOG_IMPL(LOG_CORE, LOG_##op, LOG_DOMAIN, LOG_TAG, "[%{public}s:%{public}d]" fmt,     \
                                    MAKE_FILE_NAME, __LINE__, ##__VA_ARGS__)
#else
#define PRINT_LOG(op, fmt, ...)
#endif

#define NETMGR_EXT_LOG_D(fmt, ...) PRINT_LOG(DEBUG, fmt, ##__VA_ARGS__)
#define NETMGR_EXT_LOG_E(fmt, ...) PRINT_LOG(ERROR, fmt, ##__VA_ARGS__)
#define NETMGR_EXT_LOG_W(fmt, ...) PRINT_LOG(WARN, fmt, ##__VA_ARGS__)
#define NETMGR_EXT_LOG_I(fmt, ...) PRINT_LOG(INFO, fmt, ##__VA_ARGS__)
#define NETMGR_EXT_LOG_F(fmt, ...) PRINT_LOG(FATAL, fmt, ##__VA_ARGS__)
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMGR_EXT_LOG_WRAPPER_H
