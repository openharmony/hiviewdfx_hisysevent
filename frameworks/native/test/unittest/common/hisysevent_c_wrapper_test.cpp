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

#include "hisysevent_c_wrapper_test.h"

#include "def.h"
#include "hisysevent_c_wrapper.h"
#include "hisysevent_rust_listener.h"
#include "hisysevent_rust_manager.h"
#include "hisysevent_rust_querier.h"
#include "securec.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int TEST_WROTE_EVENT_SIZE = 5;
constexpr int TEST_RULE_CNT = 1;
constexpr int RECORDS_CNT = 1;

void TestCallback()
{
    // do nothing
}

void TestOnEventWrapperCb(OnRustCb callback, HiSysEventRecordC record)
{
    // do nothing
}

void TestOnServiceDiedWrapperCb(OnRustCb callback)
{
    // do nothing
}

void TestOnQueryWrapperCb(OnRustCb callback, HiSysEventRecordC* events, unsigned int size)
{
    // do nothing
}

void TestOnCompleteWrapperCb(OnRustCb callback, int reason, int total)
{
    // do nothing
}
}
void HiSysEventCWrapperUnitTest::SetUpTestCase() {}

void HiSysEventCWrapperUnitTest::TearDownTestCase() {}

void HiSysEventCWrapperUnitTest::SetUp() {}

void HiSysEventCWrapperUnitTest::TearDown() {}

/**
 * @tc.name: HiSysEventCWrapperUnitTest001
 * @tc.desc: Test APIs of defined in hisysevent_c_wrapper.h head file
 * @tc.type: FUNC
 * @tc.require: issueI8YWH1
 */
