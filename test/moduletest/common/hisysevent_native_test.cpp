/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "hisysevent_native_test.h"

#include <functional>
#include <iosfwd>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include "hilog/log.h"

#include "def.h"
#include "event_socket_factory.h"
#include "hisysevent.h"
#include "hisysevent_base_manager.h"
#include "hisysevent_manager.h"
#include "hisysevent_record.h"
#include "hisysevent_query_callback.h"
#include "hisysevent_listener.h"
#include "ret_code.h"
#include "rule_type.h"
#include "securec.h"

#ifndef SYS_EVENT_PARAMS
#define SYS_EVENT_PARAMS(A) "key"#A, 0 + (A), "keyA"#A, 1 + (A), "keyB"#A, 2 + (A), "keyC"#A, 3 + (A), \
    "keyD"#A, 4 + (A), "keyE"#A, 5 + (A), "keyF"#A, 6 + (A), "keyG"#A, 7 + (A), "keyH"#A, 8 + (A), \
    "keyI"#A, 9 + (A)
#endif
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_NATIVE_TEST"

namespace {
constexpr char TEST_DOMAIN[] = "DEMO";
constexpr char TEST_DOMAIN2[] = "KERNEL_VENDOR";
int32_t WriteSysEventByMarcoInterface()
{
    return HiSysEventWrite(TEST_DOMAIN, "DEMO_EVENTNAME", HiSysEvent::EventType::FAULT,
        "PARAM_KEY", "PARAM_VAL");
}

class Watcher : public HiSysEventListener {
public:
    Watcher() {}
    virtual ~Watcher() {}

    void OnEvent(std::shared_ptr<HiSysEventRecord> sysEvent) final
    {
        if (sysEvent == nullptr) {
            return;
        }
        HILOG_DEBUG(LOG_CORE, "domain: %{public}s, eventName: %{public}s, eventType: %{public}d, extra: %{public}s.",
            sysEvent->GetDomain().c_str(), sysEvent->GetEventName().c_str(), sysEvent->GetEventType(),
            sysEvent->AsJson().c_str());
    }

    void OnServiceDied() final
    {
        HILOG_DEBUG(LOG_CORE, "OnServiceDied");
    }
};

using OnQueryCallback = std::function<bool(std::shared_ptr<std::vector<HiSysEventRecord>>)>;
using OnCompleteCallback = std::function<bool(int32_t, int32_t)>;

class Querier : public HiSysEventQueryCallback {
public:
    explicit Querier(OnQueryCallback onQueryCallback = nullptr, OnCompleteCallback onCompleteCallback = nullptr)
        : onQueryCallback(onQueryCallback), onCompleteCallback(onCompleteCallback) {}
    virtual ~Querier() {}

    void OnQuery(std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) final
    {
        if (onQueryCallback != nullptr) {
            ASSERT_TRUE(onQueryCallback(sysEvents));
        }
    }

    void OnComplete(int32_t reason, int32_t total) final
    {
        if (onCompleteCallback != nullptr) {
            ASSERT_TRUE(onCompleteCallback(reason, total));
        }
    }

private:
    OnQueryCallback onQueryCallback;
    OnCompleteCallback onCompleteCallback;
};

void BuildRawData(RawData& data, const std::string& domain, const std::string& name, HiSysEvent::EventType type)
{
    struct Encoded::HiSysEventHeader header = {
        {0}, {0}, 0, 0, 0, 0, 0, 0, 0, 0
    };
    auto ret = memcpy_s(header.domain, MAX_DOMAIN_LENGTH, domain.c_str(), domain.length());
    ASSERT_EQ(ret, EOK);
    header.domain[domain.length()] = '\0';
    ret = memcpy_s(header.name, MAX_EVENT_NAME_LENGTH, name.c_str(), name.length());
    ASSERT_EQ(ret, EOK);
    header.name[name.length()] = '\0';
    header.type = type - 1; // 1 is offset
    int32_t len = sizeof(struct Encoded::HiSysEventHeader) + sizeof(int32_t);
    (void)data.Update(reinterpret_cast<uint8_t*>(&len), sizeof(int32_t), 0);
    (void)data.Update(reinterpret_cast<uint8_t*>(&header), sizeof(struct Encoded::HiSysEventHeader),
        sizeof(int32_t));
}
}

static bool WrapSysEventWriteAssertion(int32_t ret, bool cond)
{
    return cond || ret == OHOS::HiviewDFX::ERR_SEND_FAIL ||
        ret == OHOS::HiviewDFX::ERR_WRITE_IN_HIGH_FREQ ||
        ret == OHOS::HiviewDFX::ERR_DOMAIN_MASKED ||
        ret == OHOS::HiviewDFX::ERR_TOO_MANY_CONCURRENT_QUERIES ||
        ret == OHOS::HiviewDFX::ERR_QUERY_TOO_FREQUENTLY;
}

void HiSysEventNativeTest::SetUpTestCase(void)
{
}

void HiSysEventNativeTest::TearDownTestCase(void)
{
}

void HiSysEventNativeTest::SetUp(void)
{
}

void HiSysEventNativeTest::TearDown(void)
{
}

/**
 * @tc.name: TestHiSysEventManagerQueryWithInvalidQueryRules001
 * @tc.desc: Query with query rules which contains empty domain
 * @tc.type: FUNC
 * @tc.require: issueI62B10
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventManagerQueryWithInvalidQueryRules001, TestSize.Level1)
{
    sleep(1);
    auto querier = std::make_shared<Querier>();
    long long defaultTimeStap = -1;
    int queryCount = 10;
    struct OHOS::HiviewDFX::QueryArg args(defaultTimeStap, defaultTimeStap, queryCount);
    std::vector<OHOS::HiviewDFX::QueryRule> queryRules;
    std::vector<std::string> eventNames {"EVENT_NAME"};
    OHOS::HiviewDFX::QueryRule rule("", eventNames); // empty domain
    queryRules.emplace_back(rule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::Query(args, queryRules, querier);
    // only process with root or shell uid return OHOS::HiviewDFX::IPC_CALL_SUCCEED
    ASSERT_TRUE(ret == OHOS::HiviewDFX::ERR_QUERY_RULE_INVALID || ret == OHOS::HiviewDFX::IPC_CALL_SUCCEED);
}

/**
 * @tc.name: TestHiSysEventManagerQueryWithInvalidQueryRules002
 * @tc.desc: Query with query rules which contains empty event names
 * @tc.type: FUNC
 * @tc.require: issueI62B10
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventManagerQueryWithInvalidQueryRules002, TestSize.Level1)
{
    sleep(1);
    auto querier = std::make_shared<Querier>();
    long long defaultTimeStap = -1;
    int queryCount = 10;
    struct OHOS::HiviewDFX::QueryArg args(defaultTimeStap, defaultTimeStap, queryCount);
    std::vector<OHOS::HiviewDFX::QueryRule> queryRules;
    std::vector<std::string> eventNames; // empty event name
    OHOS::HiviewDFX::QueryRule rule("DOMAIN", eventNames);
    queryRules.emplace_back(rule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::Query(args, queryRules, querier);
    // only process with root or shell uid return OHOS::HiviewDFX::IPC_CALL_SUCCEED
    ASSERT_TRUE(ret == OHOS::HiviewDFX::ERR_QUERY_RULE_INVALID || ret == OHOS::HiviewDFX::IPC_CALL_SUCCEED);
}

/**
 * @tc.name: TestSubscribeSysEventByTag
 * @tc.desc: Subscribe sysevent by event tag
 * @tc.type: FUNC
 * @tc.require: issueI62B10
 */
