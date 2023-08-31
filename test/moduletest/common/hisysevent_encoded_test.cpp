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

#include <gtest/gtest.h>

#include <limits>
#include <memory>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"

#include "encoded_param.h"
#include "raw_data_base_def.h"
#include "raw_data_builder.h"
#include "raw_data_encoder.h"
#include "raw_data.h"

#include "hisysevent.h"
#include "hitrace/trace.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::Encoded;

class HiSysEventEncodedTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HiSysEventEncodedTest::SetUpTestCase(void)
{
}

void HiSysEventEncodedTest::TearDownTestCase(void)
{
}

void HiSysEventEncodedTest::SetUp(void)
{
}

void HiSysEventEncodedTest::TearDown(void)
{
}

/**
 * @tc.name: RawDataTest001
 * @tc.desc: RawData constructor test
 * @tc.type: FUNC
 * @tc.require: issueI7E737
 */
HWTEST_F(HiSysEventEncodedTest, RawDataTest001, TestSize.Level1)
{
    auto dataBuilderEmpty = std::make_shared<RawDataBuilder>();
    ASSERT_TRUE(dataBuilderEmpty != nullptr);
    RawData originEmpty = *(dataBuilderEmpty->Build());
    RawData dest1(originEmpty.GetData(), originEmpty.GetDataLength());
    ASSERT_TRUE(dest1.GetData() != nullptr);
    RawData dest2(originEmpty);
    ASSERT_TRUE(dest2.GetData() != nullptr);
    auto dataBuilder = std::make_shared<RawDataBuilder>();
    ASSERT_TRUE(dataBuilder != nullptr);
    dataBuilder->AppendDomain("DOMAIN");
    dataBuilder->AppendName("NAME");
    dataBuilder->AppendType(HiSysEvent::EventType::SECURITY);
    RawData origin = *(dataBuilder->Build());
    RawData dest3(origin.GetData(), origin.GetDataLength());
    ASSERT_TRUE((dest3.GetDataLength() > 0) && (dest3.GetData() != nullptr));
    RawData dest4(origin);
    ASSERT_TRUE((dest4.GetDataLength() > 0) && (dest4.GetData() != nullptr));
}

/**
 * @tc.name: RawDataTest002
 * @tc.desc: RawData assign, reset and update api test
 * @tc.type: FUNC
 * @tc.require: issueI7E737
 */
HWTEST_F(HiSysEventEncodedTest, RawDataTest002, TestSize.Level1)
{
    auto dataBuilderEmpty = std::make_shared<RawDataBuilder>();
    ASSERT_TRUE(dataBuilderEmpty != nullptr);
    RawData originEmpty = *(dataBuilderEmpty->Build());
    RawData tmpData = originEmpty;
    originEmpty.Reset();
    ASSERT_TRUE(originEmpty.GetDataLength() == 0);
    auto ret = originEmpty.Update(nullptr, 0, 0);
    ASSERT_TRUE(!ret);
    auto dataBuilder = std::make_shared<RawDataBuilder>();
    ASSERT_TRUE(dataBuilder != nullptr);
    dataBuilder->AppendDomain("DOMAIN");
    dataBuilder->AppendName("NAME");
    RawData origin = *(dataBuilderEmpty->Build());
    ret = originEmpty.Update(origin.GetData(), origin.GetDataLength(), 100); // 100 is an invalid position
    ASSERT_TRUE(!ret);
    RawData tmpData2 = origin;
    ASSERT_TRUE(origin.GetDataLength() > 0);
    origin.Reset();
    ASSERT_TRUE(origin.GetDataLength() == 0);
    ASSERT_TRUE(tmpData2.GetDataLength() > 0);
}

/**
 * @tc.name: EncodeParamTest001
 * @tc.desc: EncodeParam api interfaces test
 * @tc.type: FUNC
 * @tc.require: issueI7E737
 */
