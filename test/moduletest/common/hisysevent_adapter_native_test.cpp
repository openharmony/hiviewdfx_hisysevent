/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <chrono>
#include <iosfwd>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"

#include "ash_mem_utils.h"
#include "file_util.h"
#include "hilog/log.h"
#include "hisysevent_base_listener.h"
#include "hisysevent_base_query_callback.h"
#include "hisysevent_delegate.h"
#include "hisysevent_listener_proxy.h"
#include "hisysevent_query_proxy.h"
#include "hisysevent_rules.h"
#include "iquery_sys_event_callback.h"
#include "query_argument.h"
#include "query_sys_event_callback_stub.h"
#include "ret_code.h"
#include "string_ex.h"
#include "string_util.h"
#include "sys_event_service_proxy.h"


using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;

namespace {
constexpr char ASH_MEM_NAME[] = "TestSharedMemory";
constexpr int32_t ASH_MEM_SIZE = 1024 * 2; // 2K
constexpr char LOG_DIR_PATH[] = "/data/test/adapter_native_test";
constexpr char FILE_PATH[] = "/data/test/adapter_native_test/test.log";

sptr<Ashmem> GetAshmem()
{
    auto ashmem = Ashmem::CreateAshmem(ASH_MEM_NAME, ASH_MEM_SIZE);
    if (ashmem == nullptr) {
        return nullptr;
    }
    if (!ashmem->MapReadAndWriteAshmem()) {
        return ashmem;
    }
    return ashmem;
}

uint64_t GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

class QuerySysEventCallbackStubTest : public QuerySysEventCallbackStub {
public:
    QuerySysEventCallbackStubTest() {}
    virtual ~QuerySysEventCallbackStubTest() {}

    void OnQuery(const std::vector<std::u16string>& sysEvent, const std::vector<int64_t>& seq) {}
    void OnComplete(int32_t reason, int32_t total, int64_t seq) {}

public:
    enum Code {
        DEFAULT = -1,
        ON_QUERY = 0,
        ON_COMPLETE,
    };
};

class SysEventCallbackStubTest : public SysEventCallbackStub {
public:
    SysEventCallbackStubTest() {}
    virtual ~SysEventCallbackStubTest() {}

    void Handle(const std::u16string& domain, const std::u16string& eventName, uint32_t eventType,
        const std::u16string& eventDetail) {}

public:
    enum Code {
        DEFAULT = -1,
        HANDLE = 0,
    };
};
}

class HiSysEventAdapterNativeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HiSysEventAdapterNativeTest::SetUpTestCase(void)
{
}

void HiSysEventAdapterNativeTest::TearDownTestCase(void)
{
}

void HiSysEventAdapterNativeTest::SetUp(void)
{
}

void HiSysEventAdapterNativeTest::TearDown(void)
{
}

/**
 * @tc.name: TestAshMemory
 * @tc.desc: Ashmemory test
 * @tc.type: FUNC
 * @tc.require: issueI62BDW
 */
HWTEST_F(HiSysEventAdapterNativeTest, TestAshMemory, TestSize.Level1)
{
    MessageParcel data;
    std::vector<std::u16string> src = {
        Str8ToStr16(std::string("0")),
        Str8ToStr16(std::string("1")),
    };
    auto ret = AshMemUtils::WriteBulkData(data, src);
    ASSERT_NE(ret, nullptr);
    std::vector<std::u16string> dest;
    auto ret1 = AshMemUtils::ReadBulkData(data, dest);
    ASSERT_TRUE(ret1);
    ASSERT_EQ(src.size(), dest.size());
    ASSERT_EQ(Str16ToStr8(dest[0]), "0");
    ASSERT_EQ(Str16ToStr8(dest[1]), "1");
    AshMemUtils::CloseAshmem(nullptr);
    ASSERT_TRUE(true);
    AshMemUtils::CloseAshmem(GetAshmem());
    ASSERT_TRUE(true);
}

