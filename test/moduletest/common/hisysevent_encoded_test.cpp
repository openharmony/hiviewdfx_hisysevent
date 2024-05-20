/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "hisysevent.h"
#include "raw_data_base_def.h"
#include "raw_data_encoder.h"
#include "raw_data.h"
#include "transport.h"

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
    ASSERT_EQ(data, nullptr);
    ASSERT_TRUE(!param->Encode());
    auto rawData = std::make_shared<Encoded::RawData>();
    param->SetRawData(rawData);
    ASSERT_TRUE(param->Encode());
    data = param->GetRawData();
    ASSERT_NE(data, nullptr);
    ASSERT_NE(data->GetData(), nullptr);
    ASSERT_GT(data->GetDataLength(), 0);
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
    ASSERT_EQ(tzIndex, 0); // reference to ALL_TIME_ZONES defined in raw_data_base_def.cpp
    tzIndex = ParseTimeZone(15); // 15 is an invalid timezone value
    ASSERT_EQ(tzIndex, 14); // default index
}

/**
 * @tc.name: RawDataTest001
 * @tc.desc: Construction and destruction of RawData
 * @tc.type: FUNC
 * @tc.require: issueI8YWH1
 */
HWTEST_F(HiSysEventEncodedTest, RawDataTest001, TestSize.Level1)
{
    Encoded::RawData rawData1(nullptr, 0);
    ASSERT_TRUE(rawData1.IsEmpty());
    Encoded::RawData rawData2 = rawData1;
    ASSERT_TRUE(rawData2.IsEmpty());
    uint64_t val = 2323232; // 2323232 is a random test numeber
    std::shared_ptr<EncodedParam> param = std::make_shared<UnsignedVarintEncodedParam<uint64_t>>("KEY1", val);
    auto rawData3 = std::make_shared<Encoded::RawData>(nullptr, 0);
    param->SetRawData(rawData3);
    auto appendRawData = param->GetRawData();
    rawData1 = *appendRawData;
    ASSERT_TRUE(!rawData1.IsEmpty());
    Encoded::RawData rawData4 = rawData1;
    ASSERT_EQ(rawData1.GetDataLength(), rawData4.GetDataLength());
    Encoded::RawData rawData5(appendRawData->GetData(), appendRawData->GetDataLength());
    ASSERT_EQ(appendRawData->GetDataLength(), rawData5.GetDataLength());
}

/**
 * @tc.name: TransportTest001
 * @tc.desc: Send raw data
 * @tc.type: FUNC
 * @tc.require: issueI8YWH1
 */
HWTEST_F(HiSysEventEncodedTest, TransportTest001, TestSize.Level1)
{
    auto rawData1 = std::make_shared<Encoded::RawData>();
    ASSERT_TRUE(rawData1->IsEmpty());
    ASSERT_EQ(Transport::GetInstance().SendData(*rawData1), ERR_EMPTY_EVENT);
    uint64_t val = 2323232; // 2323232 is a random test numeber
    std::shared_ptr<EncodedParam> param = std::make_shared<UnsignedVarintEncodedParam<uint64_t>>("KEY1", val);
    ASSERT_NE(param, nullptr);
    param->SetRawData(rawData1);
    param->Encode();
    auto rawData2 = param->GetRawData();
    ASSERT_TRUE(!rawData2->IsEmpty());
    ASSERT_EQ(Transport::GetInstance().SendData(*rawData2), SUCCESS);
}
