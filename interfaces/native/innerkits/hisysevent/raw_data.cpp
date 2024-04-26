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

#include "raw_data.h"

#include <new>

#include "hilog/log.h"
#include "securec.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_RAW_DATA"

namespace OHOS {
namespace HiviewDFX {
namespace Encoded {
namespace {
constexpr size_t EXPAND_BUF_SIZE = 128;
}

RawData::RawData()
{
    data_ = new(std::nothrow) uint8_t[EXPAND_BUF_SIZE];
    if (data_ == nullptr) {
        return;
    }
    capacity_ = EXPAND_BUF_SIZE;
    len_ = 0;
}

RawData::RawData(uint8_t* data, size_t dataLen)
{
    if (data == nullptr || dataLen == 0) {
        data_ = new(std::nothrow) uint8_t[EXPAND_BUF_SIZE];
        if (data_ == nullptr) {
            return;
        }
        capacity_ = EXPAND_BUF_SIZE;
        len_ = 0;
        return;
    }
    data_ = new(std::nothrow) uint8_t[dataLen];
    if (data_ == nullptr) {
        return;
    }
    auto ret = memcpy_s(data_, dataLen, data, dataLen);
    if (ret != EOK) {
        HILOG_ERROR(LOG_CORE, "Failed to copy RawData in constructor, ret is %{public}d.", ret);
        return;
    }
    capacity_ = dataLen;
    len_ = dataLen;
}

RawData::RawData(const RawData& data)
{
    auto dataLen = data.GetDataLength();
    auto rawData = data.GetData();
    if (dataLen == 0 || rawData == nullptr) {
        data_ = new(std::nothrow) uint8_t[EXPAND_BUF_SIZE];
        if (data_ == nullptr) {
            return;
        }
        capacity_ = EXPAND_BUF_SIZE;
        len_ = 0;
        return;
    }
    data_ = new(std::nothrow) uint8_t[dataLen];
    if (data_ == nullptr) {
        return;
    }
    auto ret = memcpy_s(data_, dataLen, rawData, dataLen);
    if (ret != EOK) {
        HILOG_ERROR(LOG_CORE, "Failed to copy RawData in constructor, ret is %{public}d.", ret);
        return;
    }
    capacity_ = dataLen;
    len_ = dataLen;
}

RawData& RawData::operator=(const RawData& data)
{
    if (this == &data) {
        return *this;
    }
    auto dataLen = data.GetDataLength();
    auto rawData = data.GetData();
    if (dataLen == 0 || rawData == nullptr) {
        return *this;
    }
    uint8_t* tmpData = new(std::nothrow) uint8_t[dataLen];
    if (tmpData == nullptr) {
        return *this;
    }
    auto ret = memcpy_s(tmpData, dataLen, rawData, dataLen);
    if (ret != EOK) {
        HILOG_ERROR(LOG_CORE, "Failed to copy RawData in constructor, ret is %{public}d.", ret);
        delete[] tmpData;
        return *this;
    }
    if (data_ != nullptr) {
        delete[] data_;
    }
    data_ = tmpData;
    capacity_ = dataLen;
    len_ = dataLen;
    return *this;
}

RawData::~RawData()
{
    if (data_ != nullptr) {
        delete[] data_;
        data_ = nullptr;
    }
}

bool RawData::Append(uint8_t* data, size_t len)
{
    if (len == 0) {
        return true;
    }
    return Update(data, len, len_);
}

bool RawData::IsEmpty()
{
    return len_ == 0 || data_ == nullptr;
}

bool RawData::Update(uint8_t* data, size_t len, size_t pos)
{
    if (data == nullptr || pos > len_) {
        HILOG_ERROR(LOG_CORE, "Invalid memory operation: length is %{public}zu, pos is %{public}zu, "
            "len_ is %{public}zu", len, pos, len_);
        return false;
    }
    auto ret = EOK;
    if ((pos + len) > capacity_) {
        size_t expandedSize = (len > EXPAND_BUF_SIZE) ? len : EXPAND_BUF_SIZE;
        uint8_t* resizedData = new(std::nothrow) uint8_t[capacity_ + expandedSize];
        if (resizedData == nullptr) {
            return false;
        }
        ret = memcpy_s(resizedData, len_, data_, len_);
        if (ret != EOK) {
            HILOG_ERROR(LOG_CORE, "Failed to expand capacity of raw data, ret is %{public}d.", ret);
            delete[] resizedData;
            return false;
        }
        capacity_ += expandedSize;
        if (data_ != nullptr) {
            delete[] data_;
        }
        data_ = resizedData;
    }
    // append new data
    ret = memcpy_s(data_ + pos, len, data, len);
    if (ret != EOK) {
        HILOG_ERROR(LOG_CORE, "Failed to append new data, ret is %{public}d.", ret);
        return false;
    }
    if ((pos + len) > len_) {
        len_ = pos + len;
    }
    return true;
}

uint8_t* RawData::GetData() const
{
    return data_;
}

size_t RawData::GetDataLength() const
{
    return len_;
}
} // namespace Encoded
} // namespace HiviewDFX
} // namespace OHOS