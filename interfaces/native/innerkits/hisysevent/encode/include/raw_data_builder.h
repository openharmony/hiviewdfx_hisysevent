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

#ifndef INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_RAW_DATA_BUILDER_H
#define INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_RAW_DATA_BUILDER_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "encoded_param.h"
#include "def.h"
#include "raw_data_base_def.h"
#include "raw_data.h"

namespace OHOS {
namespace HiviewDFX {
namespace Encode {
class RawDataBuilder {
public:
    RawDataBuilder() {};
    RawDataBuilder(const std::string& domain, const std::string& name, const int eventType);
    ~RawDataBuilder() = default;

public:
    RawData& Build();

public:
    RawDataBuilder& AppendDomain(const std::string& domain);
    RawDataBuilder& AppendName(const std::string& name);
    RawDataBuilder& AppendType(const int eventType);
    RawDataBuilder& AppendTimeStamp(const uint64_t timeStamp);
    RawDataBuilder& AppendTimeZone(const std::string& timeZone);
    RawDataBuilder& AppendUid(const uint32_t uid);
    RawDataBuilder& AppendPid(const uint32_t pid);
    RawDataBuilder& AppendTid(const uint32_t tid);
    RawDataBuilder& AppendId(const uint64_t id);
    RawDataBuilder& AppendTraceInfo(const uint64_t traceId, const uint32_t spanId,
        const uint32_t pSpanId, const uint8_t traceFlag);

public:
    RawDataBuilder& AppendValue(std::shared_ptr<EncodedParam> param)
    {
        if (param == nullptr || !param->Encode()) {
            return *this;
        }
        auto paramKey = param->GetKey();
        for (auto iter = allParams_.begin(); iter < allParams_.end(); iter++) {
            if ((*iter) == nullptr) {
                continue;
            }
            if ((*iter)->GetKey() == paramKey) {
                allParams_.erase(iter);
                break;
            }
        }
        allParams_.emplace_back(param);
        return *this;
    }

public:
    std::string GetDomain();
    std::string GetName();
    size_t GetParamCnt();

private:
    bool BuildHeader();
    bool BuildCustomizedParams();

private:
    struct HiSysEventHeader header_ = {
        .domain = {0},
        .name = {0},
        .timestamp = 0,
        .timeZone = 0,
        .uid = 0,
        .pid = 0,
        .tid = 0,
        .id = 0,
        .type = 0,
        .isTraceOpened = 0,
    };
    struct TraceInfo traceInfo_ {
        .traceFlag = 0,
        .traceId = 0,
        .spanId = 0,
        .pSpanId = 0,
    };
    std::vector<std::shared_ptr<EncodedParam>> allParams_;
    RawData rawData_;
};
} // namesapce Encode
} // namespace HiviewDFX
} // namespace OHOS

#endif // INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_RAW_DATA_BUILDER_H