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
#include "raw_data_encoder.h"
#include "raw_data.h"

#include "hisysevent.h"

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
 * @tc.name: EncodeParamTest001
 * @tc.desc: EncodeParam api interfaces test
 * @tc.type: FUNC
 * @tc.require: issueI7E737
 */
HWTEST_F(HiSysEventEncodedTest, EncodeParamTest001, TestSize.Level1)
{
    uint64_t val = 39912344; // a random numeber
    std::shared_ptr<EncodedParam> param = std::make_shared<UnsignedVarintEncodedParam<uint64_t>>("KEY", val);
    auto data = param->GetRawData();
    ASSERT_TRUE(data == nullptr);
    ASSERT_TRUE(!param->Encode());
    auto rawData = std::make_shared<Encoded::RawData>();
    param->SetRawData(rawData);
    ASSERT_TRUE(param->Encode());
    data = param->GetRawData();
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE((data->GetData() != nullptr) && (data->GetDataLength() > 0));
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
 * @tc.name: RawDatabaseDefTest001
 * @tc.desc: Some api interfaces of raw data base definition test
 * @tc.type: FUNC
 * @tc.require: issueI7E737
 */
HWTEST_F(HiSysEventEncodedTest, RawDatabaseDefTest001, TestSize.Level1)
{
    auto tzIndex = ParseTimeZone(3600); // 3600 is a valid timezone value
    ASSERT_TRUE(tzIndex == 0); // reference to ALL_TIME_ZONES defined in raw_data_base_def.cpp
    tzIndex = ParseTimeZone(15); // 15 is an invalid timezone value
    ASSERT_TRUE(tzIndex == 14); // default index
}
