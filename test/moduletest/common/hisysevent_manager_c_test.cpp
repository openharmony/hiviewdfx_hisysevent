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
#include <chrono>
#include <cinttypes>
#include <cstring>
#include <map>
#include <unistd.h>

#include <gtest/gtest.h>
#include "hilog/log.h"
#include "hisysevent_manager_c.h"
#include "hisysevent_record_c.h"
#include "ret_code.h"
#include "string_util.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_MANAGER_C_TEST"

namespace {
constexpr int64_t MAX_NUM_OF_QUERY = 10;
constexpr size_t MAX_LEN_OF_DOMAIN = 16;
constexpr size_t MAX_LEN_OF_NAME = 32;
constexpr char TEST_DOMAIN[] = "HIVIEWDFX";
constexpr char TEST_NAME[] = "PLUGIN_LOAD";
constexpr uint32_t QUERY_INTERVAL_TIME = 2;
constexpr int ERR_NULL = -1;

int64_t GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

void InitQueryArg(HiSysEventQueryArg& arg)
{
    arg.beginTime = 0;
    arg.endTime = GetMilliseconds();
    arg.maxEvents = MAX_NUM_OF_QUERY;
}

void InitQueryRule(HiSysEventQueryRule& rule)
{
    (void)StringUtil::CopyCString(rule.domain, TEST_DOMAIN, MAX_LEN_OF_DOMAIN);
    (void)StringUtil::CopyCString(rule.eventList[0], TEST_NAME, MAX_LEN_OF_NAME);
    rule.eventListSize = 1;
    rule.condition = nullptr;
}

void InitQueryRuleWithCondition(HiSysEventQueryRule& rule, const std::string& cond)
{
    (void)StringUtil::CopyCString(rule.domain, TEST_DOMAIN, MAX_LEN_OF_DOMAIN);
    (void)StringUtil::CopyCString(rule.eventList[0], TEST_NAME, MAX_LEN_OF_NAME);
    rule.eventListSize = 1;
    (void)StringUtil::CreateCString(&rule.condition, cond);
}

void RecordBaseParamPrint(const HiSysEventRecord& record)
{
    HILOG_DEBUG(LOG_CORE, "event: domain=%{public}s, name=%{public}s, type=%{public}d, tz=%{public}s, "
            "time=%{public}" PRIu64 ", pid=%{public}" PRId64 ", tid=%{public}" PRId64 ", uid=%{public}"
            PRId64 ", traceId=%{public}" PRIu64 ", spandId=%{public}" PRIu64 ", pspanId=%{public}"
            PRIu64 ", level=%{public}s" ", tag=%{public}s",
            record.domain, record.eventName, record.type,
            record.tz, record.time, record.pid, record.tid, record.uid,
            record.traceId, record.spandId, record.pspanId,
            record.level, record.tag == nullptr ? "null" : record.tag);
}

void OnQueryTest(HiSysEventRecord records[], size_t size)
{
    HILOG_INFO(LOG_CORE, "OnQuery: size of records is %{public}zu", size);
    for (size_t i = 0; i < size; i++) {
        HiSysEventRecord record = records[i];
        ASSERT_EQ(strcmp(record.domain, TEST_DOMAIN), 0);
        ASSERT_GT(strlen(record.eventName), 0);
        ASSERT_GT(strlen(record.tz), 0);
        ASSERT_GT(record.type, 0);
        ASSERT_GT(record.time, 0);
        ASSERT_GE(record.pid, 0);
        ASSERT_GE(record.tid, 0);
        ASSERT_GE(record.uid, 0);
        ASSERT_GE(record.traceId, 0);
        ASSERT_GE(record.spandId, 0);
        ASSERT_GE(record.pspanId, 0);
        ASSERT_GT(strlen(record.level), 0);
        if (record.tag != nullptr) {
            ASSERT_GT(strlen(record.tag), 0);
        }
        ASSERT_GT(strlen(record.jsonStr), 0);
        RecordBaseParamPrint(record);
        HILOG_INFO(LOG_CORE, "OnQuery: event=%{public}s", record.jsonStr);
    }
}

void OnCompleteTest(int32_t reason, int32_t total)
{
    HILOG_INFO(LOG_CORE, "OnCompleted, res=%{public}d, total=%{public}d", reason, total);
}

void InitCallback(HiSysEventQueryCallback& callback)
{
    callback.OnQuery = OnQueryTest;
    callback.OnComplete = OnCompleteTest;
}

void OnEventTest(HiSysEventRecordC record)
{
    ASSERT_GT(strlen(record.jsonStr), 0);
    HILOG_INFO(LOG_CORE, "OnEvent: event=%{public}s", record.jsonStr);
}

void OnServiceDiedTest()
{
    HILOG_INFO(LOG_CORE, "OnServiceDied");
}

void InitWatcher(HiSysEventWatcher& watcher)
{
    watcher.OnEvent = OnEventTest;
    watcher.OnServiceDied = OnServiceDiedTest;
}

void QueryTestWithCondition(const std::string& cond)
{
    sleep(QUERY_INTERVAL_TIME); // avoid triggering high frequency queries
    HiSysEventQueryArg arg;
    InitQueryArg(arg);

    HiSysEventQueryRule rule;
    InitQueryRuleWithCondition(rule, cond);
    HiSysEventQueryRule rules[] = { rule };

    HiSysEventQueryCallback callback;
    InitCallback(callback);

    auto res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, 0);
    StringUtil::DeletePointer<char>(&rule.condition);
}

