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

#include "hisysevent_wrote_result_check.h"

#include <functional>
#include <iosfwd>
#include <limits>
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
#include "hisysevent.h"
#include "hisysevent_base_manager.h"
#include "hisysevent_manager.h"
#include "hisysevent_record.h"
#include "hisysevent_query_callback.h"
#include "hisysevent_listener.h"
#ifdef HIVIEWDFX_HITRACE_ENABLED_FOR_TEST
#include "hitrace/trace.h"
#endif
#include "ret_code.h"
#include "rule_type.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENTTEST_WRITE_RESULT_CHECK_TEST"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr char DOMAIN[] = "KERNEL_VENDOR";
constexpr char EVENT_NAME[] = "POWER_KEY";
constexpr char PARAM_KEY[] = "key";
constexpr int ARRAY_TOTAL_CNT = 3;
constexpr int FIRST_ITEM_INDEX = 0;
constexpr int SECOND_ITEM_INDEX = 1;
constexpr int THIRD_ITEM_INDEX = 2;
constexpr int USLEEP_DURATION = 1000000;
constexpr int EVENT_QUERY_CNT = 1000;
constexpr long long DEFAULT_TIME_STAMP = -1;

class Watcher : public HiSysEventListener {
public:
    explicit Watcher(std::function<bool(std::shared_ptr<HiSysEventRecord>)> assertFunc)
    {
        assertFunc_ = assertFunc;
    }

    virtual ~Watcher() {}

    void OnEvent(std::shared_ptr<HiSysEventRecord> sysEvent) final
    {
        if (sysEvent == nullptr || assertFunc_ == nullptr) {
            return;
        }
        ASSERT_TRUE(assertFunc_(sysEvent));
    }

    void OnServiceDied() final
    {
        HILOG_DEBUG(LOG_CORE, "OnServiceDied");
    }

private:
    std::function<bool(std::shared_ptr<HiSysEventRecord>)> assertFunc_;
};

class Querier : public HiSysEventQueryCallback {
public:
    explicit Querier(std::function<bool(std::shared_ptr<std::vector<HiSysEventRecord>>)> onQueryHandleFunc)
    {
        onQueryHandleFunc_ = onQueryHandleFunc;
    }

    virtual ~Querier() {}

    void OnQuery(std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents)
    {
        matched |= onQueryHandleFunc_(sysEvents);
    }

    void OnComplete(int32_t reason, int32_t total)
    {
        ASSERT_TRUE(matched);
    }

private:
    std::function<bool(std::shared_ptr<std::vector<HiSysEventRecord>>)> onQueryHandleFunc_;
    bool matched = false;
};

void Sleep()
{
    usleep(USLEEP_DURATION);
}

