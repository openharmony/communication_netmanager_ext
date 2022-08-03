/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef ETHERNET_CONFIG_H
#define ETHERNET_CONFIG_H

#include <map>
#include <string>
#include <vector>
#include <set>
#include <mutex>

#include "ethernet_dhcp_callback.h"
#include "interface_configuration.h"
#include "net_all_capabilities.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace NetManagerStandard {
class EthernetConfiguration {
public:
    EthernetConfiguration();
    ~EthernetConfiguration();

    bool ReadSysteamConfiguration(std::map<std::string, std::set<NetCap>> &devCaps,
        std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs);
    bool ReadUserConfiguration(std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs);
    bool WriteUserConfiguration(const std::string &iface, sptr<InterfaceConfiguration> &cfg);
    bool ClearAllUserConfiguration();
    bool ConvertToConfiguration(const EthernetDhcpCallback::DhcpResult &dhcpResult,
        sptr<StaticConfiguration> &config);
private:
    std::string ReadJsonFile(const std::string &filePath);
    sptr<InterfaceConfiguration> ConvertJsonToConfiguration(const nlohmann::json &jsonData);
    bool IsDirExist(const std::string &dirPath);
    bool CreateDir(const std::string &dirPath);
    bool DelDir(const std::string &dirPath);
    bool IsFileExist(const std::string &filePath);
    bool ReadFile(const std::string &filePath, std::string &fileContent);
    bool WriteFile(const std::string &filePath, const std::string &fileContent);
    void ParserFileConfig(const std::string &fileContent, std::string &iface, sptr<InterfaceConfiguration> cfg);
    void GenCfgContent(const std::string &iface, sptr<InterfaceConfiguration> cfg, std::string &fileContent);
private:
    std::mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_CONFIG_H