std::string BuildRecordString(const std::map<std::string, std::string>& recordData)
{
    std::string recordStr = "{";
    for (auto& recordParam : recordData) {
        recordStr.append(recordParam.first).append(":").append(recordParam.second).append(",");
    }
    if (recordData.size() > 0) {
        recordStr.pop_back();
    }
    recordStr.append("}");
    return recordStr;
}

void RecordParamNameTest(const HiSysEventRecord& record, const std::map<std::string, std::string>& recordData)
{
    char** params = nullptr;
    size_t len = 0;
    OH_HiSysEvent_GetParamNames(&record, &params, &len);
    ASSERT_EQ(len, recordData.size());
    for (size_t i = 0; i < len; i++) {
        HILOG_DEBUG(LOG_CORE, "param[%{public}zu]=%{public}s", i, params[i]);
        ASSERT_NE(recordData.find("\"" + std::string(params[i]) + "\""), recordData.end());
    }
    StringUtil::DeletePointers<char>(&params, len);
}

void RecordParamIntValueTest(const HiSysEventRecord& record, const std::string& name, int64_t value)
{
    int64_t testValue = 0;
    int res = OH_HiSysEvent_GetParamInt64Value(&record, name.c_str(), &testValue);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(testValue, value);
}

void RecordParamUintValueTest(const HiSysEventRecord& record, const std::string& name, uint64_t value)
{
    uint64_t testValue = 0;
    int res = OH_HiSysEvent_GetParamUint64Value(&record, name.c_str(), &testValue);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(testValue, value);
}

void RecordParamDouValueTest(const HiSysEventRecord& record, const std::string& name, double value)
{
    double testValue = 0;
    int res = OH_HiSysEvent_GetParamDoubleValue(&record, name.c_str(), &testValue);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(testValue, value);
}

void RecordParamStrValueTest(const HiSysEventRecord& record, const std::string& name, const std::string& value)
{
    char* testValue = nullptr;
    int res = OH_HiSysEvent_GetParamStringValue(&record, name.c_str(), &testValue);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(strcmp(testValue, value.c_str()), 0);
    StringUtil::DeletePointer<char>(&testValue);
}

void RecordParamIntValuesTest(const HiSysEventRecord& record, const std::string& name,
    const std::vector<int64_t>& values)
{
    int64_t* testValues = nullptr;
    size_t len = 0;
    int res = OH_HiSysEvent_GetParamInt64Values(&record, name.c_str(), &testValues, &len);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(values.size(), len);
    for (size_t i = 0; i < len; i++) {
        ASSERT_EQ(testValues[i], values[i]);
    }
    StringUtil::DeletePointer<int64_t>(&testValues);
}

void RecordParamUintValuesTest(const HiSysEventRecord& record, const std::string& name,
    const std::vector<uint64_t>& values)
{
    uint64_t* testValues = nullptr;
    size_t len = 0;
    int res = OH_HiSysEvent_GetParamUint64Values(&record, name.c_str(), &testValues, &len);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(values.size(), len);
    for (size_t i = 0; i < len; i++) {
        ASSERT_EQ(testValues[i], values[i]);
    }
    StringUtil::DeletePointer<uint64_t>(&testValues);
}

void RecordParamDouValuesTest(const HiSysEventRecord& record, const std::string& name,
    const std::vector<double>& values)
{
    double* testValues = nullptr;
    size_t len = 0;
    int res = OH_HiSysEvent_GetParamDoubleValues(&record, name.c_str(), &testValues, &len);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(values.size(), len);
    for (size_t i = 0; i < len; i++) {
        ASSERT_EQ(testValues[i], values[i]);
    }
    StringUtil::DeletePointer<double>(&testValues);
}

void RecordParamStrValuesTest(const HiSysEventRecord& record, const std::string& name,
    const std::vector<std::string>& values)
{
    char** testValues = nullptr;
    size_t len = 0;
    int res = OH_HiSysEvent_GetParamStringValues(&record, name.c_str(), &testValues, &len);
    ASSERT_EQ(res, 0);
    for (size_t i = 0; i < len; i++) {
        ASSERT_EQ(strcmp(testValues[i], values[i].c_str()), 0);
    }
    StringUtil::DeletePointers<char>(&testValues, len);
}

void RecordParamNameInvalidTest(const HiSysEventRecord& record)
{
    char** params = nullptr;
    size_t len = 0;
    OH_HiSysEvent_GetParamNames(&record, &params, &len);
    ASSERT_EQ(len, 0);
}

void RecordParamIntValueInvalidTest(const HiSysEventRecord& record, const std::string& name, int expRes)
{
    int64_t testValue = 0;
    int res = OH_HiSysEvent_GetParamInt64Value(&record, name.c_str(), &testValue);
    ASSERT_EQ(res, expRes);
}

