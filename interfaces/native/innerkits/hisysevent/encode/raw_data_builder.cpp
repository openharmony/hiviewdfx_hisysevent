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

#include "raw_data_builder.h"

#include <securec.h>
#include <vector>

#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace Encode {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HiSysEvent-RawDataBuilder" };
constexpr int DEFAULT_TZ_POS = 14; // default "+0000"

static int ParseTimeZone(const std::string& formatted)
{
    int ret = DEFAULT_TZ_POS;
    std::vector<std::string> allTimeZones {
        "-0100", "-0200", "-0300", "-0330", "-0400", "-0500", "-0600",
        "-0700", "-0800", "-0900", "-0930", "-1000", "-1100", "-1200",
        "+0000", "+0100", "+0200", "+0300", "+0330", "+0400", "+0430",
        "+0500", "+0530", "+0545", "+0600", "+0630", "+0700", "+0800",
        "+0845", "+0900", "+0930", "+1000", "+1030", "+1100", "+1200",
        "+1245", "+1300", "+1400"
    };
    for (auto iter = allTimeZones.begin(); iter < allTimeZones.end(); ++iter) {
        if (*iter == formatted) {
            ret = (iter - allTimeZones.begin()) + 1; // item value ascends from 1.
            break;
        }
    }
    return ret;
}
}

RawDataBuilder::RawDataBuilder(const std::string& domain, const std::string& name, const int eventType)
{
    (void)AppendDomain(domain);
    (void)AppendName(name);
    (void)AppendType(eventType);
    HiLog::Debug(LABEL, "Encoded type is %{public}d for event[%{public}s:%{public}s].", header_.type,
        header_.domain, header_.name);
}

bool RawDataBuilder::BuildHeader()
{
    if (!rawData_.Append(reinterpret_cast<uint8_t*>(&header_), sizeof(struct HiSysEventHeader))) {
        HiLog::Error(LABEL, "Event header copy failed.");
        return false;
    }
    // append trace info
    if (header_.isTraceOpened == 1 &&
        !rawData_.Append(reinterpret_cast<uint8_t*>(&traceInfo_), sizeof(struct TraceInfo))) {
        HiLog::Error(LABEL, "Trace info copy failed.");
        return false;
    }
    return true;
}

bool RawDataBuilder::BuildCustomizedParams()
{
    for (auto param : allParams_) {
        auto rawData = param->GetRawData();
        if (!rawData_.Append(rawData.GetData(), rawData.GetDataLength())) {
            return false;
        }
    }
    return true;
}

RawData& RawDataBuilder::Build()
{
    // placehold block size
    int32_t blockSize = 0;
    rawData_.Reset();
    if (!rawData_.Append(reinterpret_cast<uint8_t*>(&blockSize), sizeof(int32_t))) {
        HiLog::Error(LABEL, "Block size copy failed.");
        return rawData_;
    }
    if (!BuildHeader()) {
        HiLog::Error(LABEL, "Header of sysevent build failed.");
        return rawData_;
    }
    // append parameter count
    int32_t paramCnt = static_cast<int32_t>(allParams_.size());
    if (!rawData_.Append(reinterpret_cast<uint8_t*>(&paramCnt), sizeof(int32_t))) {
        HiLog::Error(LABEL, "Parameter count copy failed.");
        return rawData_;
    }
    if (!BuildCustomizedParams()) {
        HiLog::Error(LABEL, "Customized paramters of sys event build failed.");
        return rawData_;
    }
    // update block size
    blockSize = static_cast<int32_t>(rawData_.GetDataLength());
    if (!rawData_.Update(reinterpret_cast<uint8_t*>(&blockSize), sizeof(int32_t), 0)) {
        HiLog::Error(LABEL, "Failed to update block size.");
    }
    return rawData_;
}

RawDataBuilder& RawDataBuilder::AppendDomain(const std::string& domain)
{
    auto ret = memcpy_s(header_.domain, MAX_DOMAIN_LENGTH, domain.c_str(), domain.length());
    if (ret != EOK) {
        HiLog::Error(LABEL, "Failed to copy event domain, ret is %{public}d.", ret);
    }
    auto resetPos = std::min(domain.length(), static_cast<size_t>(MAX_DOMAIN_LENGTH));
    header_.domain[resetPos] = '\0';
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendName(const std::string& name)
{
    auto ret = memcpy_s(header_.name, MAX_EVENT_NAME_LENGTH, name.c_str(), name.length());
    if (ret != EOK) {
        HiLog::Error(LABEL, "Failed to copy event name, ret is %{public}d.", ret);
    }
    auto resetPos = std::min(name.length(), static_cast<size_t>(MAX_EVENT_NAME_LENGTH));
    header_.name[resetPos] = '\0';
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendType(const int eventType)
{
    header_.type = static_cast<uint8_t>(eventType - 1); // header_.type is only 2 bits which must be
                                                    // subtracted 1 in order to avoid data overrflow.
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTimeStamp(const uint64_t timestamp)
{
    header_.timestamp = timestamp;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTimeZone(const std::string& timeZone)
{
    header_.timeZone = static_cast<uint8_t>(ParseTimeZone(timeZone));
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendUid(const uint32_t uid)
{
    header_.uid = uid;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendPid(const uint32_t pid)
{
    header_.pid = pid;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTid(const uint32_t tid)
{
    header_.tid = tid;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendId(const uint64_t id)
{
    header_.id = id;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTraceInfo(const uint64_t traceId, const uint32_t spanId,
    const uint32_t pSpanId, const uint8_t traceFlag)
{
    header_.isTraceOpened = 1; // 1: include trace info, 0: exclude trace info.

    traceInfo_.traceId = traceId;
    traceInfo_.spanId = spanId;
    traceInfo_.pSpanId = pSpanId;
    traceInfo_.traceFlag = traceFlag;

    return *this;
}

std::string RawDataBuilder::GetDomain()
{
    return std::string(header_.domain);
}

std::string RawDataBuilder::GetName()
{
    return std::string(header_.name);
}

size_t RawDataBuilder::GetParamCnt()
{
    return allParams_.size();
}
} // namespace Encode
} // namespace HiviewDFX
} // namespace OHOS