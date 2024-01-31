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
#include "ret_code.h"
#include "securec.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int TEST_WROTE_EVENT_SIZE = 5;
constexpr int TEST_RULE_CNT = 1;
constexpr int RECORDS_CNT = 1;
constexpr int EXT_BYTE_CNT = 1;

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

HiSysEventWatchRule CreateWatchRule(const std::string& domain, const std::string& name, const std::string& tag)
{
    HiSysEventWatchRule rule;
    if (memcpy_s(rule.domain, MAX_LENGTH_OF_EVENT_DOMAIN, domain.c_str(), domain.length() + EXT_BYTE_CNT) != EOK ||
        memcpy_s(rule.name, MAX_LENGTH_OF_EVENT_NAME, name.c_str(), name.length() + EXT_BYTE_CNT) != EOK ||
        memcpy_s(rule.tag, MAX_LENGTH_OF_EVENT_TAG, tag.c_str(), tag.length() + EXT_BYTE_CNT) != EOK) {
        return rule;
    }
    rule.ruleType = 1; // 1 means whole_word
    rule.eventType = 1; // 1 means event type is fault
    return rule;
}

HiSysEventQueryRuleWrapper CreateQueryRule(const std::string& domain, const std::string& name,
    const std::string& condition)
{
    HiSysEventQueryRuleWrapper rule;
    if (memcpy_s(rule.domain, MAX_LENGTH_OF_EVENT_DOMAIN, domain.c_str(), domain.length() + EXT_BYTE_CNT) != EOK ||
        memcpy_s(rule.eventList, MAX_EVENT_LIST_LEN, name.c_str(), name.length() + EXT_BYTE_CNT) != EOK) {
        return rule;
    }
    rule.eventListSize = name.length();
    rule.condition = const_cast<char*>(condition.c_str());
    return rule;
}

HiSysEventRecordC CreateSysEventRecord(const std::string& domain, const std::string& name,
    const std::string& timeZone)
{
    HiSysEventRecordC record;
    if (memcpy_s(record.domain, MAX_LENGTH_OF_EVENT_DOMAIN, domain.c_str(), domain.length() + EXT_BYTE_CNT) != EOK ||
        memcpy_s(record.eventName, MAX_LENGTH_OF_EVENT_NAME, name.c_str(), name.length() + EXT_BYTE_CNT) != EOK ||
        memcpy_s(record.tz, MAX_LENGTH_OF_TIME_ZONE, timeZone.c_str(), timeZone.length() + EXT_BYTE_CNT) != EOK) {
        return record;
    }
    record.type = 1; // 1 means event type is fault
    record.time = 0; // 0 is a test timestamp
    return record;
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
        if (i == 0) {
            wrapper.paramType = HISYSEVENT_STRING;
        } else {
            wrapper.paramType = HISYSEVENT_UINT64;
        }
        HiSysEventParamValue value;
        if (i == 0) {
            value.s = new char[5]; // 5 is a random length
        } else {
            value.ui64 = 18;
        }
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
    const HiSysEventWatchRule rules[TEST_RULE_CNT] = {
        CreateWatchRule("KERNEL_VENDOR", "POWER_KEY", "")
    };
    auto watchRet = HiSysEventAddWatcherWrapper(watcher, rules, TEST_RULE_CNT);
    ASSERT_EQ(watchRet, SUCCESS);
    watchRet = HiSysEventRemoveWatcherWrapper(watcher);
    ASSERT_EQ(watchRet, SUCCESS);
    watchRet = HiSysEventRemoveWatcherWrapper(watcher);
    ASSERT_EQ(watchRet, ERR_LISTENER_NOT_EXIST);
    RecycleRustEventWatcher(watcher);
}

/**
 * @tc.name: HiSysEventCWrapperUnitTest003
 * @tc.desc: Test GetHiSysEventRecordByIndexWrapper
 * @tc.type: FUNC
 * @tc.require: issueI8YWH1
 */