void RecordParamUintValueInvalidTest(const HiSysEventRecord& record, const std::string& name, int expRes)
{
    uint64_t testValue = 0;
    int res = OH_HiSysEvent_GetParamUint64Value(&record, name.c_str(), &testValue);
    ASSERT_EQ(res, expRes);
}

void RecordParamDouValueInvalidTest(const HiSysEventRecord& record, const std::string& name, int expRes)
{
    double testValue = 0;
    int res = OH_HiSysEvent_GetParamDoubleValue(&record, name.c_str(), &testValue);
    ASSERT_EQ(res, expRes);
}

void RecordParamStrValueInvalidTest(const HiSysEventRecord& record, const std::string& name, int expRes)
{
    char* testValue = nullptr;
    int res = OH_HiSysEvent_GetParamStringValue(&record, name.c_str(), &testValue);
    ASSERT_EQ(res, expRes);
}

void RecordParamIntValuesInvalidTest(const HiSysEventRecord& record, const std::string& name, int expRes)
{
    int64_t* testValues = nullptr;
    size_t len = 0;
    int res = OH_HiSysEvent_GetParamInt64Values(&record, name.c_str(), &testValues, &len);
    ASSERT_EQ(res, expRes);
}

void RecordParamUintValuesInvalidTest(const HiSysEventRecord& record, const std::string& name, int expRes)
{
    uint64_t* testValues = nullptr;
    size_t len = 0;
    int res = OH_HiSysEvent_GetParamUint64Values(&record, name.c_str(), &testValues, &len);
    ASSERT_EQ(res, expRes);
}

void RecordParamDouValuesInvalidTest(const HiSysEventRecord& record, const std::string& name, int expRes)
{
    double* testValues = nullptr;
    size_t len = 0;
    int res = OH_HiSysEvent_GetParamDoubleValues(&record, name.c_str(), &testValues, &len);
    ASSERT_EQ(res, expRes);
}

void RecordParamStrValuesInvalidTest(const HiSysEventRecord& record, const std::string& name, int expRes)
{
    char** testValues = nullptr;
    size_t len = 0;
    int res = OH_HiSysEvent_GetParamStringValues(&record, name.c_str(), &testValues, &len);
    ASSERT_EQ(res, expRes);
}
}

class HiSysEventManagerCTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HiSysEventManagerCTest::SetUp()
{}

void HiSysEventManagerCTest::TearDown()
{}

