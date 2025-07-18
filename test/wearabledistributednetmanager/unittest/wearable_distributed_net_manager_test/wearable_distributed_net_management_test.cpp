/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "net_manager_constants.h"

#define private public
#include "wearable_distributed_net_agent.h"
#include "wearable_distributed_net_management.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t TCP_PORT_ID = 8080;
constexpr int32_t UDP_PORT_ID = 8081;
using namespace testing::ext;
} // namesapce

class WearableDistributedNetManagementTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp();
    void TearDown();
};

void WearableDistributedNetManagementTest::SetUpTestCase() {}

void WearableDistributedNetManagementTest::TearDownTestCase() {}

void WearableDistributedNetManagementTest::SetUp() {}

void WearableDistributedNetManagementTest::TearDown() {}

HWTEST_F(WearableDistributedNetManagementTest, StartWearableDistributedNetNetwork001, TestSize.Level1)
{
    auto ret = WearableDistributedNetManagement::GetInstance()
        .StartWearableDistributedNetwork(TCP_PORT_ID, UDP_PORT_ID, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    WearableDistributedNetManagement::GetInstance().StopWearableDistributedNetwork();
    ret = WearableDistributedNetManagement::GetInstance()
        .StartWearableDistributedNetwork(TCP_PORT_ID, UDP_PORT_ID, true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    WearableDistributedNetManagement::GetInstance().StopWearableDistributedNetwork();
}

HWTEST_F(WearableDistributedNetManagementTest, StartWearableDistributedNetNetwork002, TestSize.Level1)
{
    auto ret = WearableDistributedNetManagement::GetInstance()
        .StartWearableDistributedNetwork(TCP_PORT_ID, UDP_PORT_ID, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    WearableDistributedNetManagement::GetInstance().StopWearableDistributedNetwork();
}

HWTEST_F(WearableDistributedNetManagementTest, StopWearableDistributedNetwork, TestSize.Level1)
{
    auto ret = WearableDistributedNetManagement::GetInstance().StopWearableDistributedNetwork();
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(WearableDistributedNetManagementTest, UpdateNetScore, TestSize.Level1)
{
    bool isCharging = true;
    WearableDistributedNetManagement::GetInstance().UpdateNetScore(isCharging);
    EXPECT_EQ(WearableDistributedNetAgent::GetInstance().score_, NET_SCORE_WITH_CHARGE_STATE);
}

HWTEST_F(WearableDistributedNetManagementTest, EnableWearableDistributedNetwork, TestSize.Level1)
{
    bool enableFlag = false;
    int32_t result = WearableDistributedNetManagement::GetInstance().EnableWearableDistributedNetwork(enableFlag);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
