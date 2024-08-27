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

#include "hisysevent_record_convertor.h"

#include "hilog/log.h"
#include "hisysevent_record_c.h"
#include "string_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_RECORD_CONVERTOR"

namespace OHOS {
namespace HiviewDFX {
int HiSysEventRecordConvertor::ConvertDomain(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct)
{
    return OHOS::HiviewDFX::StringUtil::CopyCString(recordStruct.domain, recordObj.GetDomain(),
        MAX_LENGTH_OF_EVENT_DOMAIN - 1);
}

int HiSysEventRecordConvertor::ConvertEventName(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct)
{
    return OHOS::HiviewDFX::StringUtil::CopyCString(recordStruct.eventName, recordObj.GetEventName(),
        MAX_LENGTH_OF_EVENT_NAME - 1);
}

int HiSysEventRecordConvertor::ConvertTimeZone(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct)
{
    return OHOS::HiviewDFX::StringUtil::CopyCString(recordStruct.tz, recordObj.GetTimeZone(),
        MAX_LENGTH_OF_TIME_ZONE - 1);
}

int HiSysEventRecordConvertor::ConvertLevel(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct)
{
    return OHOS::HiviewDFX::StringUtil::CreateCString(&recordStruct.level, recordObj.GetLevel());
}

int HiSysEventRecordConvertor::ConvertTag(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct)
{
    return OHOS::HiviewDFX::StringUtil::CreateCString(&recordStruct.tag, recordObj.GetTag());
}

int HiSysEventRecordConvertor::ConvertJsonStr(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct)
{
    constexpr size_t maxLen = 384 * 1024; // max length of the event is 384KB
    return OHOS::HiviewDFX::StringUtil::CreateCString(&recordStruct.jsonStr, recordObj.AsJson(), maxLen);
}

void HiSysEventRecordConvertor::InitRecord(HiSysEventRecordC& record)
{
    OHOS::HiviewDFX::StringUtil::MemsetSafe(&record, sizeof(HiSysEventRecordC));
}

void HiSysEventRecordConvertor::DeleteRecord(HiSysEventRecordC& record)
{
    OHOS::HiviewDFX::StringUtil::DeletePointer<char>(&record.level);
    OHOS::HiviewDFX::StringUtil::DeletePointer<char>(&record.tag);
    OHOS::HiviewDFX::StringUtil::DeletePointer<char>(&record.jsonStr);
}

void HiSysEventRecordConvertor::DeleteRecords(HiSysEventRecordC** records, size_t len)
{
    if (records == nullptr || *records == nullptr) {
        return;
    }
    auto realRs = *records;
    for (size_t i = 0; i < len; i++) {
        DeleteRecord(realRs[i]);
    }
    delete[] realRs;
    realRs = nullptr;
}

int HiSysEventRecordConvertor::ConvertRecord(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct)
{
    if (int res = ConvertDomain(recordObj, recordStruct); res != 0) {
        return res;
    }
    if (int res = ConvertEventName(recordObj, recordStruct); res != 0) {
        HILOG_ERROR(LOG_CORE, "Failed to convert name=%{public}s",  recordObj.GetEventName().c_str());
        return res;
    }
    recordStruct.type = HiSysEventEventType(recordObj.GetEventType());
    recordStruct.time = recordObj.GetTime();
    if (int res = ConvertTimeZone(recordObj, recordStruct); res != 0) {
        HILOG_ERROR(LOG_CORE, "Failed to convert tz=%{public}s",  recordObj.GetTimeZone().c_str());
        return res;
    }
    recordStruct.pid = recordObj.GetPid();
    recordStruct.tid = recordObj.GetTid();
    recordStruct.uid = recordObj.GetUid();
    recordStruct.traceId = recordObj.GetTraceId();
    recordStruct.spandId = recordObj.GetSpanId();
    recordStruct.pspanId = recordObj.GetPspanId();
    recordStruct.traceFlag = recordObj.GetTraceFlag();
    if (int res = ConvertLevel(recordObj, recordStruct); res != 0) {
        HILOG_ERROR(LOG_CORE, "Failed to convert level=%{public}s",  recordObj.GetLevel().c_str());
        return res;
    }
    if (int res = ConvertTag(recordObj, recordStruct); res != 0) {
        HILOG_ERROR(LOG_CORE, "Failed to convert tag=%{public}s",  recordObj.GetTag().c_str());
        return res;
    }
    if (int res = ConvertJsonStr(recordObj, recordStruct); res != 0) {
        return res;
    }
    return 0;
}
}
}