/**
 * @tc.name: HiSysEventMgrCQueryTest001
 * @tc.desc: Testing to query events.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest001 start");
    HiSysEventQueryArg arg;
    InitQueryArg(arg);

    HiSysEventQueryRule rule;
    InitQueryRule(rule);
    HiSysEventQueryRule rules[] = { rule };

    HiSysEventQueryCallback callback;
    InitCallback(callback);

    auto res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, 0);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest001 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest002
 * @tc.desc: Testing to query events with condition.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest002 start");
    std::string cond = R"~({"version":"V1","condition":{"and":[{"param":"NAME","op":"=",
        "value":"SysEventStore"}]}})~";
    QueryTestWithCondition(cond);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest002 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest003
 * @tc.desc: Testing to query events with condition.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest003 start");
    std::string cond = R"~({"version":"V1","condition":{"and":[{"param":"uid_","op":"=","value":1201}]}})~";
    QueryTestWithCondition(cond);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest003 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest004
 * @tc.desc: Testing to query events with condition.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest004 start");
    std::string cond = R"~({"version":"V1","condition":{"and":[{"param":"pid_","op":">=","value":0}]}})~";
    QueryTestWithCondition(cond);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest004 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest005
 * @tc.desc: Testing to query events with condition.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest005, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest005 start");
    std::string cond = R"~({"version":"V1","condition":{"and":[{"param":"type_","op":"<=","value":4}]}})~";
    QueryTestWithCondition(cond);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest005 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest006
 * @tc.desc: Testing to query events with condition.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest006, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest006 start");
    std::string cond = R"~({"version":"V1","condition":{"and":[{"param":"pid_","op":">","value":0}]}})~";
    QueryTestWithCondition(cond);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest006 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest007
 * @tc.desc: Testing to query events with condition.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest007, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest007 start");
    std::string cond = R"~({"version":"V1","condition":{"and":[{"param":"pid_","op":"<","value":0}]}})~";
    QueryTestWithCondition(cond);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest007 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest008
 * @tc.desc: Testing to query events with many rules.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest008, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest008 start");
    sleep(QUERY_INTERVAL_TIME);
    HiSysEventQueryArg arg;
    InitQueryArg(arg);

    HiSysEventQueryRule rule1;
    (void)StringUtil::CopyCString(rule1.domain, TEST_DOMAIN, MAX_LEN_OF_DOMAIN);
    (void)StringUtil::CopyCString(rule1.eventList[0], TEST_NAME, MAX_LEN_OF_NAME);
    (void)StringUtil::CopyCString(rule1.eventList[1], "PLUGIN_UNLOAD", MAX_LEN_OF_NAME);
    rule1.eventListSize = 2;
    HiSysEventQueryRule rule2;
    (void)StringUtil::CopyCString(rule2.domain, TEST_DOMAIN, MAX_LEN_OF_DOMAIN);
    (void)StringUtil::CopyCString(rule2.eventList[0], "APP_USAGE", MAX_LEN_OF_NAME);
    (void)StringUtil::CopyCString(rule2.eventList[1], "SYS_USAGE", MAX_LEN_OF_NAME);
    rule2.eventListSize = 2;
    HiSysEventQueryRule rules[] = { rule1, rule2 };

    HiSysEventQueryCallback callback;
    InitCallback(callback);

    auto res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, 0);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest008 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest009
 * @tc.desc: Testing to query events with many conditions.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest009, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest009 start");
    std::string cond1 = R"~({"version":"V1","condition":{"and":[{"param":"NAME","op":"=",
        "value":"SysEventStore"},{"param":"uid_","op":"=","value":1201}]}})~";
    QueryTestWithCondition(cond1);

    std::string cond2 = R"~({"version":"V1","condition":{"and":[{"param":"type_","op":">","value":0},
        {"param":"uid_","op":"=","value":1201}]}})~";
    QueryTestWithCondition(cond2);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest009 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest010
 * @tc.desc: Testing to query events with many rules and condtions.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest010, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest010 start");
    sleep(QUERY_INTERVAL_TIME);
    HiSysEventQueryArg arg;
    InitQueryArg(arg);

    std::string cond = R"~({"version":"V1","condition":{"and":[{"param":"time_","op":">",
        "value":0},{"param":"type_","op":">","value":0}]}})~";
    HiSysEventQueryRule rule1;
    (void)StringUtil::CopyCString(rule1.domain, TEST_DOMAIN, MAX_LEN_OF_DOMAIN);
    (void)StringUtil::CopyCString(rule1.eventList[0], TEST_NAME, MAX_LEN_OF_NAME);
    (void)StringUtil::CopyCString(rule1.eventList[1], "PLUGIN_UNLOAD", MAX_LEN_OF_NAME);
    rule1.eventListSize = 2;
    (void)StringUtil::CreateCString(&rule1.condition, cond);
    HiSysEventQueryRule rule2;
    (void)StringUtil::CopyCString(rule2.domain, TEST_DOMAIN, MAX_LEN_OF_DOMAIN);
    (void)StringUtil::CopyCString(rule2.eventList[0], "APP_USAGE", MAX_LEN_OF_NAME);
    (void)StringUtil::CopyCString(rule2.eventList[1], "SYS_USAGE", MAX_LEN_OF_NAME);
    rule2.eventListSize = 2;
    (void)StringUtil::CreateCString(&rule2.condition, cond);
    HiSysEventQueryRule rules[] = { rule1, rule2 };

    HiSysEventQueryCallback callback;
    InitCallback(callback);

    auto res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, 0);
    StringUtil::DeletePointer<char>(&rule1.condition);
    StringUtil::DeletePointer<char>(&rule2.condition);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest010 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest011
 * @tc.desc: Testing to query events with invalid condition.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest011, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest011 start");

    std::string cond1 = R"~({"version":"xx","condition":{}})~";
    QueryTestWithCondition(cond1);

    std::string cond2 = "invalid condition";
    QueryTestWithCondition(cond2);

    std::string cond3 = R"~({"version":"V1","condition":{"invalid":[]}})~";
    QueryTestWithCondition(cond3);

    std::string cond4 = R"~({"version":"V1","condition":{"and":[{"invalid":"PLUGIN_NAME","op":"=",
        "value":"SysEventStore"}]}})~";
    QueryTestWithCondition(cond4);

    std::string cond5 = R"~({"version":"V1","condition":{"and":[{"param":"PLUGIN_NAME","invalid":"=",
        "value":"SysEventStore"}]}})~";
    QueryTestWithCondition(cond5);

    std::string cond6 = R"~({"version":"V1","condition":{"and":[{"param":"PLUGIN_NAME","op":"**",
        "value":"SysEventStore"}]}})~";
    QueryTestWithCondition(cond6);

    std::string cond7 = R"~({"version":"V1","condition":{"and":[{"param":"PLUGIN_NAME","op":"=",
        "invalid":"SysEventStore"}]}})~";
    QueryTestWithCondition(cond7);

    std::string cond8 = R"~({"version":"V1","condition":{"and":[{"param":"PLUGIN_NAME","op":"=",
        "value":[]}]}})~";
    QueryTestWithCondition(cond8);

    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest011 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest012
 * @tc.desc: Testing to query events only with domain.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest012, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest012 start");
    sleep(QUERY_INTERVAL_TIME);
    HiSysEventQueryArg arg;
    InitQueryArg(arg);
    HiSysEventQueryRule rule;
    (void)StringUtil::CopyCString(rule.domain, TEST_DOMAIN, MAX_LEN_OF_DOMAIN);
    rule.eventListSize = 0;
    HiSysEventQueryRule rules[] = { rule };
    HiSysEventQueryCallback callback;
    InitCallback(callback);
    auto res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, ERR_QUERY_RULE_INVALID);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest012 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest013
 * @tc.desc: Testing to query events only with name.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest013, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest013 start");
    sleep(QUERY_INTERVAL_TIME);
    HiSysEventQueryArg arg;
    InitQueryArg(arg);
    HiSysEventQueryRule rule;
    (void)StringUtil::CopyCString(rule.eventList[0], TEST_NAME, MAX_LEN_OF_NAME);
    rule.eventListSize = 1;
    HiSysEventQueryRule rules[] = { rule };
    HiSysEventQueryCallback callback;
    InitCallback(callback);
    auto res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, ERR_QUERY_RULE_INVALID);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest013 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest014
 * @tc.desc: Testing to query events only with domain and condition.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest014, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest014 start");
    sleep(QUERY_INTERVAL_TIME);
    HiSysEventQueryArg arg;
    InitQueryArg(arg);
    HiSysEventQueryRule rule;
    (void)StringUtil::CopyCString(rule.domain, TEST_DOMAIN, MAX_LEN_OF_DOMAIN);
    rule.eventListSize = 0;
    std::string cond = R"~({"version":"V1","condition":{"and":[{"param":"NAME","op":"=",
        "value":"SysEventStore"}]}})~";
    (void)StringUtil::CreateCString(&rule.condition, cond);
    HiSysEventQueryRule rules[] = { rule };
    HiSysEventQueryCallback callback;
    InitCallback(callback);
    auto res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, ERR_QUERY_RULE_INVALID);
    StringUtil::DeletePointer<char>(&rule.condition);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest014 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest015
 * @tc.desc: Testing to query events only with name and condition.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest015, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest015 start");
    sleep(QUERY_INTERVAL_TIME);
    HiSysEventQueryArg arg;
    InitQueryArg(arg);
    HiSysEventQueryRule rule;
    (void)StringUtil::CopyCString(rule.eventList[0], TEST_NAME, MAX_LEN_OF_NAME);
    rule.eventListSize = 1;
    std::string cond = R"~({"version":"V1","condition":{"and":[{"param":"NAME","op":"=",
        "value":"SysEventStore"}]}})~";
    (void)StringUtil::CreateCString(&rule.condition, cond);
    HiSysEventQueryRule rules[] = { rule };
    HiSysEventQueryCallback callback;
    InitCallback(callback);
    auto res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, ERR_QUERY_RULE_INVALID);
    StringUtil::DeletePointer<char>(&rule.condition);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest015 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest016
 * @tc.desc: Testing to query events only with condition.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest016, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest016 start");
    sleep(QUERY_INTERVAL_TIME);
    HiSysEventQueryArg arg;
    InitQueryArg(arg);
    HiSysEventQueryRule rule;
    rule.eventListSize = 0;
    std::string cond = R"~({"version":"V1","condition":{"and":[{"param":"NAME","op":"=",
        "value":"SysEventStore"}]}})~";
    (void)StringUtil::CreateCString(&rule.condition, cond);
    HiSysEventQueryRule rules[] = { rule };
    HiSysEventQueryCallback callback;
    InitCallback(callback);
    auto res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, ERR_QUERY_RULE_INVALID);
    StringUtil::DeletePointer<char>(&rule.condition);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest016 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest017
 * @tc.desc: Testing to query events are too frequent.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest017, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest017 start");
    sleep(QUERY_INTERVAL_TIME);
    HiSysEventQueryArg arg;
    InitQueryArg(arg);

    HiSysEventQueryRule rule;
    InitQueryRule(rule);
    HiSysEventQueryRule rules[] = { rule };

    HiSysEventQueryCallback callback;
    InitCallback(callback);

    const int threshhold = 50;
    const int delayDuration = 1; // 1 second
    for (int i = 0; i < 2; i++) { // 2 cycles
        sleep(delayDuration);
        for (int j = 0; j <= threshhold; j++) { // more than 50 queries in 1 second is never allowed
            auto ret = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
            ASSERT_TRUE((ret == ERR_QUERY_TOO_FREQUENTLY) || (ret == IPC_CALL_SUCCEED));
        }
    }

    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest017 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest018
 * @tc.desc: Testing to query events with too many rules.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest018, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest018 start");
    sleep(QUERY_INTERVAL_TIME);
    HiSysEventQueryArg arg;
    InitQueryArg(arg);

    HiSysEventQueryRule rule;
    InitQueryRule(rule);
    const int invalidRuleCnt = 101; // maximum count for query rule is 100.
    HiSysEventQueryRule rules[invalidRuleCnt];
    for (int i = 0; i < invalidRuleCnt; i++) {
        rules[i] = rule;
    }

    HiSysEventQueryCallback callback;
    InitCallback(callback);

    auto res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, ERR_TOO_MANY_QUERY_RULES);

    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest018 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest019
 * @tc.desc: Testing to query events with null param.
 * @tc.type: FUNC
 * @tc.require: issueI7O8IM
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest019, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest019 start");
    HiSysEventQueryRule rules[] = {};
    HiSysEventQueryCallback callback;
    InitCallback(callback);
    auto res = OH_HiSysEvent_Query(nullptr, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, ERR_QUERY_ARG_NULL);

    HiSysEventQueryArg arg;
    InitQueryArg(arg);
    res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), nullptr);
    ASSERT_EQ(res, ERR_QUERY_CALLBACK_NULL);

    callback.OnQuery = nullptr;
    res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, ERR_QUERY_CALLBACK_NULL);

    InitCallback(callback);
    callback.OnComplete = nullptr;
    res = OH_HiSysEvent_Query(&arg, rules, sizeof(rules) / sizeof(HiSysEventQueryRule), &callback);
    ASSERT_EQ(res, ERR_QUERY_CALLBACK_NULL);

    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest019 end");
}

/**
 * @tc.name: HiSysEventMgrCQueryTest020
 * @tc.desc: Testing to query events with invalid condition.
 * @tc.type: FUNC
 * @tc.require: issueIAXEER
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCQueryTest020, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create HiSysEventQueryArg.
     * @tc.steps: step2. create HiSysEventQueryRule.
     * @tc.steps: step3. create HiSysEventQueryCallback.
     * @tc.steps: step4. query event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest020 start");
    std::string rootIsArrayCond = R"~([])~";
    QueryTestWithCondition(rootIsArrayCond);
    std::string condIsArrayCond = R"~({"version":"V1","condition":[]})~";
    QueryTestWithCondition(condIsArrayCond);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCQueryTest020 end");
}

/**
 * @tc.name: HiSysEventMgrCRecordTest001
 * @tc.desc: Testing to get the record information.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCRecordTest001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. build record.
     * @tc.steps: step2. check the information from record.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCRecordTest001 start");
    const std::map<std::string, std::string> recordData = {
        {"\"domain_\"", "\"TEST_DOMAIN\""},
        {"\"name_\"", "\"TEST_NAME\""},
        {"\"type_\"", "4"},
        {"\"PARAM_INT\"", "-123"},
        {"\"PARAM_INTS\"", "[-1,-2,3]"},
        {"\"PARAM_UINT\"", "123"},
        {"\"PARAM_UINTS\"", "[1,2,3]"},
        {"\"PARAM_DOU\"", "123.456"},
        {"\"PARAM_DOUS\"", "[1.1,-2.2,3.3]"},
        {"\"PARAM_STR\"", "\"test\""},
        {"\"PARAM_STRS\"", "[\"test1\",\"test2\",\"test3\"]"}
    };
    HiSysEventRecord record;
    auto res = StringUtil::CreateCString(&record.jsonStr, BuildRecordString(recordData));
    if (res != 0) {
        HILOG_WARN(LOG_CORE, "failed to create record string");
        ASSERT_TRUE(false);
    }

    RecordParamNameTest(record, recordData);
    RecordParamIntValueTest(record, "PARAM_INT", -123);
    RecordParamUintValueTest(record, "PARAM_UINT", 123);
    RecordParamDouValueTest(record, "PARAM_DOU", 123.456);
    RecordParamStrValueTest(record, "PARAM_STR", "test");
    RecordParamIntValuesTest(record, "PARAM_INTS", {-1, -2, 3});
    RecordParamUintValuesTest(record, "PARAM_UINTS", {1, 2, 3});
    RecordParamDouValuesTest(record, "PARAM_DOUS", {1.1, -2.2, 3.3});
    RecordParamStrValuesTest(record, "PARAM_STRS", {"test1", "test2", "test3"});

    int expRes = -3;
    RecordParamIntValueInvalidTest(record, "PARAM_STR", expRes);
    RecordParamUintValueInvalidTest(record, "PARAM_STR", expRes);
    RecordParamDouValueInvalidTest(record, "PARAM_STR", expRes);
    RecordParamIntValuesInvalidTest(record, "PARAM_STRS", expRes);
    RecordParamUintValuesInvalidTest(record, "PARAM_STRS", expRes);
    RecordParamDouValuesInvalidTest(record, "PARAM_STRS", expRes);

    StringUtil::DeletePointer<char>(&record.jsonStr);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCRecordTest001 end");
}

/**
 * @tc.name: HiSysEventMgrCRecordTest002
 * @tc.desc: Testing to get the record information.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCRecordTest002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. build record.
     * @tc.steps: step2. check the information from record.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCRecordTest002 start");
    HiSysEventRecord record;
    auto res = StringUtil::CreateCString(&record.jsonStr, "invalid record");
    if (res != 0) {
        HILOG_WARN(LOG_CORE, "failed to create record string");
        ASSERT_TRUE(false);
    }

    int expRes = -1;
    RecordParamNameInvalidTest(record);
    RecordParamIntValueInvalidTest(record, "PARAM_INT", expRes);
    RecordParamUintValueInvalidTest(record, "PARAM_UINT", expRes);
    RecordParamDouValueInvalidTest(record, "PARAM_DOU", expRes);
    RecordParamStrValueInvalidTest(record, "PARAM_STR", expRes);
    RecordParamIntValuesInvalidTest(record, "PARAM_INTS", expRes);
    RecordParamUintValuesInvalidTest(record, "PARAM_UINTS", expRes);
    RecordParamDouValuesInvalidTest(record, "PARAM_DOUS", expRes);
    RecordParamStrValuesInvalidTest(record, "PARAM_STRS", expRes);

    StringUtil::DeletePointer<char>(&record.jsonStr);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCRecordTest002 end");
}

/**
 * @tc.name: HiSysEventMgrCRecordTest003
 * @tc.desc: Testing to get the record information.
 * @tc.type: FUNC
 * @tc.require: issueI5X08B
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCRecordTest003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. build record.
     * @tc.steps: step2. check the information from record.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCRecordTest003 start");
    HiSysEventRecord record;
    auto res = StringUtil::CreateCString(&record.jsonStr, R"~({})~");
    if (res != 0) {
        HILOG_WARN(LOG_CORE, "failed to create record string");
        ASSERT_TRUE(false);
    }

    int expRes = -2;
    RecordParamNameTest(record, {});
    RecordParamIntValueInvalidTest(record, "PARAM_INT", expRes);
    RecordParamUintValueInvalidTest(record, "PARAM_UINT", expRes);
    RecordParamDouValueInvalidTest(record, "PARAM_DOU", expRes);
    RecordParamStrValueInvalidTest(record, "PARAM_STR", expRes);
    RecordParamIntValuesInvalidTest(record, "PARAM_INTS", expRes);
    RecordParamUintValuesInvalidTest(record, "PARAM_UINTS", expRes);
    RecordParamDouValuesInvalidTest(record, "PARAM_DOUS", expRes);
    RecordParamStrValuesInvalidTest(record, "PARAM_STRS", expRes);

    StringUtil::DeletePointer<char>(&record.jsonStr);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCRecordTest003 end");
}

/**
 * @tc.name: HiSysEventMgrCRecordTest004
 * @tc.desc: Test apis of HisysventRecordC
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCRecordTest004, TestSize.Level3)
{
    struct HiSysEventRecord record;
    char*** testp = nullptr;
    size_t len = 0;
    OH_HiSysEvent_GetParamNames(&record, testp, &len);
    ASSERT_TRUE(true);
    int64_t value1;
    auto ret = OH_HiSysEvent_GetParamInt64Value(&record, "KEY", &value1);
    ASSERT_EQ(ret, ERR_NULL);
    ret = OH_HiSysEvent_GetParamInt64Value(&record, nullptr, &value1);
    ASSERT_EQ(ret, ERR_NULL);
    uint64_t value2;
    ret = OH_HiSysEvent_GetParamUint64Value(&record, "KEY", &value2);
    ASSERT_EQ(ret, ERR_NULL);
    ret = OH_HiSysEvent_GetParamUint64Value(&record, nullptr, &value2);
    ASSERT_EQ(ret, ERR_NULL);
    double value3;
    ret = OH_HiSysEvent_GetParamDoubleValue(&record, "KEY", &value3);
    ASSERT_EQ(ret, ERR_NULL);
    ret = OH_HiSysEvent_GetParamDoubleValue(&record, nullptr, &value3);
    ASSERT_EQ(ret, ERR_NULL);
    char value4[100];
    char* value4p = value4;
    char** value4pp = &value4p;
    ret = OH_HiSysEvent_GetParamStringValue(&record, "KEY", value4pp);
    ASSERT_EQ(ret, ERR_NULL);
    ret = OH_HiSysEvent_GetParamStringValue(&record, nullptr, value4pp);
    ASSERT_EQ(ret, ERR_NULL);
    size_t dataLen;
    int64_t value5[10];
    int64_t* value5p = value5;
    int64_t** value5pp = &value5p;
    ret = OH_HiSysEvent_GetParamInt64Values(&record, "KEY", value5pp, &dataLen);
    ASSERT_EQ(ret, ERR_NULL);
    ret = OH_HiSysEvent_GetParamInt64Values(&record, nullptr, value5pp, &dataLen);
    ASSERT_EQ(ret, ERR_NULL);
    uint64_t value6[10];
    uint64_t* value6p = value6;
    uint64_t** value6pp = &value6p;
    ret = OH_HiSysEvent_GetParamUint64Values(&record, "KEY", value6pp, &dataLen);
    ASSERT_EQ(ret, ERR_NULL);
    ret = OH_HiSysEvent_GetParamUint64Values(&record, nullptr, value6pp, &dataLen);
    ASSERT_EQ(ret, ERR_NULL);
    double value7[10];
    double* value7p = value7;
    double** value7pp = &value7p;
    ret = OH_HiSysEvent_GetParamDoubleValues(&record, "KEY", value7pp, &dataLen);
    ASSERT_EQ(ret, ERR_NULL);
    ret = OH_HiSysEvent_GetParamDoubleValues(&record, nullptr, value7pp, &dataLen);
    ASSERT_EQ(ret, ERR_NULL);
    char v3[10][100] {};
    char* dest3p = v3[0];
    char** dest3pp = &dest3p;
    char*** dest3ppp = &dest3pp;
    ret = OH_HiSysEvent_GetParamStringValues(&record, "KEY", dest3ppp, &dataLen);
    ASSERT_EQ(ret, ERR_NULL);
    ret = OH_HiSysEvent_GetParamStringValues(&record, nullptr, dest3ppp, &dataLen);
    ASSERT_EQ(ret, ERR_NULL);
}

/**
 * @tc.name: HiSysEventMgrCWatchTest001
 * @tc.desc: Testing to watch events with null param.
 * @tc.type: FUNC
 * @tc.require: issueI7O8IM
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCWatcherTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create HiSysEventWatcher object.
     * @tc.steps: step2. create HiSysEventWatchRule objects.
     * @tc.steps: step3. watch event.
     */
    // watcher is null
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCWatcherTest001 start");
    auto ret = OH_HiSysEvent_Add_Watcher(nullptr, nullptr, 0);
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);

    // watcher.OnEvent is null
    HiSysEventWatcher nullWatcher;
    InitWatcher(nullWatcher);
    nullWatcher.OnEvent = nullptr;
    HiSysEventWatchRule rule = {"HIVIEWDFX", "PLUGIN_LOAD", "", 1, 0};
    HiSysEventWatchRule rules[] = {rule};
    ret = OH_HiSysEvent_Add_Watcher(&nullWatcher, rules, sizeof(rules) / sizeof(HiSysEventWatchRule));
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);

    // watcher.OnServiceDied is null
    InitWatcher(nullWatcher);
    nullWatcher.OnServiceDied = nullptr;
    ret = OH_HiSysEvent_Add_Watcher(&nullWatcher, rules, sizeof(rules) / sizeof(HiSysEventWatchRule));
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);

    // watcher does not exist
    HiSysEventWatcher watcher;
    InitWatcher(watcher);
    ret = OH_HiSysEvent_Remove_Watcher(&watcher);
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);

    // normal function test
    ret = OH_HiSysEvent_Add_Watcher(&watcher, rules, sizeof(rules) / sizeof(HiSysEventWatchRule));
    ASSERT_EQ(ret, 0);
    ret = OH_HiSysEvent_Write("HIVIEWDFX", "PLUGIN_LOAD", HISYSEVENT_BEHAVIOR, nullptr, 0);
    ASSERT_EQ(ret, 0);
    sleep(3);
    ret = OH_HiSysEvent_Remove_Watcher(&watcher);
    ASSERT_EQ(ret, 0);

    HILOG_INFO(LOG_CORE, "HiSysEventMgrCWatcherTest001 end");
}

