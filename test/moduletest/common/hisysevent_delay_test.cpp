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

#include "hisysevent_delay_test.h"

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
#include "hisysevent.h"
#include "string_ex.h"
#include "string_util.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENT_DELAY_TEST" };
constexpr int WROTE_TOTAL_CNT = 30;
constexpr int MILL_TO_MICRO = 1000;

long long GetSystemNanos()
{
    struct timespec ts {};
    (void)clock_gettime(CLOCK_REALTIME, &ts);
    long long nanoseconds = (ts.tv_sec * MILL_TO_MICRO * MILL_TO_MICRO * MILL_TO_MICRO) + ts.tv_nsec;
    return nanoseconds;
}

void WriteStringWithLength(const std::string testCaseName, const std::string testCaseDescription, int cnt)
{
    HiLog::Info(LABEL, "================= %{public}s start =============", testCaseName.c_str());
    HiLog::Info(LABEL, "================= %{public}s =============", testCaseDescription.c_str());
    string param;
    param.append(cnt, 'a');
    std::vector<int> wroteRet;
    long long start = GetSystemNanos();
    int ret = SUCCESS;
    for (int i = 0; i < WROTE_TOTAL_CNT; ++i) {
        ret = HiSysEventWrite(HiSysEvent::Domain::AAFWK, "LIFECYCLE_TIMEOUT", HiSysEvent::EventType::FAULT, "key",
            param);
        wroteRet.emplace_back(ret);
    }
    long long finish = GetSystemNanos();
    HiLog::Info(LABEL, "this time cost is %{public}lld us", (finish - start) / MILL_TO_MICRO);
    HiLog::Info(LABEL, "this avarage time is %{public}lld us", (finish - start) / MILL_TO_MICRO / WROTE_TOTAL_CNT);
    ASSERT_TRUE((wroteRet.size() == WROTE_TOTAL_CNT) &&
        (std::count(wroteRet.begin(), wroteRet.end(), SUCCESS) == WROTE_TOTAL_CNT));
    HiLog::Info(LABEL, "================= %{public}s end =============", testCaseName.c_str());
}

template<typename T>
void WriteSingleValue(const std::string testCaseName, const std::string testCaseDescription, T val)
{
    HiLog::Info(LABEL, "================= %{public}s start =============", testCaseName.c_str());
    HiLog::Info(LABEL, "================= %{public}s =============", testCaseDescription.c_str());
    std::vector<int> wroteRet;
    long long start = GetSystemNanos();
    int ret = SUCCESS;
    for (int i = 0; i < WROTE_TOTAL_CNT; ++i) {
        ret = HiSysEventWrite(HiSysEvent::Domain::AAFWK, "LIFECYCLE_TIMEOUT", HiSysEvent::EventType::FAULT, "key",
            val);
        wroteRet.emplace_back(ret);
    }
    long long finish = GetSystemNanos();
    HiLog::Info(LABEL, "this time cost is %{public}lld us", (finish - start) / MILL_TO_MICRO);
    HiLog::Info(LABEL, "this avarage time is %{public}lld us", (finish - start) / MILL_TO_MICRO / WROTE_TOTAL_CNT);
    ASSERT_TRUE((wroteRet.size() == WROTE_TOTAL_CNT) &&
        (std::count(wroteRet.begin(), wroteRet.end(), SUCCESS) == WROTE_TOTAL_CNT));
    HiLog::Info(LABEL, "================= %{public}s end =============", testCaseName.c_str());
}
}

void HiSysEventDelayTest::SetUpTestCase(void)
{
}

void HiSysEventDelayTest::TearDownTestCase(void)
{
}

void HiSysEventDelayTest::SetUp(void)
{
}

void HiSysEventDelayTest::TearDown(void)
{
}

