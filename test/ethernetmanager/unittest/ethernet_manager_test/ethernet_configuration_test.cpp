/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"

#define private public
#include "ethernet_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *IFACE = "iface0";
std::string IFACE_NAME = "iface0";
constexpr const char *FILE_PATH = "./BUILD.gn";
constexpr const char *DIR_PATH = "./BUILD.gn";
std::string REAL_PATH = "./BUILD.gn";
std::string FILE_CONTENT = "./BUILD.gn";
} // namespace

class EthernetConfigurationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void EthernetConfigurationTest::SetUpTestCase() {}

void EthernetConfigurationTest::TearDownTestCase() {}

void EthernetConfigurationTest::SetUp() {}

void EthernetConfigurationTest::TearDown() {}

HWTEST_F(EthernetConfigurationTest, EthernetConfiguration001, TestSize.Level1)
{
    EthernetConfiguration ethernetConfiguration;
    std::map<std::string, std::set<NetCap>> devCaps;
    std::map<std::string, sptr<InterfaceConfiguration>> devCfgs;
    bool ret = ethernetConfiguration.ReadSysteamConfiguration(devCaps, devCfgs);
    EXPECT_TRUE(ret);
    ret = ethernetConfiguration.ReadUserConfiguration(devCfgs);
    EXPECT_TRUE(ret);
    sptr<InterfaceConfiguration> cfg = (std::make_unique<InterfaceConfiguration>()).release();
    ret = ethernetConfiguration.WriteUserConfiguration(IFACE, cfg);
    EXPECT_TRUE(ret);
    ret = ethernetConfiguration.ClearAllUserConfiguration();
    EXPECT_TRUE(ret);
    EthernetDhcpCallback::DhcpResult dhcpResult;
    sptr<StaticConfiguration> config = (std::make_unique<StaticConfiguration>()).release();
    ret = ethernetConfiguration.ConvertToConfiguration(dhcpResult, config);
    EXPECT_TRUE(ret);
    std::string strRet = ethernetConfiguration.ReadJsonFile(FILE_PATH);
    EXPECT_NE(strRet, "./BUILD.gn");
    ret = ethernetConfiguration.IsDirExist(DIR_PATH);
    EXPECT_FALSE(ret);
    ret = ethernetConfiguration.CreateDir(DIR_PATH);
    EXPECT_TRUE(ret);
    ret = ethernetConfiguration.DelDir(DIR_PATH);
    EXPECT_TRUE(ret);
    ret = ethernetConfiguration.IsFileExist(FILE_PATH, REAL_PATH);
    EXPECT_FALSE(ret);
    ret = ethernetConfiguration.ReadFile(FILE_PATH, FILE_CONTENT);
    EXPECT_FALSE(ret);
    ret = ethernetConfiguration.WriteFile(FILE_PATH, FILE_CONTENT);
    EXPECT_TRUE(ret);
    ethernetConfiguration.ParserFileConfig(FILE_CONTENT, IFACE_NAME, cfg);
    ethernetConfiguration.GenCfgContent(IFACE, cfg, FILE_CONTENT);
}
} // namespace NetManagerStandard
} // namespace OHOS