/**
 * @tc.name: HiSysEventMgrCWatchTest002
 * @tc.desc: Testing to watch events with too many rules.
 * @tc.type: FUNC
 * @tc.require: issueI7O8IM
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCWatchTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create HiSysEventWatcher object.
     * @tc.steps: step2. create HiSysEventWatchRule objects.
     * @tc.steps: step3. watch event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCWatchTest002 start");
    HiSysEventWatcher watcher;
    InitWatcher(watcher);
    HiSysEventWatchRule rule = {"HIVIEWDFX", "PLUGIN_LOAD", "", 1, 0};
    const size_t maxNum = 20;
    HiSysEventWatchRule rules[maxNum + 1];
    for (size_t i = 0; i <= maxNum; i++) {
        rules[i] = rule;
    }
    auto ret = OH_HiSysEvent_Add_Watcher(&watcher, rules, sizeof(rules) / sizeof(HiSysEventWatchRule));
    ASSERT_EQ(ret, ERR_TOO_MANY_WATCH_RULES);
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCWatchTest002 end");
}

/**
 * @tc.name: HiSysEventMgrCWatchTest003
 * @tc.desc: Testing to watch events with too many watchers.
 * @tc.type: FUNC
 * @tc.require: issueI7O8IM
 */
HWTEST_F(HiSysEventManagerCTest, HiSysEventMgrCWatchTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create HiSysEventWatcher object.
     * @tc.steps: step2. create HiSysEventWatchRule objects.
     * @tc.steps: step3. watch event.
     */
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCWatchTest003 start");
    const size_t maxNum = 30;
    HiSysEventWatcher watchers[maxNum + 1];
    for (size_t i = 0; i <= maxNum; i++) {
        HiSysEventWatcher watcher;
        InitWatcher(watcher);
        watchers[i] = watcher;
    }
    HiSysEventWatchRule rule = {"HIVIEWDFX", "PLUGIN_LOAD", "", 1, 0};
    HiSysEventWatchRule rules[] = {rule};
    for (size_t i = 0; i < maxNum; i++) {
        (void)OH_HiSysEvent_Add_Watcher(&watchers[i], rules, sizeof(rules) / sizeof(HiSysEventWatchRule));
    }
    auto ret = OH_HiSysEvent_Add_Watcher(&watchers[maxNum], rules, sizeof(rules) / sizeof(HiSysEventWatchRule));
    ASSERT_EQ(ret, ERR_TOO_MANY_WATCHERS);

    for (size_t i = 0; i <= maxNum; i++) {
        (void)OH_HiSysEvent_Remove_Watcher(&watchers[i]);
    }
    HILOG_INFO(LOG_CORE, "HiSysEventMgrCWatchTest003 end");
}