/**
 * @tc.name: TestHiSysEventDelegateApisWithInvalidInstance
 * @tc.desc: Call Add/Removelistener/SetDebugMode with a HiSysEventDelegate instance directly
 * @tc.type: FUNC
 * @tc.require: issueI62BDW
 */
HWTEST_F(HiSysEventAdapterNativeTest, TestHiSysEventDelegateApisWithInvalidInstance, TestSize.Level1)
{
    std::shared_ptr<OHOS::HiviewDFX::HiSysEventDelegate> delegate =
        std::make_shared<OHOS::HiviewDFX::HiSysEventDelegate>();
    std::thread t([delegate] () {
        delegate->BinderFunc();
    });
    t.detach();
    auto ret = delegate->RemoveListener(nullptr);
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);
    ret = delegate->SetDebugMode(nullptr, true);
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);
    auto listener = std::make_shared<HiSysEventBaseListener>();
    std::vector<ListenerRule> rules;
    ListenerRule listenerRule("DOMAIN", "EVENT_NAME", "TAG", RuleType::WHOLE_WORD);
    rules.emplace_back(listenerRule);
    ret = delegate->AddListener(listener, rules);
    ASSERT_EQ(ret, IPC_CALL_SUCCEED);
    ret = delegate->SetDebugMode(listener, true);
    if (ret == IPC_CALL_SUCCEED) {
        ret = delegate->SetDebugMode(listener, false);
        ASSERT_EQ(ret, IPC_CALL_SUCCEED);
    }
    ret = delegate->RemoveListener(listener);
    ASSERT_EQ(ret, IPC_CALL_SUCCEED);
    long long defaultTimeStap = -1;
    int queryCount = 10;
    struct QueryArg args(defaultTimeStap, defaultTimeStap, queryCount);
    std::vector<QueryRule> queryRules;
    std::vector<std::string> eventNames {"START_ABILITY"};
    QueryRule rule("AAFWK", eventNames);
    queryRules.emplace_back(rule);
    auto baseQuerier = std::make_shared<HiSysEventBaseQueryCallback>();
    ret = delegate->Query(args, queryRules, baseQuerier);
    ASSERT_EQ(ret, IPC_CALL_SUCCEED);
    int64_t currentTime = static_cast<int64_t>(GetMilliseconds());
    auto result = delegate->Subscribe(queryRules);
    ASSERT_GE(std::to_string(result).length(), std::to_string(currentTime).length());
    ret = delegate->Unsubscribe();
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: TestQuerySysEventCallback
 * @tc.desc: QuerySysEventCallbackStub test
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(HiSysEventAdapterNativeTest, TestQuerySysEventCallback, TestSize.Level1)
{
    QuerySysEventCallbackStub* querySysEventCallbackStub = new(std::nothrow) QuerySysEventCallbackStubTest();
    MessageParcel data, reply;
    MessageOption option;
    querySysEventCallbackStub->OnRemoteRequest(QuerySysEventCallbackStubTest::Code::DEFAULT, data, reply, option);
    ASSERT_TRUE(true);
    querySysEventCallbackStub->OnRemoteRequest(QuerySysEventCallbackStubTest::Code::ON_QUERY, data, reply, option);
    ASSERT_TRUE(true);
    querySysEventCallbackStub->OnRemoteRequest(QuerySysEventCallbackStubTest::Code::ON_COMPLETE, data, reply, option);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: TestSysEventCallback
 * @tc.desc: SysEventCallbackStub test
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(HiSysEventAdapterNativeTest, TestSysEventCallback, TestSize.Level1)
{
    SysEventCallbackStub* sysEventCallbackStub = new(std::nothrow) SysEventCallbackStubTest();
    MessageParcel data, reply;
    MessageOption option;
    sysEventCallbackStub->OnRemoteRequest(SysEventCallbackStubTest::Code::DEFAULT, data, reply, option);
    ASSERT_TRUE(true);
    sysEventCallbackStub->OnRemoteRequest(SysEventCallbackStubTest::Code::HANDLE, data, reply, option);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: FileUtilOhosTest001
 * @tc.desc: FileUtil test
 * @tc.type: FUNC
 * @tc.require: SR000I1G43
 */
HWTEST_F(HiSysEventAdapterNativeTest, FileUtilOhosTest001, TestSize.Level3)
{
    auto ret = FileUtil::ForceCreateDirectory(LOG_DIR_PATH);
    ASSERT_TRUE(ret);
    ret = FileUtil::IsDirectory(LOG_DIR_PATH);
    ASSERT_TRUE(ret);
    ret = FileUtil::RemoveDirectory(LOG_DIR_PATH);
    ASSERT_TRUE(ret);
    ret = FileUtil::IsDirectory(LOG_DIR_PATH);
    ASSERT_TRUE(!ret);
    auto result = FileUtil::GetFilePathByDir(LOG_DIR_PATH, "test.log");
    ASSERT_EQ(result, FILE_PATH);
}

/**
 * @tc.name: FileUtilOhosTest002
 * @tc.desc: FileUtil test
 * @tc.type: FUNC
 * @tc.require: SR000I1G43
 */
HWTEST_F(HiSysEventAdapterNativeTest, FileUtilOhosTest002, TestSize.Level3)
{
    auto ret = FileUtil::IsLegalPath("aa/../bb");
    ASSERT_TRUE(!ret);
    ret = FileUtil::IsLegalPath("aa/./bb");
    ASSERT_TRUE(!ret);
    ret = FileUtil::IsLegalPath("aa/bb/");
    ASSERT_TRUE(ret);
    ret = FileUtil::IsLegalPath("aa/bb/cc");
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: MarshallingTAndUnmarshallingTest
 * @tc.desc: Unmarshalling test
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(HiSysEventAdapterNativeTest, MarshallingTAndUnmarshallingTest, TestSize.Level1)
{
    long long defaultTimeStap = -1;
    int queryCount = 10;
    OHOS::HiviewDFX::QueryArgument args(defaultTimeStap, defaultTimeStap, queryCount);
    MessageParcel parcel1;
    auto ret = args.Marshalling(parcel1);
    ASSERT_TRUE(ret);
    QueryArgument* argsPtr = args.Unmarshalling(parcel1);
    ASSERT_NE(argsPtr, nullptr);
    ASSERT_EQ(argsPtr->maxEvents, 10); // 10 is a expcted test value
    ASSERT_EQ(argsPtr->beginTime, -1); // -1 is a expcted test value
    OHOS::HiviewDFX::SysEventRule sysEventRule("DOMAIN", "EVENT_NAME", "TAG", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    MessageParcel parcel2;
    ret = sysEventRule.Marshalling(parcel2);
    ASSERT_TRUE(ret);
    OHOS::HiviewDFX::SysEventRule* sysEventRulePtr = sysEventRule.Unmarshalling(parcel2);
    ASSERT_NE(sysEventRulePtr, nullptr);
    ASSERT_EQ(sysEventRulePtr->domain, "DOMAIN");
    ASSERT_EQ(sysEventRulePtr->eventName, "EVENT_NAME");
    ASSERT_EQ(sysEventRulePtr->tag, "TAG");
    std::vector<std::string> eventNames { "EVENT_NAME1", "EVENT_NAME2" };
    OHOS::HiviewDFX::SysEventQueryRule queryRule("DOMAIN", eventNames);
    MessageParcel parcel3;
    ret = queryRule.Marshalling(parcel3);
    ASSERT_TRUE(ret);
    OHOS::HiviewDFX::SysEventQueryRule* queryRulePtr = queryRule.Unmarshalling(parcel3);
    ASSERT_NE(queryRulePtr, nullptr);
    ASSERT_EQ(queryRulePtr->domain, "DOMAIN");;
    ASSERT_EQ(queryRulePtr->eventList.size(), 2); // 2 is a expcted test value
    ASSERT_EQ(queryRulePtr->eventList[0], "EVENT_NAME1");
}

/**
 * @tc.name: CStringUtilTest
 * @tc.desc: Test methods which defined in namespace StringUtil
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(HiSysEventAdapterNativeTest, CStringUtilTest, TestSize.Level1)
{
    char dest[100] {}; // 100 is a test length
    std::string src = "01234567";

    auto ret = StringUtil::CopyCString(dest, src, 3); // 3 is a test length
    ASSERT_EQ(ret, -1); // -1 is a expected result
    ret = StringUtil::CopyCString(dest, src, 10); // 3 is a test length
    ASSERT_NE(ret, -1); // -1 is a expected result
    char* dest2p = dest;
    char** dest2pp = &dest2p;
    ret = StringUtil::CreateCString(dest2pp, src, 3); // 3 is a test length
    ASSERT_EQ(ret, -1); // -1 is a expected result
    ret = StringUtil::CreateCString(dest2pp, src, 10); // 3 is a test length
    ASSERT_NE(ret, -1); // -1 is a expected result
    ret = StringUtil::ConvertCString(src, dest2pp, 3); // 3 is a test length
    ASSERT_EQ(ret, -1); // -1 is a expected result
    ret = StringUtil::ConvertCString(src, dest2pp, 10); // 3 is a test length
    ASSERT_NE(ret, -1); // -1 is a expected result
    char v3[10][100] {};
    char* dest3p = v3[0];
    char** dest3pp = &dest3p;
    char*** dest3ppp = &dest3pp;
    std::vector<std::string> srcs1 = {};
    std::vector<std::string> srcs2 = {
        "01234567",
        "01234567",
    };
    size_t len;
    ret = StringUtil::ConvertCStringVec(srcs1, dest3ppp, len);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(len, 0);
    ret = StringUtil::ConvertCStringVec(srcs2, dest3ppp, len);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(len, 2); // 2 is a expected length
    char dest4[3] = {'0', '1', '2'};
    StringUtil::MemsetSafe(reinterpret_cast<void*>(dest4), 3); // 3 is a test length
    ASSERT_EQ(dest4[0], 0);
    ASSERT_EQ(dest4[1], 0); // 1 is a test index
    ASSERT_EQ(dest4[2], 0); // 2 is a test index
}

/**
 * @tc.name: HiSysEventListenerProxyTest
 * @tc.desc: Test apis of HiSysEventListenerProxy
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(HiSysEventAdapterNativeTest, HiSysEventListenerProxyTest, TestSize.Level1)
{
    auto baseListener = std::make_shared<HiSysEventBaseListener>();
    HiSysEventListenerProxy proxy(baseListener);
    proxy.Handle(Str8ToStr16(std::string("DOMAIN")), Str8ToStr16(std::string("EVENT_NAME")), 0,
        Str8ToStr16(std::string("{}")));
    auto listener = proxy.GetEventListener();
    ASSERT_NE(listener, nullptr);
    auto deathRecipient = proxy.GetCallbackDeathRecipient();
    ASSERT_NE(deathRecipient, nullptr);
    if (deathRecipient != nullptr) {
        deathRecipient->OnRemoteDied(nullptr);
        ASSERT_NE(deathRecipient->GetEventListener(), nullptr);
    }
}

/**
 * @tc.name: HiSysEventQueryProxyTest
 * @tc.desc: Test apis of HiSysEventQueryProxy
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(HiSysEventAdapterNativeTest, HiSysEventQueryProxyTest, TestSize.Level1)
{
    auto baseQuerier = std::make_shared<HiSysEventBaseQueryCallback>();
    HiSysEventQueryProxy proxy(baseQuerier);
    std::vector<std::u16string> sysEvent {};
    std::vector<int64_t> seq {};
    proxy.OnQuery(sysEvent, seq);
    ASSERT_TRUE(true);
    proxy.OnComplete(0, 0, 0);
    ASSERT_TRUE(true);
}