/**
 * @tc.name: HiSysEventDelayTest001
 * @tc.desc: Write a sysevent without any parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest001, TestSize.Level1)
{
    HiLog::Info(LABEL, "================= HiSysEventDelayTest001 start =============");
    HiLog::Info(LABEL, "================= Write a sysevent without any parameter =============");
    std::vector<int> wroteRet;
    long long start = GetSystemNanos();
    int ret = SUCCESS;
    for (int i = 0; i < WROTE_TOTAL_CNT; ++i) {
        ret = HiSysEventWrite(HiSysEvent::Domain::AAFWK, "LIFECYCLE_TIMEOUT", HiSysEvent::EventType::FAULT);
        wroteRet.emplace_back(ret);
    }
    long long finish = GetSystemNanos();
    HiLog::Info(LABEL, "this time cost is %{public}lld us", (finish - start) / MILL_TO_MICRO);
    HiLog::Info(LABEL, "this avarage time is %{public}lld us", (finish - start) / MILL_TO_MICRO / WROTE_TOTAL_CNT);
    ASSERT_TRUE((wroteRet.size() == WROTE_TOTAL_CNT) &&
        (std::count(wroteRet.begin(), wroteRet.end(), SUCCESS) == WROTE_TOTAL_CNT));
    HiLog::Info(LABEL, "================= HiSysEventDelayTest001 end =============");
}

/**
 * @tc.name: HiSysEventDelayTest002
 * @tc.desc: Write sysevent with a bool parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest002, TestSize.Level1)
{
    bool value = true;
    WriteSingleValue("HiSysEventDelayTest002", "Write sysevent with a bool parameter", value);
}

/**
 * @tc.name: HiSysEventDelayTest003
 * @tc.desc: Write sysevent with a char parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest003, TestSize.Level1)
{
    char value = 'a';
    WriteSingleValue("HiSysEventDelayTest003", "Write sysevent with a char parameter", value);
}

/**
 * @tc.name: HiSysEventDelayTest004
 * @tc.desc: Write sysevent with a double parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest004, TestSize.Level1)
{
    double value = 30949.374;
    WriteSingleValue("HiSysEventDelayTest004", "Write sysevent with a double parameter", value);
}

/**
 * @tc.name: HiSysEventDelayTest005
 * @tc.desc: Write sysevent with a float parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest005, TestSize.Level1)
{
    float value = 230.47;
    WriteSingleValue("HiSysEventDelayTest005", "Write sysevent with a float parameter", value);
}

/**
 * @tc.name: HiSysEventDelayTest006
 * @tc.desc: Write sysevent with a integer parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest006, TestSize.Level1)
{
    int value = 100;
    WriteSingleValue("HiSysEventDelayTest006", "Write sysevent with a integer parameter", value);
}

/**
 * @tc.name: HiSysEventDelayTest007
 * @tc.desc: Write sysevent with a long parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest007, TestSize.Level1)
{
    long value = 1000000;
    WriteSingleValue("HiSysEventDelayTest007", "Write sysevent with a long parameter", value);
}

/**
 * @tc.name: HiSysEventDelayTest008
 * @tc.desc: Write sysevent with a short parameter
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest008, TestSize.Level1)
{
    short value = 10;
    WriteSingleValue("HiSysEventDelayTest008", "Write sysevent with a short parameter", value);
}

/**
 * @tc.name: HiSysEventDelayTest009
 * @tc.desc: Write a sysevent with a string param whose length is 32
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest009, TestSize.Level1)
{
    WriteStringWithLength("HiSysEventDelayTest009", "Write a sysevent with a string param whose length is 32",
        32);
}

/**
 * @tc.name: HiSysEventDelayTest010
 * @tc.desc: Write a sysevent with a string param whose length is 64
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest010, TestSize.Level1)
{
    WriteStringWithLength("HiSysEventDelayTest010", "Write a sysevent with a string param whose length is 64",
        64);
}

/**
 * @tc.name: HiSysEventDelayTest011
 * @tc.desc: Write a sysevent with a string param whose length is 128
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest011, TestSize.Level1)
{
    WriteStringWithLength("HiSysEventDelayTest011", "Write a sysevent with a string param whose length is 128",
        128);
}

/**
 * @tc.name: HiSysEventDelayTest012
 * @tc.desc: Write a sysevent with a string param whose length is 256
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest012, TestSize.Level1)
{
    WriteStringWithLength("HiSysEventDelayTest012", "Write a sysevent with a string param whose length is 256",
        256);
}


/**
 * @tc.name: HiSysEventDelayTest013
 * @tc.desc: Write a sysevent with a string param whose length is 512
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest013, TestSize.Level1)
{
    WriteStringWithLength("HiSysEventDelayTest013", "Write a sysevent with a string param whose length is 512",
        512);
}

/**
 * @tc.name: HiSysEventDelayTest014
 * @tc.desc: Write a sysevent with a string param whose length is 1024
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest014, TestSize.Level1)
{
    WriteStringWithLength("HiSysEventDelayTest014", "Write a sysevent with a string param whose length is 1024",
        1024);
}

/**
 * @tc.name: HiSysEventDelayTest015
 * @tc.desc: Write a sysevent with a string param whose length is 2048
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest015, TestSize.Level1)
{
    WriteStringWithLength("HiSysEventDelayTest015", "Write a sysevent with a string param whose length is 2048",
        2048);
}

/**
 * @tc.name: HiSysEventDelayTest016
 * @tc.desc: Write a sysevent with a string param whose length is 3072
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest016, TestSize.Level1)
{
    WriteStringWithLength("HiSysEventDelayTest016", "Write a sysevent with a string param whose length is 3072",
        3072);
}

/**
 * @tc.name: HiSysEventDelayTest017
 * @tc.desc: Write a sysevent with a string param whose length is 4096
 * @tc.type: FUNC
 * @tc.require: issueI76V6J
 */
HWTEST_F(HiSysEventDelayTest, HiSysEventDelayTest017, TestSize.Level1)
{
    WriteStringWithLength("HiSysEventDelayTest017", "Write a sysevent with a string param whose length is 4096",
        4096);
}