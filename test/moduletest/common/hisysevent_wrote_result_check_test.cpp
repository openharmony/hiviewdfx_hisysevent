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
#include "ret_code.h"
#include "rule_type.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENTTEST_WRITE_RESULT_CHECK_TEST" };
constexpr char DOMAIN[] = "KERNEL_VENDOR";
constexpr char EVENT_NAME[] = "POWER_KEY";
constexpr char PARAM_KEY[] = "key";
constexpr int ARRAY_TOTAL_CNT = 3;
constexpr int FIRST_ITEM_INDEX = 0;
constexpr int SECOND_ITEM_INDEX = 1;
constexpr int THIRD_ITEM_INDEX = 2;

class Watcher : public HiSysEventListener {
public:
    explicit Watcher(std::function<bool(std::shared_ptr<HiSysEventRecord>)> assertFunc)
    {
        assertFunc_ = assertFunc;
    }

    virtual ~Watcher() {}

    void OnEvent(std::shared_ptr<HiSysEventRecord> sysEvent) final
    {
        HiLog::Info(LABEL, "================= OnEvent =============");
        if (sysEvent == nullptr || assertFunc_ == nullptr) {
            return;
        }
        ASSERT_TRUE(assertFunc_(sysEvent));
    }

    void OnServiceDied() final
    {
        HiLog::Debug(LABEL, "OnServiceDied");
    }

private:
    std::function<bool(std::shared_ptr<HiSysEventRecord>)> assertFunc_;
};

template<typename T>
void WriteAndWatch(std::shared_ptr<Watcher> watcher, T& val)
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
    sleep(1);
    ret = OHOS::HiviewDFX::HiSysEventManager::RemoveListener(watcher);
    ASSERT_TRUE(ret == SUCCESS);
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
    sleep(1);
    bool val = true;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        int64_t ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == static_cast<int>(val);
    });
    WriteAndWatch(watcher, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest002
 * @tc.desc: Write sysevent with int64_t parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest002, TestSize.Level1)
{
    sleep(1);
    int64_t val = -18888888882321;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        int64_t ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    WriteAndWatch(watcher, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest003
 * @tc.desc: Write sysevent with uint64_t parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest003, TestSize.Level1)
{
    sleep(1);
    uint64_t val = 18888888882326141;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        uint64_t ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    WriteAndWatch(watcher, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest004
 * @tc.desc: Write sysevent with floating parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest004, TestSize.Level1)
{
    sleep(1);
    double val = -3.5;
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        double ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    WriteAndWatch(watcher, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest005
 * @tc.desc: Write sysevent with string parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest005, TestSize.Level1)
{
    sleep(1);
    std::string val = "value";
    auto watcher = std::make_shared<Watcher>([val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::string ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return ret == val;
    });
    WriteAndWatch(watcher, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest006
 * @tc.desc: Write sysevent with bool array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest006, TestSize.Level1)
{
    sleep(1);
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
    WriteAndWatch(watcher, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest007
 * @tc.desc: Write sysevent with int64_t array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest007, TestSize.Level1)
{
    sleep(1);
    std::vector<int64_t> val = {
        1111111111111111,
        -222222222222222222,
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
    WriteAndWatch(watcher, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest008
 * @tc.desc: Write sysevent with uint64_t array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest008, TestSize.Level1)
{
    sleep(1);
    std::vector<uint64_t> val = {
        1111111111111111,
        222222222222222222,
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
    WriteAndWatch(watcher, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest009
 * @tc.desc: Write sysevent with double array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest009, TestSize.Level1)
{
    sleep(1);
    std::vector<double> val = {
        1.5,
        2.5,
        100000000.5,
    };
    auto watcher = std::make_shared<Watcher>([&val] (std::shared_ptr<HiSysEventRecord> sysEvent) {
        if (sysEvent == nullptr) {
            return false;
        }
        std::vector<double> ret;
        sysEvent->GetParamValue(PARAM_KEY, ret);
        return (ret.size() == ARRAY_TOTAL_CNT) && (val[FIRST_ITEM_INDEX] == ret[FIRST_ITEM_INDEX]) &&
            (val[SECOND_ITEM_INDEX] == ret[SECOND_ITEM_INDEX]) && (val[THIRD_ITEM_INDEX] == ret[THIRD_ITEM_INDEX]);
    });
    WriteAndWatch(watcher, val);
}

/**
 * @tc.name: HiSysEventWroteResultCheckTest010
 * @tc.desc: Write sysevent with string array parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventWroteResultCheckTest, HiSysEventWroteResultCheckTest010, TestSize.Level1)
{
    sleep(1);
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
    WriteAndWatch(watcher, val);
}
