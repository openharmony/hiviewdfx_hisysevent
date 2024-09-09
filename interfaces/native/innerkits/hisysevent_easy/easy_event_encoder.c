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

#include "easy_event_encoder.h"

#include <string.h>

#include "easy_def.h"
#include "easy_util.h"

#ifdef __cplusplus
extern "C" {
#endif

static const unsigned int TAG_BYTE_OFFSET = 5;
static const unsigned int TAG_BYTE_BOUND = (1 << TAG_BYTE_OFFSET);
static const unsigned int TAG_BYTE_MASK = (TAG_BYTE_BOUND - 1);

static const unsigned int NON_TAG_BYTE_OFFSET = 7;
static const unsigned int NON_TAG_BYTE_BOUND = (1 << NON_TAG_BYTE_OFFSET);
static const unsigned int NON_TAG_BYTE_MASK = (NON_TAG_BYTE_BOUND - 1);

static uint8_t LENGTH_DELIMITED_ENCODE_TYPE = 1;

static const int VAR_INT_ENCODE_SUCCESS = 0;
static const int VAR_INT_ENCODE_FAIL = 1;

static uint8_t EncodeTag(uint8_t encodeType)
{
    return (encodeType << (TAG_BYTE_OFFSET + 1));
}

static int EncodeUnsignedVarint(uint8_t* data, const size_t dataLen, size_t* offset, uint8_t encodeType,
    uint64_t u64Val)
{
    uint8_t cpyVal = EncodeTag(encodeType) | ((u64Val < TAG_BYTE_BOUND) ? 0 : TAG_BYTE_BOUND) |
        (uint8_t)(u64Val & TAG_BYTE_MASK);
    if ((dataLen < *offset) || ((dataLen - *offset) < sizeof(uint8_t))) {
        return VAR_INT_ENCODE_FAIL;
    }
    *(data + *offset) = cpyVal;
    *offset += sizeof(uint8_t);
    u64Val >>= TAG_BYTE_OFFSET;
    while (u64Val > 0) {
        cpyVal = ((u64Val < NON_TAG_BYTE_BOUND) ? 0 : NON_TAG_BYTE_BOUND) | (uint8_t)(u64Val & NON_TAG_BYTE_MASK);
        if ((dataLen < *offset) || ((dataLen - *offset) < sizeof(uint8_t))) {
            return VAR_INT_ENCODE_FAIL;
        }
        *(data + *offset) = cpyVal;
        *offset += sizeof(uint8_t);
        u64Val >>= NON_TAG_BYTE_OFFSET;
    }
    return VAR_INT_ENCODE_SUCCESS;
}

int EncodeValueType(uint8_t* data, const size_t dataLen, size_t* offset, struct HiSysEventParamValueType* valueType)
{
    if (data == NULL || offset == NULL) {
        return ERR_EVENT_BUF_INVALID;
    }
    if (valueType == NULL) {
        return ERR_MEM_OPT_FAILED;
    }
    if ((dataLen < *offset) || ((dataLen - *offset) < sizeof(struct HiSysEventParamValueType))) {
        return ERR_ENCODE_VALUE_TYPE_FAILED;
    }
    *((struct HiSysEventParamValueType*)(data + *offset)) = *valueType;
    *offset += sizeof(struct HiSysEventParamValueType);
    return SUCCESS;
}

int EncodeStringValue(uint8_t* data, const size_t dataLen, size_t* offset, const char* content)
{
    if (data == NULL || offset == NULL) {
        return ERR_ENCODE_STR_FAILED;
    }
    size_t contentLen = strlen(content);
    if (EncodeUnsignedVarint(data, dataLen, offset, LENGTH_DELIMITED_ENCODE_TYPE, contentLen) !=
        VAR_INT_ENCODE_SUCCESS) {
        return ERR_ENCODE_STR_FAILED;
    }
    if ((dataLen < *offset) || ((dataLen - *offset) < contentLen)) {
        return ERR_ENCODE_STR_FAILED;
    }
    int cpyRet = MemoryCpy(data + *offset, (uint8_t*)content, contentLen);
    if (cpyRet != SUCCESS) {
        return cpyRet;
    }
    *offset += contentLen;
    return SUCCESS;
}

#ifdef __cplusplus
}
#endif