HWTEST_F(HiSysEventCWrapperUnitTest, HiSysEventCWrapperUnitTest001, testing::ext::TestSize.Level3)
{
    const std::string testKey = "TEST_KEY";
    HiSysEventParamWrapper wroteEvents[TEST_WROTE_EVENT_SIZE];
    for (int i = 0; i < TEST_WROTE_EVENT_SIZE; ++i) {
        HiSysEventParamWrapper wrapper;
        auto ret = memcpy_s(wrapper.paramName, MAX_LENGTH_OF_PARAM_NAME, testKey.c_str(),
            testKey.length() + 1); // copy length + 1 bytes
        if (ret != EOK) {
            ASSERT_TRUE(false);
        }
        wrapper.paramType = HISYSEVENT_UINT64;
        HiSysEventParamValue value;
        value.ui64 = 18; // 18 is a test value
        wrapper.paramValue = value;
        wrapper.arraySize = 0; // 0 means value is not an array
        wroteEvents[i] = wrapper;
    }
    auto ret = HiSysEventWriteWrapper("TestFuncName", 1000, "KERNEL_VENDOR", "POWER_KEY", 1,
        wroteEvents, TEST_WROTE_EVENT_SIZE); // test code line number
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: HiSysEventCWrapperUnitTest002
 * @tc.desc: Test APIs of HiSysEventRustListener
 * @tc.type: FUNC
 * @tc.require: issueI8YWH1
 */
HWTEST_F(HiSysEventCWrapperUnitTest, HiSysEventCWrapperUnitTest002, testing::ext::TestSize.Level3)
{
    HiSysEventRustWatcherC* watcher = CreateRustEventWatcher(reinterpret_cast<const void*>(TestCallback),
        TestOnEventWrapperCb, reinterpret_cast<const void*>(TestCallback), TestOnServiceDiedWrapperCb);

    const std::string testDomain = "KERNEL_VENDOR";
    const std::string testEventName = "POWER_KEY";
    const std::string testTag = "";
    HiSysEventWatchRule rule;
    auto ret = memcpy_s(rule.domain, MAX_LENGTH_OF_EVENT_DOMAIN, testDomain.c_str(),
        testDomain.length() + 1); // copy length + 1 bytes
    if (ret != EOK) {
        ASSERT_TRUE(false);
    }
    ret = memcpy_s(rule.name, MAX_LENGTH_OF_EVENT_NAME, testEventName.c_str(),
        testEventName.length() + 1); // copy length + 1 bytes
    if (ret != EOK) {
        ASSERT_TRUE(false);
    }
    ret = memcpy_s(rule.tag, MAX_LENGTH_OF_EVENT_TAG, testTag.c_str(),
        testTag.length() + 1); // copy length + 1 bytes
    if (ret != EOK) {
        ASSERT_TRUE(false);
    }
    rule.ruleType = 1; // 1 means whole_word
    rule.eventType = 1; // 1 means event type is fault
    const HiSysEventWatchRule rules[TEST_RULE_CNT] = {
        rule
    };

    auto watchRet = HiSysEventAddWatcherWrapper(watcher, rules, TEST_RULE_CNT);
    ASSERT_EQ(watchRet, SUCCESS);
    watchRet = HiSysEventRemoveWatcherWrapper(watcher);
    ASSERT_EQ(watchRet, SUCCESS);
}

/**
 * @tc.name: HiSysEventCWrapperUnitTest003
 * @tc.desc: Test GetHiSysEventRecordByIndexWrapper
 * @tc.type: FUNC
 * @tc.require: issueI8YWH1
 */
HWTEST_F(HiSysEventCWrapperUnitTest, HiSysEventCWrapperUnitTest003, testing::ext::TestSize.Level3)
{
    std::string testDomain = "KERNEL_VENDOR";
    std::string testEventName = "POWER_KEY";
    std::string testTimeZone = "+0800";
    HiSysEventRecordC record;
    auto ret = memcpy_s(record.domain, MAX_LENGTH_OF_EVENT_DOMAIN, testDomain.c_str(),
        testDomain.length() + 1); // copy length + 1 bytes
    if (ret != EOK) {
        ASSERT_TRUE(false);
    }
    ret = memcpy_s(record.eventName, MAX_LENGTH_OF_EVENT_NAME, testEventName.c_str(),
        testEventName.length() + 1); // copy length + 1 bytes
    if (ret != EOK) {
        ASSERT_TRUE(false);
    }
    record.type = 1; // 1 means event type is fault
    record.time = 0; // 0 is a test timestamp
    ret = memcpy_s(record.tz, MAX_LENGTH_OF_TIME_ZONE, testTimeZone.c_str(),
        testTimeZone.length() + 1); // copy length + 1 bytes
    const HiSysEventRecordC records[RECORDS_CNT] = {
        record
    };
    HiSysEventRecordC recordRet = GetHiSysEventRecordByIndexWrapper(records, RECORDS_CNT, 0);
    ASSERT_EQ(recordRet.domain, testDomain);
    ASSERT_EQ(recordRet.eventName, testEventName);
}

/**
 * @tc.name: HiSysEventCWrapperUnitTest004
 * @tc.desc: Test APIs of HiSysEventRustQuerier
 * @tc.type: FUNC
 * @tc.require: issueI8YWH1
 */
HWTEST_F(HiSysEventCWrapperUnitTest, HiSysEventCWrapperUnitTest004, testing::ext::TestSize.Level3)
{
    HiSysEventQueryArg quertArg {
        .beginTime = -1, // -1 means no time fit
        .endTime = -1,   // -1 means no time fit
        .maxEvents = 10, // query 10 events
    };

    std::string testDomain = "KERNEL_VENDOR";
    std::string testEventName = "POWER_KEY";
    std::string testCondition = "";
    HiSysEventQueryRuleWrapper rule;
    auto ret = memcpy_s(rule.domain, MAX_LENGTH_OF_EVENT_DOMAIN, testDomain.c_str(),
        testDomain.length() + 1); // copy length + 1 bytes
    if (ret != EOK) {
        ASSERT_TRUE(false);
    }
    ret = memcpy_s(rule.eventList, MAX_EVENT_LIST_LEN, testEventName.c_str(),
        testEventName.length() + 1); // copy length + 1 bytes
    if (ret != EOK) {
        ASSERT_TRUE(false);
    }
    rule.eventListSize = testEventName.length();
    rule.condition = const_cast<char*>(testCondition.c_str());
    const HiSysEventQueryRuleWrapper rules[TEST_RULE_CNT] = {
        rule
    };
    HiSysEventRustQuerierC* querier = CreateRustEventQuerier(reinterpret_cast<const void*>(TestCallback),
        TestOnQueryWrapperCb, reinterpret_cast<const void*>(TestCallback), TestOnCompleteWrapperCb);
    auto queryRet = HiSysEventQueryWrapper(&quertArg, rules, TEST_RULE_CNT, querier);
    sleep(3);
    ASSERT_EQ(queryRet, SUCCESS);
    ASSERT_EQ(querier->status, STATUS_NORMAL);
    RecycleRustEventQuerier(nullptr);
    RecycleRustEventQuerier(querier);
    ASSERT_EQ(querier->status, STATUS_MEM_NEED_RECYCLE);
}

/**
 * @tc.name: HiSysEventCWrapperUnitTest005
 * @tc.desc: Test APIs of HiSysEventRustQuerier under unnormal conditon
 * @tc.type: FUNC
 * @tc.require: issueI8YWH1
 */
HWTEST_F(HiSysEventCWrapperUnitTest, HiSysEventCWrapperUnitTest005, testing::ext::TestSize.Level3)
{
    HiSysEventRustQuerierC* querier = nullptr;
    HiSysEventRustQuerier eventQuerier1(querier);
    eventQuerier1.OnQuery(nullptr);
    eventQuerier1.OnComplete(0, 0); // 0 is a test value
    ASSERT_TRUE(querier == nullptr);
    auto events = std::make_shared<std::vector<OHOS::HiviewDFX::HiSysEventRecord>>();
    ASSERT_TRUE(events != nullptr);
    std::string eventStrListeral = "{\"domain_\": \"DEMO\",\"name_\": \"EVENT_NAME_A\",\"type_\": 4,\
        \"PARAM_A\": 3.4,\"UINT64_T\": 18446744073709551610,\"DOUBLE_T\": 3.3,\"INT64_T\": 9223372036854775800,\
        \"PARAM_B\":[\"123\", \"456\", \"789\"],\"PARAM_C\":[]}";
    OHOS::HiviewDFX::HiSysEventRecord event(eventStrListeral);
    events->emplace_back(event);
    events->emplace_back(event);
    querier = CreateRustEventQuerier(reinterpret_cast<const void*>(TestCallback),
        TestOnQueryWrapperCb, reinterpret_cast<const void*>(TestCallback), TestOnCompleteWrapperCb);
    HiSysEventRustQuerier eventQuerier2(querier);
    eventQuerier2.OnQuery(events);
    eventQuerier2.OnComplete(0, events->size()); // 0 is a test value
    ASSERT_TRUE(querier != nullptr);
}

/**
 * @tc.name: HiSysEventCWrapperUnitTest006
 * @tc.desc: Test APIs of HiSysEventRustListener under unnormal conditon
 * @tc.type: FUNC
 * @tc.require: issueI8YWH1
 */
HWTEST_F(HiSysEventCWrapperUnitTest, HiSysEventCWrapperUnitTest006, testing::ext::TestSize.Level3)
{
    HiSysEventRustWatcherC* watcher = nullptr;
    HiSysEventRustListener listerner1(watcher);
    listerner1.OnEvent(nullptr);
    listerner1.OnServiceDied();
    ASSERT_TRUE(watcher == nullptr);
    watcher = CreateRustEventWatcher(reinterpret_cast<const void*>(TestCallback),
        TestOnEventWrapperCb, reinterpret_cast<const void*>(TestCallback), TestOnServiceDiedWrapperCb);
    HiSysEventRustListener listerner2(watcher);
    listerner2.OnEvent(nullptr);
    ASSERT_TRUE(watcher != nullptr);
    std::string eventStrListeral = "{\"domain_\": \"DEMO\",\"name_\": \"EVENT_NAME_A\",\"type_\": 4,\
        \"PARAM_A\": 3.4,\"UINT64_T\": 18446744073709551610,\"DOUBLE_T\": 3.3,\"INT64_T\": 9223372036854775800,\
        \"PARAM_B\":[\"123\", \"456\", \"789\"],\"PARAM_C\":[]}";
    auto event = std::make_shared<OHOS::HiviewDFX::HiSysEventRecord>(eventStrListeral);
    listerner2.OnEvent(event);
    listerner2.OnServiceDied();
    ASSERT_TRUE(event != nullptr);
}
} // HiviewDFX
} // OHOS