HWTEST_F(HiSysEventCWrapperUnitTest, HiSysEventCWrapperUnitTest003, testing::ext::TestSize.Level3)
{
    const std::string testDomain = "KERNEL_VENDOR";
    const std::string testEventName = "POWER_KEY";
    const HiSysEventRecordC records[RECORDS_CNT] = {
        CreateSysEventRecord(testDomain, testEventName, "+0800")
    };
    HiSysEventRecordC recordRet = GetHiSysEventRecordByIndexWrapper(records, RECORDS_CNT, 0);
    ASSERT_EQ(recordRet.domain, testDomain);
    ASSERT_EQ(recordRet.eventName, testEventName);
    recordRet = GetHiSysEventRecordByIndexWrapper(records, RECORDS_CNT, RECORDS_CNT);
    ASSERT_TRUE(recordRet.domain != testDomain);
    ASSERT_TRUE(recordRet.eventName != testEventName);
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
        .beginTime = -1, // -1 means querying without beginning time limit
        .endTime = -1,   // -1 means querying without ending time limit
        .maxEvents = 10, // query 10 events
    };
    const HiSysEventQueryRuleWrapper rules[TEST_RULE_CNT] = {
        CreateQueryRule("KERNEL_VENDOR", "POWER_KEY", "")
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
    std::string eventStrListeral = "{\"domain_\": \"DEMO\", \"name_\": \"EVENT_NAME_A\", \"type_\": 4,\
        \"PARAM_A\": 3.4, \"UINT64_T\": 18446744073709551610, \"DOUBLE_T\": 3.3, \"INT64_T\": 9223372036854775800,\
        \"PARAM_B\": [\"123\", \"456\", \"789\"], \"PARAM_C\": []}";
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
    std::string eventStrListeral = "{\"domain_\": \"DEMO\", \"name_\": \"EVENT_NAME_A\", \"type_\": 4,\
        \"PARAM_A\": 3.4, \"UINT64_T\": 18446744073709551611, \"DOUBLE_T\": 3.3, \"INT64_T\": 9223372036854775801,\
        \"PARAM_B\": [\"123\", \"456\", \"789\"], \"PARAM_C\": []}";
    auto event = std::make_shared<OHOS::HiviewDFX::HiSysEventRecord>(eventStrListeral);
    listerner2.OnEvent(event);
    listerner2.OnServiceDied();
    ASSERT_TRUE(event != nullptr);
}

/**
 * @tc.name: HiSysEventCWrapperUnitTest007
 * @tc.desc: Test APIs of HiSysEventRustQuerier with invalid querier
 * @tc.type: FUNC
 * @tc.require: issueI8ZXDD
 */
HWTEST_F(HiSysEventCWrapperUnitTest, HiSysEventCWrapperUnitTest007, testing::ext::TestSize.Level3)
{
    HiSysEventQueryArg quertArg {
        .beginTime = -1, // -1 means querying without beginning time limit
        .endTime = -1,   // -1 means querying without ending time limit
        .maxEvents = 10, // query 10 events
    };
    const HiSysEventQueryRuleWrapper rules[TEST_RULE_CNT] = {
        CreateQueryRule("", "", "")
    };
    HiSysEventRustQuerierC* querier = CreateRustEventQuerier(reinterpret_cast<const void*>(TestCallback),
        TestOnQueryWrapperCb, reinterpret_cast<const void*>(TestCallback), TestOnCompleteWrapperCb);
    auto queryRet = HiSysEventQueryWrapper(&quertArg, rules, TEST_RULE_CNT, querier);
    sleep(3);
    ASSERT_EQ(queryRet, ERR_QUERY_RULE_INVALID);
    HiSysEventRustQuerierC* invalidQuerier = CreateRustEventQuerier(nullptr, nullptr, nullptr, nullptr);
    ASSERT_TRUE(invalidQuerier == nullptr);
    queryRet = HiSysEventQueryWrapper(&quertArg, rules, TEST_RULE_CNT, invalidQuerier);
    ASSERT_EQ(queryRet, ERR_LISTENER_NOT_EXIST);
}

/**
 * @tc.name: HiSysEventCWrapperUnitTest008
 * @tc.desc: Test APIs of HiSysEventRustListener with invlaid watcher
 * @tc.type: FUNC
 * @tc.require: issueI8ZXDD
 */
HWTEST_F(HiSysEventCWrapperUnitTest, HiSysEventCWrapperUnitTest008, testing::ext::TestSize.Level3)
{
    HiSysEventRustWatcherC* watcher = CreateRustEventWatcher(nullptr, nullptr, nullptr, nullptr);
    ASSERT_TRUE(watcher == nullptr);
    const HiSysEventWatchRule rules[TEST_RULE_CNT] = {
        CreateWatchRule("", "", "")
    };
    auto watchRet = HiSysEventAddWatcherWrapper(watcher, rules, TEST_RULE_CNT);
    ASSERT_EQ(watchRet, ERR_LISTENER_NOT_EXIST);
    watchRet = HiSysEventRemoveWatcherWrapper(watcher);
    ASSERT_EQ(watchRet, ERR_LISTENER_NOT_EXIST);
    watchRet = HiSysEventRemoveWatcherWrapper(nullptr);
    ASSERT_EQ(watchRet, ERR_LISTENER_NOT_EXIST);
    RecycleRustEventWatcher(watcher);
    watchRet = HiSysEventRemoveWatcherWrapper(watcher);
    ASSERT_EQ(watchRet, ERR_LISTENER_NOT_EXIST);
}
} // HiviewDFX
} // OHOS