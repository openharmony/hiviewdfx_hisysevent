/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "ash_mem_utils.h"
#include "hisysevent_base_listener.h"
#include "hisysevent_delegate.h"
#include "hisysevent_rules.h"
#include "query_argument.h"
#include "ret_code.h"
#include "string_ex.h"


using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;

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
    AshMemUtils::WriteBulkData(data, src);
    std::vector<std::u16string> dest;
    AshMemUtils::ReadBulkData(data, dest);
    ASSERT_TRUE(src.size() == dest.size());
    ASSERT_TRUE(Str16ToStr8(dest[0]) == "0" &&
        Str16ToStr8(dest[1]) == "1");
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
    ASSERT_TRUE(ret == ERR_LISTENER_NOT_EXIST);
    ret = delegate->SetDebugMode(nullptr, true);
    ASSERT_TRUE(ret == ERR_LISTENER_NOT_EXIST);
    auto listener = std::make_shared<HiSysEventBaseListener>();
    std::vector<ListenerRule> rules;
    ListenerRule listenerRule("DOMAIN", "EVENT_NAME", "TAG", RuleType::WHOLE_WORD);
    rules.emplace_back(listenerRule);
    ret = delegate->AddListener(listener, rules);
    ASSERT_TRUE(ret == IPC_CALL_SUCCEED);
    ret = delegate->SetDebugMode(listener, true);
    if (ret == IPC_CALL_SUCCEED) {
        ret = delegate->SetDebugMode(listener, false);
        ASSERT_TRUE(ret == IPC_CALL_SUCCEED);
    }
    ret = delegate->RemoveListener(listener);
    ASSERT_TRUE(ret == IPC_CALL_SUCCEED);
    long long defaultTimeStap = -1;
    int queryCount = 10;
    struct QueryArg args(defaultTimeStap, defaultTimeStap, queryCount);
    std::vector<QueryRule> queryRules;
    std::vector<std::string> eventNames {"EVENT_NAME"};
    QueryRule rule("DOMAIN", eventNames);
    queryRules.emplace_back(rule);
    auto baseQuerier = std::make_shared<HiSysEventBaseQueryCallback>();
    ret = delegate->Query(args, queryRules, baseQuerier);
    ASSERT_TRUE(ret == IPC_CALL_SUCCEED);
}