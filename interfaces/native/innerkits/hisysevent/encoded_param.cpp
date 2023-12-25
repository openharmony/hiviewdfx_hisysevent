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

#include "encoded_param.h"

#include "hilog/log.h"
#include "raw_data_encoder.h"
#include "securec.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_ENCODED_PARAM"

namespace OHOS {
namespace HiviewDFX {
namespace Encoded {
EncodedParam::EncodedParam(const std::string& key)
{
    key_ = key;
}

EncodedParam::~EncodedParam()
{
}

std::string& EncodedParam::GetKey()
{
    return key_;
}

void EncodedParam::SetRawData(std::shared_ptr<RawData> rawData)
{
    rawData_ = rawData;
}

std::shared_ptr<RawData> EncodedParam::GetRawData()
{
    if (!hasEncoded_) {
        hasEncoded_ = Encode();
    }
    return rawData_;
}

bool EncodedParam::Encode()
{
    hasEncoded_ = EncodeKey() && EncodeValueType() && EncodeValue();
    return hasEncoded_;
}

bool EncodedParam::EncodeKey()
{
    if (rawData_ == nullptr) {
        return false;
    }
    if (!RawDataEncoder::StringValueEncoded(*rawData_, key_)) {
        HILOG_ERROR(LOG_CORE, "The key of customized value encoded failded.");
        return false;
    }
    return true;
}
} // namespace Encoded
} // namespace HiviewDFX
} // namespace OHOS