HWTEST_F(HiSysEventNativeTest, TestSubscribeSysEventByTag, TestSize.Level1)
{
    auto watcher = std::make_shared<Watcher>();
    OHOS::HiviewDFX::ListenerRule listenerRule("DOMAIN", "EVENT_NAME", "TAG", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<OHOS::HiviewDFX::ListenerRule> sysRules;
    sysRules.emplace_back(listenerRule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(watcher, sysRules);
    ASSERT_EQ(ret, IPC_CALL_SUCCEED);
    ret = OHOS::HiviewDFX::HiSysEventManager::RemoveListener(watcher);
    ASSERT_EQ(ret, IPC_CALL_SUCCEED);
}

/**
 * @tc.name: TestHiSysEventNormal001
 * @tc.desc: Test normal write.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventNormal001, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure SystemAbilityManager is started.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "NORMAL001";

    bool testBoolValue = true;
    char testCharValue = 'a';
    short testShortValue = -100;
    int testIntValue = -200;
    long testLongValue = -300;
    long long testLongLongValue = -400;

    unsigned char testUnsignedCharValue = 'a';
    unsigned short testUnsignedShortValue = 100;
    unsigned int testUnsignedIntValue = 200;
    unsigned long testUnsignedLongValue = 300;
    unsigned long long testUnsignedLongLongValue = 400;

    float testFloatValue = 1.1;
    double testDoubleValue = 2.2;
    std::string testStringValue = "abc";

    std::vector<bool> testBoolValues;
    testBoolValues.push_back(true);
    testBoolValues.push_back(true);
    testBoolValues.push_back(false);

    std::vector<char> testCharValues;
    testCharValues.push_back('a');
    testCharValues.push_back('b');
    testCharValues.push_back('c');

    std::vector<unsigned char> testUnsignedCharValues;
    testUnsignedCharValues.push_back('a');
    testUnsignedCharValues.push_back('b');
    testUnsignedCharValues.push_back('c');

    std::vector<short> testShortValues;
    testShortValues.push_back(-100);
    testShortValues.push_back(-200);
    testShortValues.push_back(-300);

    std::vector<unsigned short> testUnsignedShortValues;
    testUnsignedShortValues.push_back(100);
    testUnsignedShortValues.push_back(200);
    testUnsignedShortValues.push_back(300);

    std::vector<int> testIntValues;
    testIntValues.push_back(-1000);
    testIntValues.push_back(-2000);
    testIntValues.push_back(-3000);

    std::vector<unsigned int> testUnsignedIntValues;
    testUnsignedIntValues.push_back(1000);
    testUnsignedIntValues.push_back(2000);
    testUnsignedIntValues.push_back(3000);

    std::vector<long> testLongValues;
    testLongValues.push_back(-10000);
    testLongValues.push_back(-20000);
    testLongValues.push_back(-30000);

    std::vector<unsigned long> testUnsignedLongValues;
    testUnsignedLongValues.push_back(10000);
    testUnsignedLongValues.push_back(20000);
    testUnsignedLongValues.push_back(30000);

    std::vector<long long> testLongLongValues;
    testLongLongValues.push_back(-100000);
    testLongLongValues.push_back(-200000);
    testLongLongValues.push_back(-300000);

    std::vector<unsigned long long> testUnsignedLongLongValues;
    testUnsignedLongLongValues.push_back(100000);
    testUnsignedLongLongValues.push_back(200000);
    testUnsignedLongLongValues.push_back(300000);

    std::vector<float> testFloatValues;
    testFloatValues.push_back(1.1);
    testFloatValues.push_back(2.2);
    testFloatValues.push_back(3.3);

    std::vector<double> testDoubleValues;
    testDoubleValues.push_back(10.1);
    testDoubleValues.push_back(20.2);
    testDoubleValues.push_back(30.3);

    std::vector<std::string> testStringValues;
    testStringValues.push_back(std::string("a"));
    testStringValues.push_back(std::string("b"));
    testStringValues.push_back(std::string("c"));

    HILOG_INFO(LOG_CORE, "test hisysevent normal write");
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT,
        "keyBool", testBoolValue, "keyChar", testCharValue, "keyShort", testShortValue,
        "keyInt", testIntValue, "KeyLong", testLongValue, "KeyLongLong", testLongLongValue,
        "keyUnsignedChar", testUnsignedCharValue, "keyUnsignedShort", testUnsignedShortValue,
        "keyUnsignedInt", testUnsignedIntValue, "keyUnsignedLong", testUnsignedLongValue,
        "keyUnsignedLongLong", testUnsignedLongLongValue, "keyFloat", testFloatValue,
        "keyDouble", testDoubleValue, "keyString1", testStringValue, "keyString2", "efg",
        "keyBools", testBoolValues, "keyChars", testCharValues, "keyUnsignedChars", testUnsignedCharValues,
        "keyShorts", testShortValues, "keyUnsignedShorts", testUnsignedShortValues,
        "keyInts", testIntValues, "keyUnsignedInts", testUnsignedIntValues, "keyLongs", testLongValues,
        "keyUnsignedLongs", testUnsignedLongValues, "keyLongLongs", testLongLongValues,
        "keyUnsignedLongLongs", testUnsignedLongLongValues, "keyFloats", testFloatValues,
        "keyDoubles", testDoubleValues, "keyStrings", testStringValues);
    HILOG_INFO(LOG_CORE, "normal write, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventDomainSpecialChar002
 * @tc.desc: Test domain has special char.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventDomainSpecialChar002, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "_demo";
    std::string eventName = "DOMAIN_SPECIAL_CHAR";
    HILOG_INFO(LOG_CORE, "test hisysevent domain has special char");
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT);
    HILOG_INFO(LOG_CORE, "domain has special char, retCode=%{public}d", result);
    ASSERT_LT(result, 0);
}

/**
 * @tc.name: TestHiSysEventDomainEmpty003
 * @tc.desc: Test domain is empty.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventDomainEmpty003, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "";
    std::string eventName = "DOMAIN_EMPTY";
    HILOG_INFO(LOG_CORE, "test hisysevent domain is empty");
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT);
    HILOG_INFO(LOG_CORE, "domain is empty, retCode=%{public}d", result);
    ASSERT_LT(result, 0);
}

/**
 * @tc.name: TestHiSysEventDomainTooLong004
 * @tc.desc: Test domain is too long.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventDomainTooLong004, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain16[] = "AAAAAAAAAAAAAAAA";
    std::string eventName = "DOMAIN_TOO_LONG_16";
    HILOG_INFO(LOG_CORE, "test hisysevent domain is too long, normal length");

    int result = 0;
    result = HiSysEventWrite(domain16, eventName, HiSysEvent::EventType::FAULT);
    HILOG_INFO(LOG_CORE, "domain too long, equal 16 retCode=%{public}d", result);

    HILOG_INFO(LOG_CORE, "test hisysevent domain is too long");
    static constexpr char domain17[] = "AAAAAAAAAAAAAAAAL";
    eventName = "DOMAIN_TOO_LONG_17";
    result = HiSysEventWrite(domain17, eventName, HiSysEvent::EventType::FAULT);
    HILOG_INFO(LOG_CORE, "domain is too long, more than 16 retCode=%{public}d", result);
    ASSERT_LT(result, 0);
}

/**
 * @tc.name: TesetHiSysEventSpecailEventName005
 * @tc.desc: Test event name has special char.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TesetHiSysEventSpecailEventName005, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "SPEC_EVT_NAME";
    std::string eventName = "_SPECIAL_CHAR";
    HILOG_INFO(LOG_CORE, "test hisysevent event name has special char");
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT);
    HILOG_INFO(LOG_CORE, "event name has special char, retCode=%{public}d", result);
    ASSERT_LT(result, 0);
}

/**
 * @tc.name: TestHiSysEventNameEmpty006
 * @tc.desc: Test event name is empty.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventNameEmpty006, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "EMPTY";
    std::string eventName = "";
    HILOG_INFO(LOG_CORE, "test hisysevent event name is empty");
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT);
    HILOG_INFO(LOG_CORE, "event name is empty, retCode=%{public}d", result);
    ASSERT_LT(result, 0);
}

/**
 * @tc.name: TesetHiSysEventNameTooLong007
 * @tc.desc: Test event name too long.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TesetHiSysEventNameTooLong007, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain32[] = "NAME_32";
    std::string eventName = "";
    HILOG_INFO(LOG_CORE, "test hisysevent event name is too long, normal length");
    int normal = 32;
    for (int index = 0; index < normal; index++) {
        eventName.append("N");
    }
    int result = 0;
    result = HiSysEventWrite(domain32, eventName, HiSysEvent::EventType::FAULT);
    HILOG_INFO(LOG_CORE, "event name is too long, equal 32, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::SUCCESS));

    HILOG_INFO(LOG_CORE, "test hisysevent event name is too long");
    static constexpr char domain33[] = "NAME_33";
    eventName.append("L");
    result = HiSysEventWrite(domain33, eventName, HiSysEvent::EventType::FAULT);
    HILOG_INFO(LOG_CORE, "event name is too long, more than 32, retCode=%{public}d", result);
    ASSERT_LT(result, 0);
}

/**
 * @tc.name: TestHiSysEventKeySpecialChar008
 * @tc.desc: Test key has specail char.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventKeySpecialChar008, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "HiSysEvent006";
    std::string key1 = "_key1";
    std::string key2 = "key2";
    int result = 0;
    HILOG_INFO(LOG_CORE, "test hisysevent key has special char");
    bool value1 = true;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value1, key2, value1);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    short value2 = 2;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value2, key2, value2);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    unsigned short value3 = 3;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value3, key2, value3);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    int value4 = 4;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value4, key2, value4);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    unsigned int value5 = 5;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value5, key2, value5);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    long value6 = 6;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value6, key2, value6);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    unsigned long value7 = 7;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value7, key2, value7);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    long long value8 = 8;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value8, key2, value8);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    unsigned long long value9 = 9;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value9, key2, value9);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    char value10 = 'a';
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value10, key2, value10);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    unsigned char value11 = 'b';
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value11, key2, value11);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    float value12 = 12.12;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value12, key2, value12);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));

    double value13 = 13.13;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key1, value13, key2, value13);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));
}


/**
 * @tc.name: TestHiSysEventEscape009
 * @tc.desc: Test key's value need escape.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventEscape009, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "ESCAPE";
    HILOG_INFO(LOG_CORE, "test hisysevent escape char");
    std::string value = "\"escapeByCpp\"";
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", value);
    HILOG_INFO(LOG_CORE, "key's value has espcae char, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventKeyEmpty010
 * @tc.desc: Test key is empty.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventKeyEmpty010, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "KEY_EMPTY";
    HILOG_INFO(LOG_CORE, "test hisysevent key is empty");
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT,
        "", "valueIsEmpty", "key2", "notEmpty");
    HILOG_INFO(LOG_CORE, "key is empty, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventKeySpecialChar011
 * @tc.desc: Test key has special char.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventKeySpecialChar011, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "KEY_SPECIAL_CHAR";
    HILOG_INFO(LOG_CORE, "test hisysevent key is special");
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT,
        "_key1", "special", "key2", "normal");
    HILOG_INFO(LOG_CORE, "key has special char, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventKeyTooLong012
 * @tc.desc: Test key is too long.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventKeyTooLong012, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "KEY_48";
    HILOG_INFO(LOG_CORE, "test hisysevent key 48 char");
    std::string key = "";
    int normal = 48;
    for (int index = 0; index < normal; index++) {
        key.append("V");
    }
    int result = 0;
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key, "48length", "key2", "normal");
    HILOG_INFO(LOG_CORE, "key equal 48 char, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::SUCCESS));

    HILOG_INFO(LOG_CORE, "test hisysevent key 49 char");
    eventName = "KEY_49";
    key.append("V");
    result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, key, "49length", "key2", "normal");
    HILOG_INFO(LOG_CORE, "key more than 48 char, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEvent128Keys013
 * @tc.desc: Test 128 key.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEvent128Keys013, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "TEST";
    std::string eventName = "KEY_EQUAL_128";
    HILOG_INFO(LOG_CORE, "test hisysevent 128 keys");
    std::string k = "k";
    bool v = true;
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT,
        SYS_EVENT_PARAMS(10), SYS_EVENT_PARAMS(20), SYS_EVENT_PARAMS(30), SYS_EVENT_PARAMS(40), SYS_EVENT_PARAMS(50),
        SYS_EVENT_PARAMS(60), SYS_EVENT_PARAMS(70), SYS_EVENT_PARAMS(80), SYS_EVENT_PARAMS(90), SYS_EVENT_PARAMS(100),
        SYS_EVENT_PARAMS(110), SYS_EVENT_PARAMS(120),
        k, v, k, v, k, v, k, v, k, v, k, v, k, v, k, v);
    HILOG_INFO(LOG_CORE, "has 128 key, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEvent129Keys014
 * @tc.desc: Test 129 key.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEvent129Keys014, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "TEST";
    std::string eventName = "KEY_EQUAL_129";
    HILOG_INFO(LOG_CORE, "test hisysevent 129 key");
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT,
        SYS_EVENT_PARAMS(10), SYS_EVENT_PARAMS(20), SYS_EVENT_PARAMS(30), SYS_EVENT_PARAMS(40), SYS_EVENT_PARAMS(50),
        SYS_EVENT_PARAMS(60), SYS_EVENT_PARAMS(70), SYS_EVENT_PARAMS(80), SYS_EVENT_PARAMS(90), SYS_EVENT_PARAMS(100),
        SYS_EVENT_PARAMS(110), SYS_EVENT_PARAMS(120),
        "key1", true, "key2", true, "key3", true, "key4", true, "key5", true,
        "key6", true, "key7", true, "key8", true, "key9", true);
    HILOG_INFO(LOG_CORE, "has 129 key, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventStringValueEqual256K015
 * @tc.desc: Test 256K string.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventStringValueEqual256K015, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "TEST";
    std::string eventName = "EQUAL_256K";
    HILOG_INFO(LOG_CORE, "test key's value 256K string");
    std::string value;
    int length = 256 * 1024;
    for (int index = 0; index < length; index++) {
        value.push_back('1' + index % 10);
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", value);
    HILOG_INFO(LOG_CORE, "string length is 256K, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventStringValueMoreThan256K016
 * @tc.desc: Test 256K + 1 string.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventStringValueMoreThan256K016, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "MORETHAN256K";
    HILOG_INFO(LOG_CORE, "test more than 256K string");
    std::string value;
    int length = 256 * 1024 + 1;
    for (int index = 0; index < length; index++) {
        value.push_back('1' + index % 10);
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", value);
    HILOG_INFO(LOG_CORE, "string length is more than 256K, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventArray100Item017
 * @tc.desc: Test bool array item 100.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray100Item017, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "BOOL_ARRAY_100";
    HILOG_INFO(LOG_CORE, "test bool array 100 item");
    std::vector<bool> values;
    int maxItem = 100;
    for (int index = 0; index < maxItem; index++) {
        values.push_back(true);
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HILOG_INFO(LOG_CORE, "array bool list 100, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventArray101Item018
 * @tc.desc: Test bool array item 101.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray101Item018, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "BOOL_ARRAY_101";
    HILOG_INFO(LOG_CORE, "test bool array 101 item");
    std::vector<bool> values;
    int maxItem = 101;
    for (int index = 0; index < maxItem; index++) {
        values.push_back(true);
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HILOG_INFO(LOG_CORE, "array bool list 101, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventArray100CharItem019
 * @tc.desc: Test char array item 100.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray100CharItem019, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "CHAR_ARRAY_100";
    HILOG_INFO(LOG_CORE, "test char array 100 item");
    std::vector<char> values;
    int maxItem = 100;
    for (int index = 0; index < maxItem; index++) {
        values.push_back('a');
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HILOG_INFO(LOG_CORE, "array char list 100, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventArray101CharItem020
 * @tc.desc: Test char array item 101.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray101CharItem020, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "CHAR_ARRAY_101";
    HILOG_INFO(LOG_CORE, "test char array 101 item");
    std::vector<char> values;
    int maxItem = 101;
    for (int index = 0; index < maxItem; index++) {
        values.push_back('z');
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HILOG_INFO(LOG_CORE, "array char list 101, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventArray100UnsignedCharItem021
 * @tc.desc: Test unsigned char array item 100.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray100UnsignedCharItem021, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "UCHAR_ARRAY_100";
    HILOG_INFO(LOG_CORE, "test unsigned char array 100 item");
    std::vector<unsigned char> values;
    int maxItem = 100;
    for (int index = 0; index < maxItem; index++) {
        values.push_back('a');
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HILOG_INFO(LOG_CORE, "array unsigned char list 100, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventArray101UnsignedCharItem022
 * @tc.desc: Test unsigned char array item 101.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray101UnsignedCharItem022, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "UCHAR_ARRAY_101";
    HILOG_INFO(LOG_CORE, "test unsigned char array 101 item");
    std::vector<unsigned char> values;
    int maxItem = 101;
    for (int index = 0; index < maxItem; index++) {
        values.push_back('z');
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HILOG_INFO(LOG_CORE, "array unsigned char list 101, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > OHOS::HiviewDFX::SUCCESS));
}


/**
 * @tc.name: TestHiSysEventArray100StringItem023
 * @tc.desc: Test string array item 100.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray100StringItem023, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "STR_ARRAY_100";
    HILOG_INFO(LOG_CORE, "test string array 100 item");
    std::vector<std::string> values;
    int maxItem = 100;
    for (int index = 0; index < maxItem; index++) {
        values.push_back("a");
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HILOG_INFO(LOG_CORE, "array string list 100, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::SUCCESS));
}

/**
 * @tc.name: TestHiSysEventArray101StringItem024
 * @tc.desc: Test string array item 101.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray101StringItem024, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "STR_ARRAY_101";
    HILOG_INFO(LOG_CORE, "test string array 101 item");
    std::vector<std::string> values;
    int maxItem = 101;
    for (int index = 0; index < maxItem; index++) {
        values.push_back("z");
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HILOG_INFO(LOG_CORE, "array string list 101, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > 0));
}

/**
 * @tc.name: TestHiSysEventArrayStringValueEqual256K025
 * @tc.desc: Test array item 256K string.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArrayStringValueEqual256K025, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "TEST";
    std::string eventName = "EQUAL_256K";
    HILOG_INFO(LOG_CORE, "test array item value 256K string");
    std::string value;
    int length = 256 * 1024;
    for (int index = 0; index < length; index++) {
        value.push_back('1' + index % 10);
    }
    sleep(1); // make sure hiview read all data before send large data
    std::vector<std::string> values;
    values.push_back("c");
    values.push_back(value);
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HILOG_INFO(LOG_CORE, "array item value length is 256K, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == 0));
}

/**
 * @tc.name: TestHiSysEventArrayStringValueMoreThan256K016
 * @tc.desc: Test 256K + 1 string.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArrayStringValueMoreThan256K026, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    static constexpr char domain[] = "DEMO";
    std::string eventName = "MORETHAN256K";
    HILOG_INFO(LOG_CORE, "test array item value more than 256K string");
    std::string value;
    int length = 256 * 1024 + 1;
    for (int index = 0; index < length; index++) {
        value.push_back('1' + index % 10);
    }
    sleep(1); // make sure hiview read all data before send large data
    std::vector<std::string> values;
    values.push_back("c");
    values.push_back(value);
    int result = HiSysEventWrite(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HILOG_INFO(LOG_CORE, "array item value length is more than 256K, retCode=%{public}d", result);
    ASSERT_TRUE(WrapSysEventWriteAssertion(result, result > 0));
}

/**
 * @tc.name: TestDefensingHiSysEventStorm
 * @tc.desc: Write event more than 100 times in 5 seconds
 * @tc.type: FUNC
 * @tc.require: issueI5FNPQ
 */
HWTEST_F(HiSysEventNativeTest, TestDefensingHiSysEventStorm, TestSize.Level1)
{
    int writeCount = 102;
    for (int i = 0; i < writeCount; i++) {
        auto result = WriteSysEventByMarcoInterface();
        if (i < HISYSEVENT_THRESHOLD) {
            ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == 0));
        } else {
            ASSERT_TRUE(WrapSysEventWriteAssertion(result, result == OHOS::HiviewDFX::ERR_WRITE_IN_HIGH_FREQ));
        }
    }
}