void QueryAndComparedSysEvent(std::shared_ptr<Querier> querier)
{
    struct OHOS::HiviewDFX::QueryArg args(DEFAULT_TIME_STAMP, DEFAULT_TIME_STAMP, EVENT_QUERY_CNT);
    std::vector<OHOS::HiviewDFX::QueryRule> queryRules;
    std::vector<std::string> eventNames {EVENT_NAME};
    OHOS::HiviewDFX::QueryRule rule(DOMAIN, eventNames);
    queryRules.emplace_back(rule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::Query(args, queryRules, querier);
    ASSERT_TRUE(ret == SUCCESS);
}

template<typename T>
void WriteAndWatchThenQuery(std::shared_ptr<Watcher> watcher, std::shared_ptr<Querier> querier, T& val)
{
    OHOS::HiviewDFX::ListenerRule listenerRule(DOMAIN, EVENT_NAME, "",
        OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<OHOS::HiviewDFX::ListenerRule> sysRules;
    sysRules.emplace_back(listenerRule);
    auto ret = OHOS::HiviewDFX::HiSysEventManager::AddListener(watcher, sysRules);
    ASSERT_TRUE(ret == SUCCESS);
    ret = HiSysEventWrite(DOMAIN, EVENT_NAME, HiSysEvent::EventType::FAULT, PARAM_KEY,
        val);
    ASSERT_TRUE(ret == SUCCESS);
    Sleep();
    ret = OHOS::HiviewDFX::HiSysEventManager::RemoveListener(watcher);
    ASSERT_TRUE(ret == SUCCESS);
    QueryAndComparedSysEvent(querier);
}

bool IsContains(const std::string& total, const std::string& part)
{
    if (total.empty() || part.empty()) {
        return false;
    }
    return total.find(part) != std::string::npos;
}

bool CompareEventQueryResultWithPattern(std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents,
    const std::string& pattern)
{
    if (sysEvents == nullptr) {
        return false;
    }
    auto matched = false;
    for (auto sysEvent : *sysEvents) {
        std::string eventJsonStr = sysEvent.AsJson();
        auto comparedRet = IsContains(eventJsonStr, pattern);
        matched |= comparedRet;
        if (matched) {
            break;
        }
    }
    return matched;
}
}

void HiSysEventWroteResultCheckTest::SetUpTestCase(void)
{
}

void HiSysEventWroteResultCheckTest::TearDownTestCase(void)
{
}

void HiSysEventWroteResultCheckTest::SetUp(void)
{
}

void HiSysEventWroteResultCheckTest::TearDown(void)
{
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest001
 * @tc.desc: Write sysevent with bool parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest001, TestSize.Level1)
{
    Sleep();
    bool val = true;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        int64_t ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == static_cast<int>(val);
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":1,");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest002
 * @tc.desc: Write sysevent with int64_t parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest002, TestSize.Level1)
{
    Sleep();
    int64_t val = -18888888882321;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        int64_t ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":-18888888882321,");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest003
 * @tc.desc: Write sysevent with uint64_t parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest003, TestSize.Level1)
{
    Sleep();
    uint64_t val = 18888888882326141;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        uint64_t ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":18888888882326141,");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest004
 * @tc.desc: Write sysevent with double parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest004, TestSize.Level1)
{
    Sleep();
    double val = 30949.374;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::string eventJsonStr = sysEvent->AsJson();
        return IsContains(eventJsonStr, "\"key\":30949.4,");
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":30949.4,");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest005
 * @tc.desc: Write sysevent with string parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest005, TestSize.Level1)
{
    Sleep();
    std::string val = "value";
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::string ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":\"value\"");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest006
 * @tc.desc: Write sysevent with bool array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest006, TestSize.Level1)
{
    Sleep();
    std::vector<bool> val = {
        true,
        false,
        true
    };
    auto watcher = std::make_shared<Watcher>([&val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::vector<int64_t> ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return (ret.size() == ARRAY_TOTAL_CNT) && (val[FIRST_ITEM_INDEX] == ret[FIRST_ITEM_INDEX]) &&
            (val[SECOND_ITEM_INDEX] == ret[SECOND_ITEM_INDEX]) && (val[THIRD_ITEM_INDEX] == ret[THIRD_ITEM_INDEX]);
    });
    auto querier =std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":[1,0,1]");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest007
 * @tc.desc: Write sysevent with int64_t array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest007, TestSize.Level1)
{
    Sleep();
    std::vector<int64_t> val = {
        std::numeric_limits<int64_t>::min(),
        std::numeric_limits<int64_t>::max(),
        -3333333333333333333,
    };
    auto watcher = std::make_shared<Watcher>([&val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::vector<int64_t> ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return (ret.size() == ARRAY_TOTAL_CNT) && (val[FIRST_ITEM_INDEX] == ret[FIRST_ITEM_INDEX]) &&
            (val[SECOND_ITEM_INDEX] == ret[SECOND_ITEM_INDEX]) && (val[THIRD_ITEM_INDEX] == ret[THIRD_ITEM_INDEX]);
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":[" +
            std::to_string(std::numeric_limits<int64_t>::min()) + "," +
            std::to_string(std::numeric_limits<int64_t>::max()) + ",-3333333333333333333]");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest008
 * @tc.desc: Write sysevent with uint64_t array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest008, TestSize.Level1)
{
    Sleep();
    std::vector<uint64_t> val = {
        std::numeric_limits<uint64_t>::min(),
        std::numeric_limits<uint64_t>::max(),
        3333333333333333333,
    };
    auto watcher = std::make_shared<Watcher>([&val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::vector<uint64_t> ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return (ret.size() == ARRAY_TOTAL_CNT) && (val[FIRST_ITEM_INDEX] == ret[FIRST_ITEM_INDEX]) &&
            (val[SECOND_ITEM_INDEX] == ret[SECOND_ITEM_INDEX]) && (val[THIRD_ITEM_INDEX] == ret[THIRD_ITEM_INDEX]);
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":[" +
            std::to_string(std::numeric_limits<uint64_t>::min()) + "," +
            std::to_string(std::numeric_limits<uint64_t>::max()) + ",3333333333333333333]");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest009
 * @tc.desc: Write sysevent with double array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest009, TestSize.Level1)
{
    Sleep();
    std::vector<double> val = {
        1.5,
        2.5,
        100.374,
    };
    auto watcher = std::make_shared<Watcher>([&val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::string eventJsonStr = sysEvent->AsJson();
        return IsContains(eventJsonStr, "\"key\":[1.5,2.5,100.374],");
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":[1.5,2.5,100.374]");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest010
 * @tc.desc: Write sysevent with string array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest010, TestSize.Level1)
{
    Sleep();
    std::vector<std::string> val = {
        "value1\n\r",
        "value2\n\r",
        "value3\n\r",
    };
    auto watcher = std::make_shared<Watcher>([&val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::vector<std::string> ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return (ret.size() == ARRAY_TOTAL_CNT) && (ret[FIRST_ITEM_INDEX] == "value1\n\r") &&
            (ret[SECOND_ITEM_INDEX] == "value2\n\r") && (ret[THIRD_ITEM_INDEX] == "value3\n\r");
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents,
            "\"key\":[\"value1\\n\\r\",\"value2\\n\\r\",\"value3\\n\\r\"]");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest011
 * @tc.desc: Write sysevent with float parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest011, TestSize.Level1)
{
    Sleep();
    float val = 230.47;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::string eventJsonStr = sysEvent->AsJson();
        return IsContains(eventJsonStr, "\"key\":230.47,");
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":230.47");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest012
 * @tc.desc: Write sysevent with float array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest012, TestSize.Level1)
{
    Sleep();
    std::vector<float> val = {
        1.1,
        2.2,
        3.5,
        4,
    };
    auto watcher = std::make_shared<Watcher>([&val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::string eventJsonStr = sysEvent->AsJson();
        return IsContains(eventJsonStr, "\"key\":[1.1,2.2,3.5,4],");
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":[1.1,2.2,3.5,4]");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

#ifdef HIVIEWDFX_HITRACE_ENABLED_FOR_TEST
/**
 * @tc.name: HiSysEventWroteResultCheckTest013
 * @tc.desc: Write sysevent after begin hitracechain
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest013, TestSize.Level1)
{
    Sleep();
    std::string val = "with valid hitracechain";
    auto traceId = HiTraceChain::Begin("TestCase1", HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN);
    auto watcher = std::make_shared<Watcher>([&val, &traceId] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        return (traceId.GetFlags() == sysEvent->GetTraceFlag()) && (traceId.GetChainId() == sysEvent->GetTraceId());
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":\"with valid hitracechain\",");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
    HiTraceChain::End(traceId);
}
#endif

/**
 * @tc.name: HiSysEventWroteResultCheckTest014
 * @tc.desc: Write sysevent with negative double parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest014, TestSize.Level1)
{
    Sleep();
    double val = -3.5;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::string eventJsonStr = sysEvent->AsJson();
        return IsContains(eventJsonStr, "\"key\":-3.5,");
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":-3.5,");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest015
 * @tc.desc: Write sysevent with empty array
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest015, TestSize.Level1)
{
    Sleep();
    std::vector<float> val;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::string eventJsonStr = sysEvent->AsJson();
        return IsContains(eventJsonStr, "\"key\":[],");
    });
    auto querier = std::make_shared<Querier>([] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":[]");
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest016
 * @tc.desc: Write sysevent with maximum int64_t value
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest016, TestSize.Level1)
{
    Sleep();
    int64_t val = std::numeric_limits<int64_t>::max();
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        int64_t ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    auto querier = std::make_shared<Querier>([val] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":" +
            std::to_string(val));
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest017
 * @tc.desc: Write sysevent with minimum int64_t value
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest017, TestSize.Level1)
{
    Sleep();
    int64_t val = std::numeric_limits<int64_t>::min();
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        int64_t ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    auto querier = std::make_shared<Querier>([val] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":" +
            std::to_string(val));
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest018
 * @tc.desc: Write sysevent with maximum uint64_t value
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest018, TestSize.Level1)
{
    Sleep();
    uint64_t val = std::numeric_limits<uint64_t>::max();
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        uint64_t ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    auto querier = std::make_shared<Querier>([val] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":" +
            std::to_string(val));
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest019
 * @tc.desc: Write sysevent with minimum uint64_t value
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest019, TestSize.Level1)
{
    Sleep();
    uint64_t val = std::numeric_limits<uint64_t>::min();
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        uint64_t ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    auto querier = std::make_shared<Querier>([val] (std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents) {
        return CompareEventQueryResultWithPattern(sysEvents, "\"key\":" +
            std::to_string(val));
    });
    WriteAndWatchThenQuery(watcher, querier, val);
}