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

#include "hisysevent_easy_test.h"

#include "easy_def.h"
#include "easy_event_builder.h"
#include "easy_event_encoder.h"
#include "easy_socket_writer.h"
#include "easy_util.h"
#include "hisysevent_easy.h"

using namespace testing::ext;

void HiSysEventEasyTest::SetUp()
{}

void HiSysEventEasyTest::TearDown()
{}

/**
 * @tc.name: HiSysEventEasyTest001
 * @tc.desc: Test writing event with valid content.
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest001, TestSize.Level3)
{
    int ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", "POWER_KEY", EASY_EVENT_TYPE_FAULT, "TEST_DATA");
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: HiSysEventEasyTest002
 * @tc.desc: Test writing event with invalid domain.
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest002, TestSize.Level3)
{
    // write null domain
    int ret = OH_HiSysEvent_Easy_Write(nullptr, "POWER_KEY", EASY_EVENT_TYPE_FAULT, "TEST_DATA");
    ASSERT_EQ(ret, ERR_DOMAIN_INVALID);
    // length of domain is too long
    ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDORKERNEL_VENDOR", "POWER_KEY", EASY_EVENT_TYPE_FAULT, "TEST_DATA");
    ASSERT_EQ(ret, ERR_DOMAIN_INVALID);
}

/**
 * @tc.name: HiSysEventEasyTest003
 * @tc.desc: Test writing event with different event type.
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest003, TestSize.Level3)
{
    // event type which value is lower than 1
    enum HiSysEventEasyType invalidType = (enum HiSysEventEasyType)0;
    int ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", "POWER_KEY", invalidType, "TEST_DATA"); // 0 is invalid type
    ASSERT_EQ(ret, ERR_TYPE_INVALID);
    // event type which value is greater than 4
    invalidType = (enum HiSysEventEasyType)5;
    ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", "POWER_KEY", invalidType, "TEST_DATA");
    ASSERT_EQ(ret, ERR_TYPE_INVALID);
    // write event with fault type
    ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", "POWER_KEY", EASY_EVENT_TYPE_FAULT, "TEST_DATA");
    ASSERT_EQ(ret, SUCCESS);
    // write event with statistic type
    ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", "POWER_KEY", EASY_EVENT_TYPE_STATISTIC, "TEST_DATA");
    ASSERT_EQ(ret, SUCCESS);
    // write event with security type
    ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", "POWER_KEY", EASY_EVENT_TYPE_SECURITY, "TEST_DATA");
    ASSERT_EQ(ret, SUCCESS);
    // write event with behavior type
    ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", "POWER_KEY", EASY_EVENT_TYPE_BEHAVIOR, "TEST_DATA");
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: HiSysEventEasyTest004
 * @tc.desc: Test writing event with invalid name.
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest004, TestSize.Level3)
{
    // write null name
    int ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", nullptr, EASY_EVENT_TYPE_FAULT, "TEST_DATA");
    ASSERT_EQ(ret, ERR_NAME_INVALID);
    // length of name is too long
    ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", "POWER_KEYPOWER_KEYPOWER_KEYPOWER_KEY", EASY_EVENT_TYPE_FAULT,
        "TEST_DATA");
    ASSERT_EQ(ret, ERR_NAME_INVALID);
}

/**
 * @tc.name: HiSysEventEasyTest005
 * @tc.desc: Test writing event with invalid customized string.
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest005, TestSize.Level3)
{
    // write a string with 1025 bytes
    std::string strContent;
    for (int i = 0; i < 102; i++) {
        strContent = strContent + std::string("0123456789");
    }
    strContent = strContent + "01234";
    int ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", "POWER_KEY", EASY_EVENT_TYPE_FAULT, strContent.c_str());
    ASSERT_EQ(ret, ERR_PARAM_VALUE_INVALID);
    // write null string
    ret = OH_HiSysEvent_Easy_Write("KERNEL_VENDOR", "POWER_KEY", EASY_EVENT_TYPE_FAULT, nullptr);
    ASSERT_EQ(ret, ERR_PARAM_VALUE_INVALID);
}

/**
 * @tc.name: HiSysEventEasyTest006
 * @tc.desc: Test AppendHeader
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest006, TestSize.Level3)
{
    size_t offset = 0;
    struct HiSysEventEasyHeader header;
    int ret = MemoryInit((uint8_t*)(&header), sizeof(struct HiSysEventEasyHeader));
    ASSERT_EQ(ret, SUCCESS);
    ret = AppendHeader(nullptr, EVENT_BUFF_LEN, &offset, &header);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    const int eventBufLen = 100;
    uint8_t testEventBuff[eventBufLen];
    ret = AppendHeader(testEventBuff, EVENT_BUFF_LEN, nullptr, &header);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    ret = AppendHeader(testEventBuff, EVENT_BUFF_LEN, &offset, nullptr);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    offset = EVENT_BUFF_LEN - 2; // 2 is a test value
    ret = AppendHeader(testEventBuff, EVENT_BUFF_LEN, &offset, &header);
    ASSERT_EQ(ret, ERR_MEM_OPT_FAILED);
    offset = 0;
    ret = AppendHeader(testEventBuff, EVENT_BUFF_LEN, &offset, &header);
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: HiSysEventEasyTest007
 * @tc.desc: Test AppendStringParam
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest007, TestSize.Level3)
{
    size_t offset = 0;
    int ret = AppendStringParam(nullptr, EVENT_BUFF_LEN, &offset, nullptr, nullptr);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    uint8_t testEventBuff[EVENT_BUFF_LEN];
    ret = AppendStringParam(testEventBuff, EVENT_BUFF_LEN, nullptr, nullptr, nullptr);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    ret = AppendStringParam(testEventBuff, EVENT_BUFF_LEN, &offset, nullptr, nullptr);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    ret = AppendStringParam(testEventBuff, EVENT_BUFF_LEN, &offset, "TEST_KEY", nullptr);
    ASSERT_EQ(ret, ERR_PARAM_VALUE_INVALID);
    offset = EVENT_BUFF_LEN;
    ret = AppendStringParam(testEventBuff, EVENT_BUFF_LEN, &offset, "TEST_KEY", "TEST_VAL");
    ASSERT_EQ(ret, ERR_ENCODE_STR_FAILED);
    offset = EVENT_BUFF_LEN - 9; // 9 is a test value
    ret = AppendStringParam(testEventBuff, EVENT_BUFF_LEN, &offset, "TEST_KEY", "TEST_VAL");
    ASSERT_EQ(ret, ERR_ENCODE_VALUE_TYPE_FAILED);
    offset = 0;
    ret = AppendStringParam(testEventBuff, EVENT_BUFF_LEN, &offset, "TEST_KEY", "TEST_VAL");
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: HiSysEventEasyTest008
 * @tc.desc: Test EncodeValueType
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest008, TestSize.Level3)
{
    size_t offset = 0;
    struct HiSysEventEasyParamValueType valueType;
    valueType.isArray = 0;
    valueType.valueType = 0;
    valueType.valueByteCnt = 0;
    int ret = EncodeValueType(nullptr, EVENT_BUFF_LEN, &offset, &valueType);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    uint8_t data[EVENT_BUFF_LEN];
    ret = EncodeValueType(data, EVENT_BUFF_LEN, nullptr, &valueType);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    ret = EncodeValueType(data, EVENT_BUFF_LEN, &offset, nullptr);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    offset = EVENT_BUFF_LEN; // 2 is a test value
    ret = EncodeValueType(data, EVENT_BUFF_LEN, &offset, &valueType);
    ASSERT_EQ(ret, ERR_ENCODE_VALUE_TYPE_FAILED);
    offset = 0;
    ret = EncodeValueType(data, EVENT_BUFF_LEN, &offset, &valueType);
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: HiSysEventEasyTest009
 * @tc.desc: Test EncodeStringValue
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest009, TestSize.Level3)
{
    size_t offset = 0;
    int ret = EncodeStringValue(nullptr, EVENT_BUFF_LEN, &offset, nullptr);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    uint8_t data[EVENT_BUFF_LEN];
    ret = EncodeStringValue(data, EVENT_BUFF_LEN, nullptr, nullptr);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    ret = EncodeStringValue(data, EVENT_BUFF_LEN, &offset, nullptr);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    std::string val = "TEST";
    offset = EVENT_BUFF_LEN - val.size() + 1; // 1 is a test value
    ret = EncodeStringValue(data, EVENT_BUFF_LEN, &offset, val.c_str());
    ASSERT_EQ(ret, ERR_ENCODE_STR_FAILED);
    offset = EVENT_BUFF_LEN - val.size() + 3; // 3 is a test offset value
    ret = EncodeStringValue(data, EVENT_BUFF_LEN, &offset, val.c_str());
    ASSERT_EQ(ret, ERR_ENCODE_STR_FAILED);
    offset = EVENT_BUFF_LEN - 30; // 30 is a test offset value
    ret = EncodeStringValue(data, EVENT_BUFF_LEN, &offset, val.c_str());
    ASSERT_EQ(ret, SUCCESS);
    offset = 0;
    ret = EncodeStringValue(data, EVENT_BUFF_LEN, &offset, val.c_str());
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: HiSysEventEasyTest010
 * @tc.desc: Test Write
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest010, TestSize.Level3)
{
    int ret = Write(nullptr, 0);
    ASSERT_EQ(ret, ERR_EVENT_BUF_INVALID);
    uint8_t data[EVENT_BUFF_LEN];
    ret = Write(data, EVENT_BUFF_LEN);
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: HiSysEventEasyTest011
 * @tc.desc: Test MemoryInit
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest011, TestSize.Level3)
{
    int ret = MemoryInit(nullptr, 0);
    ASSERT_EQ(ret, ERR_MEM_OPT_FAILED);
    ret = MemoryInit(nullptr, EVENT_BUFF_LEN);
    ASSERT_EQ(ret, ERR_MEM_OPT_FAILED);
    uint8_t data[EVENT_BUFF_LEN];
    ret = MemoryInit(data, EVENT_BUFF_LEN);
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: HiSysEventEasyTest012
 * @tc.desc: Test MemoryCopy
 * @tc.type: FUNC
 * @tc.require: issueIAKQGU
 */
HWTEST_F(HiSysEventEasyTest, HiSysEventEasyTest012, TestSize.Level3)
{
    uint8_t data[EVENT_BUFF_LEN];
    int ret = MemoryCopy(nullptr, 0, data, 0);
    ASSERT_EQ(ret, ERR_MEM_OPT_FAILED);
    ret = MemoryCopy(data, 0, nullptr, 0);
    ASSERT_EQ(ret, ERR_MEM_OPT_FAILED);
    uint8_t dataNew[EVENT_BUFF_LEN];
    ret = MemoryCopy(data, EVENT_BUFF_LEN - 1, dataNew, EVENT_BUFF_LEN); // 1 is a test value
    ASSERT_EQ(ret, ERR_MEM_OPT_FAILED);
    ret = MemoryCopy(data, EVENT_BUFF_LEN, dataNew, EVENT_BUFF_LEN);
    ASSERT_EQ(ret, SUCCESS);
}