/**
 * @tc.name: TestAddAndRemoveListener
 * @tc.desc: Add listener and then remove it
 * @tc.type: FUNC
 * @tc.require: issueI5KDIG
 */
HWTEST_F(HiSysEventNativeTest, TestAddAndRemoveListener, TestSize.Level1)
{
    auto watcher = std::make_shared<Watcher>();
    OHOS::HiviewDFX::ListenerRule listenerRule("DOMAIN", "EVENT_NAME", "", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<OHOS::HiviewDFX::ListenerRule> sysRules;
    sysRules.emplace_back(listenerRule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(nullptr, sysRules);
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);
    ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(watcher, sysRules);
    ASSERT_EQ(ret, 0);
    ret = OHOS::HiviewDFX::HiSysEventManager::RemoveListener(nullptr);
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);
    auto newWatcher = std::make_shared<Watcher>();
    ret = OHOS::HiviewDFX::HiSysEventManager::RemoveListener(newWatcher);
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);
    ret = OHOS::HiviewDFX::HiSysEventManager::RemoveListener(watcher);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: TestEnableAndDisableDebugMode
 * @tc.desc: Enable debug mode and then disable it on listener
 * @tc.type: FUNC
 * @tc.require: issueI5KDIG
 */
HWTEST_F(HiSysEventNativeTest, TestEnableAndDisableDebugMode, TestSize.Level1)
{
    auto watcher = std::make_shared<Watcher>();
    OHOS::HiviewDFX::ListenerRule listenerRule("DOMAIN", "EVENT_NAME", "", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<OHOS::HiviewDFX::ListenerRule> sysRules;
    sysRules.emplace_back(listenerRule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(watcher, sysRules);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: TestHiSysEventBaseManagerAddAndRemoveListener
 * @tc.desc: Add a base listener and then remove it
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventBaseManagerAddAndRemoveListener, TestSize.Level1)
{
    auto watcher = std::make_shared<Watcher>();
    auto baseWatcher = std::make_shared<HiSysEventBaseListener>(watcher);
    OHOS::HiviewDFX::ListenerRule listenerRule("DOMAIN", "EVENT_NAME", "", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<OHOS::HiviewDFX::ListenerRule> sysRules;
    sysRules.emplace_back(listenerRule);
    auto ret = OHOS::HiviewDFX::HiSysEventBaseManager::AddListener(nullptr, sysRules);
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);
    ret = OHOS::HiviewDFX::HiSysEventBaseManager::AddListener(baseWatcher, sysRules);
    ASSERT_EQ(ret, 0);
    ret = OHOS::HiviewDFX::HiSysEventBaseManager::RemoveListener(nullptr);
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);
    auto newBaseWatcher = std::make_shared<HiSysEventBaseListener>(watcher);
    ret = OHOS::HiviewDFX::HiSysEventBaseManager::RemoveListener(newBaseWatcher);
    ASSERT_EQ(ret, ERR_LISTENER_NOT_EXIST);
    ret = OHOS::HiviewDFX::HiSysEventBaseManager::RemoveListener(baseWatcher);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: TestHiSysEventBaseManagerQueryEvent
 * @tc.desc: Query sys events by base manager
 * @tc.type: FUNC
 * @tc.require: issueI5KDIG
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventBaseManagerQueryEvent, TestSize.Level1)
{
    auto querier = std::make_shared<Querier>();
    long long defaultTimeStap = -1;
    int queryCount = 10;
    struct OHOS::HiviewDFX::QueryArg args(defaultTimeStap, defaultTimeStap, queryCount);
    std::vector<OHOS::HiviewDFX::QueryRule> queryRules;
    auto baseQuerier = std::make_shared<HiSysEventBaseQueryCallback>(querier);
    auto ret = OHOS::HiviewDFX::HiSysEventBaseManager::Query(args, queryRules, baseQuerier);
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: TestHiSysEventManagerAddListenerWithTooManyRules
 * @tc.desc: Add listener with more than 20 rules
 * @tc.type: FUNC
 * @tc.require: issueI5KDIG
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventManagerAddListenerWithTooManyRules, TestSize.Level1)
{
    auto watcher = std::make_shared<Watcher>();
    OHOS::HiviewDFX::ListenerRule listenerRule("DOMAIN", "EVENT_NAME", "", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    int ruleCount = 20;
    std::vector<OHOS::HiviewDFX::ListenerRule> sysRules;
    while (ruleCount-- > 0) {
        sysRules.emplace_back(listenerRule);
    }
    auto ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(watcher, sysRules);
    ASSERT_EQ(ret, 0);
    sysRules.emplace_back(listenerRule);
    ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(watcher, sysRules);
    ASSERT_EQ(ret, OHOS::HiviewDFX::ERR_TOO_MANY_WATCH_RULES);
}

/**
 * @tc.name: TestHiSysEventManagerAddTooManyEventListener
 * @tc.desc: Adding more than 30 event listener
 * @tc.type: FUNC
 * @tc.require: issueI5KDIG
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventManagerAddTooManyEventListener, TestSize.Level1)
{
    OHOS::HiviewDFX::ListenerRule listenerRule("DOMAIN", "EVENT_NAME", "", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<OHOS::HiviewDFX::ListenerRule> sysRules;
    sysRules.emplace_back(listenerRule);
    int cnt = 30;
    int32_t ret = 0;
    while (cnt-- > 0) {
        ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(std::make_shared<Watcher>(), sysRules);
    }
    ASSERT_EQ(ret, OHOS::HiviewDFX::ERR_TOO_MANY_WATCHERS);
}

/**
 * @tc.name: TestHiSysEventManagerQueryWithTooManyRules
 * @tc.desc: Query with 11 query rules
 * @tc.type: FUNC
 * @tc.require: issueI5L2RV
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventManagerQueryWithTooManyRules, TestSize.Level1)
{
    auto querier = std::make_shared<Querier>();
    long long defaultTimeStap = -1;
    int queryCount = 10;
    struct OHOS::HiviewDFX::QueryArg args(defaultTimeStap, defaultTimeStap, queryCount);
    std::vector<OHOS::HiviewDFX::QueryRule> queryRules;
    int rulesCount = 101; // limit to 100
    while (rulesCount-- > 0) {
        std::vector<std::string> eventNames {"EVENT_NAME"};
        OHOS::HiviewDFX::QueryRule rule("DOMAIN", eventNames);
        queryRules.emplace_back(rule);
    }
    auto ret = OHOS::HiviewDFX::HiSysEventManager::Query(args, queryRules, querier);
    ASSERT_EQ(ret, OHOS::HiviewDFX::ERR_TOO_MANY_QUERY_RULES);
}

/**
 * @tc.name: TestHiSysEventManagerTooManyConcurrentQueries
 * @tc.desc: Query more than 4 times at same time
 * @tc.type: FUNC
 * @tc.require: issueI5L2RV
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventManagerTooManyConcurrentQueries, TestSize.Level1)
{
    auto querier = std::make_shared<Querier>();
    long long defaultTimeStap = -1;
    int queryCount = 10;
    struct OHOS::HiviewDFX::QueryArg args(defaultTimeStap, defaultTimeStap, queryCount);
    std::vector<OHOS::HiviewDFX::QueryRule> queryRules;
    int threadCount = 5;
    auto ret = OHOS::HiviewDFX::IPC_CALL_SUCCEED;
    for (int i = 0; i < threadCount; i++) {
        std::thread t([&ret, &args, &queryRules, &querier] () {
            ret = OHOS::HiviewDFX::HiSysEventManager::Query(args, queryRules, querier);
        });
        t.join();
    }
    ASSERT_TRUE((ret == OHOS::HiviewDFX::ERR_TOO_MANY_CONCURRENT_QUERIES) ||
        (ret == OHOS::HiviewDFX::ERR_QUERY_TOO_FREQUENTLY) ||
        (ret == OHOS::HiviewDFX::IPC_CALL_SUCCEED));
}

/**
 * @tc.name: TestHiSysEventManagerQueryTooFrequently
 * @tc.desc: Query twice in 1 seconds
 * @tc.type: FUNC
 * @tc.require: issueI5L2RV
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventManagerQueryTooFrequently, TestSize.Level1)
{
    auto querier = std::make_shared<Querier>();
    long long defaultTimeStap = -1;
    int queryCount = 10;
    struct OHOS::HiviewDFX::QueryArg args(defaultTimeStap, defaultTimeStap, queryCount);
    std::vector<OHOS::HiviewDFX::QueryRule> queryRules;
    const int threshhold = 50;
    const int delayDuration = 1; // 1 second
    for (int i = 0; i < 2; i++) { // 2 cycles
        sleep(delayDuration);
        for (int j = 0; j <= threshhold; j++) { // more than 50 queries in 1 second is never allowed
            auto ret = OHOS::HiviewDFX::HiSysEventManager::Query(args, queryRules, querier);
            ASSERT_TRUE((ret == OHOS::HiviewDFX::ERR_QUERY_TOO_FREQUENTLY) ||
                (ret == OHOS::HiviewDFX::IPC_CALL_SUCCEED));
        }
    }
}

/**
 * @tc.name: TestInitHiSysEventRecordWithIncorrectStr
 * @tc.desc: Init a hisysevent record with an incorrect string
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestInitHiSysEventRecordWithIncorrectStr, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":\"param a\",";
    HiSysEventRecord record(JSON_STR);
    int64_t val = 0;
    int ret = record.GetParamValue("type_", val);
    ASSERT_EQ(ret, ERR_INIT_FAILED);
}

/**
 * @tc.name: TestParseValueByInvalidKeyFromHiSysEventRecord
 * @tc.desc: Parse value by a invalid key from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseValueByInvalidKeyFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":\"param a\",\"PARAM_B\":\"param b\"}";
    HiSysEventRecord record(JSON_STR);
    int64_t val = 0;
    int ret = record.GetParamValue("XXX", val);
    ASSERT_EQ(ret, ERR_KEY_NOT_EXIST);
}

/**
 * @tc.name: TestParseValueByInvalidTypeFromHiSysEventRecord
 * @tc.desc: Parse value by a invalid type from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseValueByInvalidTypeFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":\"param a\",\"PARAM_B\":\"param b\"}";
    HiSysEventRecord record(JSON_STR);
    int64_t val = 0;
    int ret = record.GetParamValue("PARAM_B", val);
    ASSERT_EQ(ret, ERR_TYPE_NOT_MATCH);
}

/**
 * @tc.name: TestParseEventDomainNameTypeFromHiSysEventRecord
 * @tc.desc: Parse event domain, name and type from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseEventDomainNameTypeFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":\"param a\",\"PARAM_B\":\"param b\"}";
    HiSysEventRecord record(JSON_STR);
    ASSERT_EQ(record.GetDomain(), "DEMO");
    ASSERT_EQ(record.GetEventName(), "EVENT_NAME_A");
    ASSERT_EQ(record.GetEventType(), 4);
}

/**
 * @tc.name: TestParseInt64ValueFromHiSysEventRecord
 * @tc.desc: Parse int64 value from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseInt64ValueFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":-1,\"PARAM_B\":\"param b\"}";
    HiSysEventRecord record(JSON_STR);
    int64_t val = 0;
    int ret = record.GetParamValue("PARAM_A", val);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    ASSERT_EQ(val, -1);
}

/**
 * @tc.name: TestParseUInt64ValueFromHiSysEventRecord
 * @tc.desc: Parse uint64 value from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseUInt64ValueFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3,\"PARAM_B\":\"param b\"}";
    HiSysEventRecord record(JSON_STR);
    uint64_t val = 0;
    int ret = record.GetParamValue("PARAM_A", val);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    ASSERT_EQ(val, 3);
}

/**
 * @tc.name: TestParseDoubleValueFromHiSysEventRecord
 * @tc.desc: Parse double value from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseDoubleValueFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3.4,\"PARAM_B\":\"param b\"}";
    HiSysEventRecord record(JSON_STR);
    double val = 0;
    int ret = record.GetParamValue("PARAM_A", val);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    ASSERT_LT(abs(val - 3.4), 1e-8);
}

/**
 * @tc.name: TestParseStringValueFromHiSysEventRecord
 * @tc.desc: Parse string value from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseStringValueFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3.4,\"PARAM_B\":\"param b\"}";
    HiSysEventRecord record(JSON_STR);
    std::string val;
    int ret = record.GetParamValue("PARAM_B", val);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    ASSERT_EQ(val, "param b");
}

/**
 * @tc.name: TestParseInt64ArrayFromHiSysEventRecord
 * @tc.desc: Parse int64 array from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseInt64ArrayFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3.4,\"PARAM_B\":[-1, 0, 1]}";
    HiSysEventRecord record(JSON_STR);
    std::vector<int64_t> val;
    int ret = record.GetParamValue("PARAM_B", val);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    ASSERT_EQ(val.size(), 3);
    ASSERT_EQ(val[0], -1);
}

/**
 * @tc.name: TestParseUInt64ArrayFromHiSysEventRecord
 * @tc.desc: Parse uint64 array from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseUInt64ArrayFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3.4,\"PARAM_B\":[1, 2, 3]}";
    HiSysEventRecord record(JSON_STR);
    std::vector<uint64_t> val;
    int ret = record.GetParamValue("PARAM_B", val);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    ASSERT_EQ(val.size(), 3);
    ASSERT_EQ(val[0], 1);
}

/**
 * @tc.name: TestParseDoubleArrayFromHiSysEventRecord
 * @tc.desc: Parse double array from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseDoubleArrayFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3.4,\"PARAM_B\":[2.1, 0.0, 3.3]}";
    HiSysEventRecord record(JSON_STR);
    std::vector<double> val;
    int ret = record.GetParamValue("PARAM_B", val);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    ASSERT_EQ(val.size(), 3);
    ASSERT_LT(abs(val[0] - 2.1), 1e-8);
}

/**
 * @tc.name: TestParseStringArrayFromHiSysEventRecord
 * @tc.desc: Parse string array from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseStringArrayFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3.4,\"PARAM_B\":[\"123\", \"456\", \"789\"]}";
    HiSysEventRecord record(JSON_STR);
    std::vector<std::string> val;
    int ret = record.GetParamValue("PARAM_B", val);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    ASSERT_EQ(val.size(), 3);
    ASSERT_EQ(val[0], "123");
}

/**
 * @tc.name: TestParseParamsFromHiSysEventRecord
 * @tc.desc: Parse some inlined parameters from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseParamsFromHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"SAMGR\",\"name_\":\"SAMGR_ADD_SYSTEMABILITY_FAIL\",\"type_\":1,\
        \"time_\":1502114170549,\"tz_\":\"+0000\",\"pid_\":398,\"tid_\":398,\"uid_\":1099,\"SAID\":1155,\
        \"FILE_NAME\":\"libexternal_vpn_service.z.so\",\"level_\":\"CRITICAL\",\"tag_\":\"fault\",\
        \"id_\":\"14947264126694503475\",\"traceid_\":243156040590758234, \"spanid_\":11870123,\
        \"pspanid_\":28408891,\"trace_flag_\":1,\"info_\":\"\"}";
    HiSysEventRecord record(JSON_STR);
    auto time = record.GetTime();
    ASSERT_GT(time, 0);
    auto timeZone = record.GetTimeZone();
    ASSERT_EQ(timeZone.size(), 5);
    auto pid = record.GetPid();
    ASSERT_GT(pid, 0);
    auto tid = record.GetTid();
    ASSERT_GT(tid, 0);
    auto uid = record.GetUid();
    ASSERT_GT(uid, 0);
    auto traceId = record.GetTraceId();
    ASSERT_GE(traceId, 0);
    auto spanId = record.GetSpanId();
    ASSERT_GE(spanId, 0);
    auto pspanId = record.GetPspanId();
    ASSERT_GE(pspanId, 0);
    auto traceFlag = record.GetTraceFlag();
    ASSERT_GE(traceFlag, 0);
    auto level = record.GetLevel();
    ASSERT_EQ(level, "CRITICAL");
    auto tag = record.GetTag();
    ASSERT_GE(timeZone.size(), 0);
    std::vector<std::string> paramNames;
    record.GetParamNames(paramNames);
    ASSERT_TRUE(std::any_of(paramNames.begin(), paramNames.end(), [] (auto& name) {
        return name == "domain_" || name == "name_" || name == "type_";
    }));
}

/**
 * @tc.name: TestParseParamsFromUninitializedHiSysEventRecord
 * @tc.desc: Parse parameters from a uninitialized hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseParamsFromUninitializedHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "";
    HiSysEventRecord record(JSON_STR);
    auto time = record.GetTime();
    ASSERT_EQ(time, 0);
    auto timeZone = record.GetTimeZone();
    ASSERT_EQ(timeZone.size(), 0);
    auto traceId = record.GetTraceId();
    ASSERT_EQ(traceId, 0);
}

/**
 * @tc.name: TestParseWrongTypeParamsFromUninitializedHiSysEventRecord
 * @tc.desc: Parse parameters with unmatched type from a hisysevent record
 * @tc.type: FUNC
 * @tc.require: issueI5OA3F
 */
HWTEST_F(HiSysEventNativeTest, TestParseWrongTypeParamsFromUninitializedHiSysEventRecord, TestSize.Level1)
{
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3.4,\"UINT64_T\":18446744073709551610,\"DOUBLE_T\":3.3,\"INT64_T\":9223372036854775800,\
        \"PARAM_B\":[\"123\", \"456\", \"789\"],\"PARAM_C\":[]}";
    HiSysEventRecord record(JSON_STR);
    double num = 0;
    auto ret = record.GetParamValue("domain_", num);
    ASSERT_EQ(ret, ERR_TYPE_NOT_MATCH);
    std::vector<std::string> paramC;
    ret = record.GetParamValue("name_", paramC);
    ASSERT_EQ(ret, ERR_TYPE_NOT_MATCH);
    ret = record.GetParamValue("PARAM_C", paramC);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    uint64_t uint64TypeParam = 0;
    ret = record.GetParamValue("UINT64_T", uint64TypeParam);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    int64_t int64TypeParam = 0;
    ret = record.GetParamValue("INT64_T", int64TypeParam);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    double doubleTypeParam = 0;
    ret = record.GetParamValue("DOUBLE_T", doubleTypeParam);
    ASSERT_EQ(ret, VALUE_PARSED_SUCCEED);
    double doubleTypeParam2 = 0;
    ret = record.GetParamValue("DOUBLE_T_NOT_EXIST", doubleTypeParam2);
    ASSERT_EQ(ret, ERR_KEY_NOT_EXIST);
}

/**
 * @tc.name: TestHiSysEventManagerQueryWithDefaultQueryArgument
 * @tc.desc: Query with default arugumen
 * @tc.type: FUNC
 * @tc.require: issueI5L2RV
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventManagerQueryWithDefaultQueryArgument, TestSize.Level1)
{
    int eventWroiteCnt = 3;
    for (int index = 0; index < eventWroiteCnt; index++) {
        HiSysEventWrite(TEST_DOMAIN2, "POWER_KEY", HiSysEvent::EventType::FAULT, "DESC", "in test case");
    }
    sleep(2);
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return true;
    }, [] (int32_t reason, int32_t total) {
        return total > 0;
    });
    long long defaultTimeStap = -1; // default value
    int queryCount = -1; // default value
    struct OHOS::HiviewDFX::QueryArg args(defaultTimeStap, defaultTimeStap, queryCount);
    std::vector<OHOS::HiviewDFX::QueryRule> queryRules;
    std::vector<std::string> eventNames {"POWER_KEY"};
    OHOS::HiviewDFX::QueryRule rule("KERNEL_VENDOR", eventNames); // empty domain
    queryRules.emplace_back(rule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::Query(args, queryRules, querier);
    ASSERT_EQ(ret, OHOS::HiviewDFX::IPC_CALL_SUCCEED);
}

/**
 * @tc.name: TestEventSocketFactory1
 * @tc.desc: Test apis of EventSocketFactory
 * @tc.type: FUNC
 * @tc.require: issueIC70PG
 */
HWTEST_F(HiSysEventNativeTest, TestEventSocketFactory1, TestSize.Level1)
{
    RawData data;
    BuildRawData(data, "AAFWK", "APP_INPUT_BLOCK", HiSysEvent::EventType::FAULT);
    auto socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "AAFWK", "BUSSINESS_THREAD_BLOCK_3S", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "AAFWK", "BUSSINESS_THREAD_BLOCK_6S", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "AAFWK", "LIFECYCLE_HALF_TIMEOUT", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "AAFWK", "LIFECYCLE_TIME_OUT", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "AAFWK", "THREAD_BLOCK_3S", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "AAFWK", "THREAD_BLOCK_6S", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "AAFWK", "EXCLUDED_NAME", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent");
}

/**
 * @tc.name: TestEventSocketFactory2
 * @tc.desc: Test apis of EventSocketFactory
 * @tc.type: FUNC
 * @tc.require: issueIC70PG
 */
HWTEST_F(HiSysEventNativeTest, TestEventSocketFactory2, TestSize.Level1)
{
    RawData data;
    BuildRawData(data, "ACE", "UI_BLOCK_3S", HiSysEvent::EventType::FAULT);
    auto socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "ACE", "UI_BLOCK_6S", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "ACE", "UI_BLOCK_RECOVERED", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "ACE", "EXCLUDED_NAME", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent");
}

/**
 * @tc.name: TestEventSocketFactory3
 * @tc.desc: Test apis of EventSocketFactory
 * @tc.type: FUNC
 * @tc.require: issueIC70PG
 */
HWTEST_F(HiSysEventNativeTest, TestEventSocketFactory3, TestSize.Level1)
{
    RawData data;
    BuildRawData(data, "FRAMEWORK", "IPC_FULL", HiSysEvent::EventType::FAULT);
    auto socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "FRAMEWORK", "SERVICE_BLOCK", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "FRAMEWORK", "SERVICE_TIMEOUT", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "FRAMEWORK", "SERVICE_WARNING", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "FRAMEWORK", "EXCLUDED_NAME", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent");
}

/**
 * @tc.name: TestEventSocketFactory4
 * @tc.desc: Test apis of EventSocketFactory
 * @tc.type: FUNC
 * @tc.require: issueIC70PG
 */
HWTEST_F(HiSysEventNativeTest, TestEventSocketFactory4, TestSize.Level1)
{
    RawData data;
    BuildRawData(data, "RELIABILITY", "ANY_NAME", HiSysEvent::EventType::FAULT);
    auto socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "RELIABILITY", "ANY_NAME", HiSysEvent::EventType::BEHAVIOR);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent");
}

/**
 * @tc.name: TestEventSocketFactory5
 * @tc.desc: Test apis of EventSocketFactory
 * @tc.type: FUNC
 * @tc.require: issueIC70PG
 */
HWTEST_F(HiSysEventNativeTest, TestEventSocketFactory5, TestSize.Level1)
{
    RawData data;
    BuildRawData(data, "GRAPHIC", "NO_DRAW", HiSysEvent::EventType::FAULT);
    auto socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "GRAPHIC", "EXCLUDED_NAME", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent");

    BuildRawData(data, "MULTIMODALINPUT", "TARGET_POINTER_EVENT_FAILURE", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "MULTIMODALINPUT", "EXCLUDED_NAME", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent");

    BuildRawData(data, "POWER", "SCREEN_ON_TIMEOUT", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "POWER", "EXCLUDED_NAME", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent");

    BuildRawData(data, "WINDOWMANAGER", "NO_FOCUS_WINDOW", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "WINDOWMANAGER", "EXCLUDED_NAME", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent");

    BuildRawData(data, "SCHEDULE_EXT", "SYSTEM_LOAD_LEVEL_CHANGED", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent_fast");
    BuildRawData(data, "SCHEDULE_EXT", "EXCLUDED_NAME", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent");

    BuildRawData(data, "EXCLUDED_DOMAIN", "ANY_NAME", HiSysEvent::EventType::FAULT);
    socketAddr = EventSocketFactory::GetEventSocket(data);
    ASSERT_EQ(std::string(socketAddr.sun_path), "/dev/unix/socket/hisysevent");
}

