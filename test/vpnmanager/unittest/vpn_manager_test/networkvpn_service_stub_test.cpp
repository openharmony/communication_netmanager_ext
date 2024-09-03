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

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "mock_networkvpn_service_stub_test.h"
#include "mock_vpn_event_callback_test.h"
#include "sharing_event_callback_stub.h"
#include "net_manager_constants.h"
#include "networkvpn_service_stub.h"
#include "netmanager_ext_test_security.h"
#ifdef SUPPORT_SYSVPN
#include "ipsecvpn_config.h"
#endif // SUPPORT_SYSVPN

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class NetworkVpnServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
#ifdef SUPPORT_SYSVPN
    void AddSysVpnConfig();
    void DeleteSysVpnConfig();
    void GetSysVpnConfigList();
    void GetSysVpnConfig();
    void GetConnectedSysVpnConfig();
#endif // SUPPORT_SYSVPN
    static inline std::shared_ptr<NetworkVpnServiceStub> instance_ = std::make_shared<MockNetworkVpnServiceStub>();
    static int32_t SendRemoteRequest(MessageParcel &data, INetworkVpnService::MessageCode code);
};

void NetworkVpnServiceStubTest::SetUpTestCase() {}

void NetworkVpnServiceStubTest::TearDownTestCase() {}

void NetworkVpnServiceStubTest::SetUp() {}

void NetworkVpnServiceStubTest::TearDown() {}

#ifdef SUPPORT_SYSVPN
void NetworkVpnServiceStubTest::AddSysVpnConfig() {}

void NetworkVpnServiceStubTest::DeleteSysVpnConfig() {}

void NetworkVpnServiceStubTest::GetSysVpnConfigList() {}

void NetworkVpnServiceStubTest::GetSysVpnConfig() {}

void NetworkVpnServiceStubTest::GetConnectedSysVpnConfig() {}
#endif // SUPPORT_SYSVPN

int32_t NetworkVpnServiceStubTest::SendRemoteRequest(MessageParcel &data, INetworkVpnService::MessageCode code)
{
    MessageParcel reply;
    MessageOption option;
    return instance_->OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyPrepareTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_PREPARE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplySetUpVpnTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<VpnConfig> config = new (std::nothrow) VpnConfig();
    EXPECT_TRUE(config != nullptr);
    if (config == nullptr) {
        return;
    }
    config->isAcceptIPv4_ = false;
    config->isAcceptIPv6_ = false;
    config->isLegacy_ = false;
    config->isMetered_ = false;
    config->isBlocking_ = false;
    if (!config->Marshalling(data)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_START_VPN);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyProtectTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_PROTECT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyDestroyVpnTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_STOP_VPN);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyRegisterVpnEventTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<IVpnEventCallback> callback = new (std::nothrow) MockIVpnEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_REGISTER_EVENT_CALLBACK);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyUnregisterVpnEventTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<IVpnEventCallback> callback = new (std::nothrow) MockIVpnEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_UNREGISTER_EVENT_CALLBACK);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyCreateVpnConnectionTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_CREATE_VPN_CONNECTION);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyCreateVpnConnectionTest002, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_CREATE_VPN_CONNECTION_EXT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyRegisterSharingEventTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<IVpnEventCallback> callback = new (std::nothrow) MockIVpnEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_REGISTER_EVENT_CALLBACK);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyUnregisterSharingEventTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<IVpnEventCallback> callback = new (std::nothrow) MockIVpnEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_UNREGISTER_EVENT_CALLBACK);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyFactoryResetVpnTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_FACTORYRESET_VPN);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplySetUpVpnExtTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_START_VPN_EXT);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyProtectExtTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_PROTECT_EXT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyDestroyVpnExtTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_STOP_VPN_EXT);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyRegisterBundleNameVpnTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<IVpnEventCallback> callback = new (std::nothrow) MockIVpnEventCallback();
    if (!data.WriteRemoteObject(callback->AsObject())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_REGISTER_BUNDLENAME);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyDefaultTest001, TestSize.Level1)
{
    NetManagerExtAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    uint32_t code = static_cast<uint32_t> (INetworkVpnService::MessageCode::CMD_REGISTER_BUNDLENAME) +
        static_cast<uint32_t> (INetworkVpnService::MessageCode::CMD_REGISTER_BUNDLENAME);
    int32_t ret = SendRemoteRequest(data, static_cast<INetworkVpnService::MessageCode>(code));
    EXPECT_NE(ret, NETMANAGER_EXT_SUCCESS);
}
#ifdef SUPPORT_SYSVPN
HWTEST_F(NetworkVpnServiceStubTest, ReplyAddSysVpnConfigTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_ADD_SYS_VPN_CONFIG);
    // config is null
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyAddSysVpnConfigTest002, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<SysVpnConfig> config = new (std::nothrow) IpsecVpnConfig();
    if (config == nullptr) {
        retrun;
    }
    config->vpnId_ = "1234";
    config->vpnName_ = "test";
    config->vpnType_ = 1;
    if (!config->Marshalling(data)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_ADD_SYS_VPN_CONFIG);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyDeleteSysVpnConfigTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_DELETE_SYS_VPN_CONFIG);
    // vpnId is null
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyDeleteSysVpnConfigTest002, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    std::string vpnId = "1234";
    if (!data.WriteString(vpnId)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_DELETE_SYS_VPN_CONFIG);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyGetSysVpnConfigListTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_GET_SYS_VPN_CONFIG_LIST);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyGetSysVpnConfigTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_GET_SYS_VPN_CONFIG);
    // vpnId is null
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_READ_DATA_FAIL);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyGetSysVpnConfigTest002, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    std::string vpnId = "1234";
    if (!data.WriteString(vpnId)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_GET_SYS_VPN_CONFIG);
    // vpnId is null
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyGetConnectedSysVpnConfigTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_GET_CONNECTED_SYS_VPN_CONFIG);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(NetworkVpnServiceStubTest, ReplyNotifyConnectStageTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetworkVpnServiceStub::GetDescriptor())) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, INetworkVpnService::MessageCode::CMD_NOTIFY_CONNECT_STAGE);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}
#endif // SUPPORT_SYSVPN
} // namespace NetManagerStandard
} // namespace OHOS