HWTEST_F(HiSysEventEncodedTest, EncodeParamTest001, TestSize.Level1)
{
    uint64_t val = 39912344; // a random numeber
    std::shared_ptr<EncodedParam> param = std::make_shared<UnsignedVarintEncodedParam<uint64_t>>("KEY", val);
    RawData data = param->GetRawData();
    ASSERT_TRUE((data.GetData() != nullptr) && (data.GetDataLength() > 0));
    uint64_t destUint64Val;
    ASSERT_TRUE(param->AsUint64(destUint64Val));
    ASSERT_TRUE(destUint64Val == val);
    int64_t destInt64Val;
    ASSERT_TRUE(!param->AsInt64(destInt64Val));
    std::string destStrVal;
    ASSERT_TRUE(param->AsString(destStrVal));
    ASSERT_TRUE(destStrVal == std::to_string(val));
    double destDoubleVal;
    ASSERT_TRUE(!param->AsDouble(destDoubleVal));
    std::vector<uint64_t> destUintArrayVal;
    ASSERT_TRUE(!param->AsUint64Vec(destUintArrayVal));
    std::vector<int64_t> destIntArrayVal;
    ASSERT_TRUE(!param->AsInt64Vec(destIntArrayVal));
    std::vector<double> destDoubleArrayVal;
    ASSERT_TRUE(!param->AsDoubleVec(destDoubleArrayVal));
    std::vector<std::string> destStrArrayVal;
    ASSERT_TRUE(!param->AsStringVec(destStrArrayVal));
}

/**
 * @tc.name: RawDataBuilderTest001
 * @tc.desc: RawDataBuilder api interfaces test
 * @tc.type: FUNC
 * @tc.require: issueI7E737
 */
HWTEST_F(HiSysEventEncodedTest, RawDataBuilderTest001, TestSize.Level1)
{
    auto dataBuilder = std::make_shared<RawDataBuilder>();
    ASSERT_TRUE(dataBuilder != nullptr);
    dataBuilder->AppendDomain("DOMAIN");
    dataBuilder->AppendName("NAME");
    dataBuilder->AppendType(HiSysEvent::EventType::SECURITY);
    dataBuilder->AppendTimeStamp(1502603794820); // a random timestamp
    dataBuilder->AppendTimeZone(3); // 3 is a valid index
    dataBuilder->AppendUid(100); // 100 is a random uid
    dataBuilder->AppendPid(1000); // 1000 is a random pid
    dataBuilder->AppendTid(384); // 384 is a random tid
    uint64_t id = std::numeric_limits<uint64_t>::max();
    dataBuilder->AppendId(id);
    dataBuilder->AppendId(std::to_string(id));
    uint64_t traceId = std::numeric_limits<uint64_t>::max();
    dataBuilder->AppendTraceId(traceId);
    dataBuilder->AppendSpanId(0); // default span id is 0
    dataBuilder->AppendPSpanId(0); // default parent span id is 0
    dataBuilder->AppendTraceFlag(HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN);
    dataBuilder->AppendValue(nullptr);
    uint64_t val = 39912344; // a random numeber
    std::shared_ptr<EncodedParam> param = std::make_shared<UnsignedVarintEncodedParam<uint64_t>>("KEY", val);
    dataBuilder->AppendValue(param);
    dataBuilder->Build();
    std::shared_ptr<EncodedParam> param1 = dataBuilder->GetValue("KEY1");
    ASSERT_TRUE(param1 == nullptr);
    std::shared_ptr<EncodedParam> param2 = dataBuilder->GetValue("KEY");
    uint64_t val2 = 0;
    ASSERT_TRUE(param2->AsUint64(val2));
    ASSERT_TRUE(val == val2);
    ASSERT_TRUE(dataBuilder->GetEventType() == HiSysEvent::EventType::SECURITY);
    auto header = dataBuilder->GetHeader();
    ASSERT_TRUE((header.type + 1) == HiSysEvent::EventType::SECURITY);
    auto traceInfo = dataBuilder->GetTraceInfo();
    ASSERT_TRUE(traceInfo.traceFlag == (HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN));
}

/**
 * @tc.name: RawDatabaseDefTest001
 * @tc.desc: Some api interfaces of raw data base definition test
 * @tc.type: FUNC
 * @tc.require: issueI7E737
 */
HWTEST_F(HiSysEventEncodedTest, RawDatabaseDefTest001, TestSize.Level1)
{
    auto timeZone = ParseTimeZone(40); // 40 is a invalid index
    ASSERT_TRUE(timeZone == "+0000");
    timeZone = ParseTimeZone(15); // 15 is a valid index
    ASSERT_TRUE(timeZone == "+0100");
}
