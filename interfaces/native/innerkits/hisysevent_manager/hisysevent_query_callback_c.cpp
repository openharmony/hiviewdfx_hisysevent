/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "hisysevent_query_callback_c.h"

#include "hilog/log.h"
#include "hisysevent_record_convertor.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENT_C_QUERY" };
using HiSysEventRecordCls = OHOS::HiviewDFX::HiSysEventRecord;
using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiSysEventRecordConvertor;
}

void HiSysEventQueryCallbackC::OnQuery(std::shared_ptr<std::vector<OHOS::HiviewDFX::HiSysEventRecord>> sysEvents)
{
    if (onQuery_ == nullptr) {
        HiLog::Error(LABEL, "OnQuery callback is null");
        return;
    }
    if (sysEvents == nullptr || sysEvents->empty()) {
        onQuery_(nullptr, 0);
        return;
    }
    size_t size = sysEvents->size();
    auto records = new(std::nothrow) HiSysEventRecordC[size];
    for (size_t i = 0; i < size; i++) {
        HiSysEventRecordConvertor::InitRecord(records[i]);
        if (HiSysEventRecordConvertor::ConvertRecord(sysEvents->at(i), records[i]) != 0) {
            HiLog::Error(LABEL, "Failed to covert record, index=%{public}zu, size=%{public}zu",  i, size);
            HiSysEventRecordConvertor::DeleteRecords(&records, i + 1); // +1 for release the current record
            return;
        }
    }
    onQuery_(records, size);
    HiSysEventRecordConvertor::DeleteRecords(&records, size);
}

void HiSysEventQueryCallbackC::OnComplete(int32_t reason, int32_t total)
{
    if (onComplete_ == nullptr) {
        HiLog::Error(LABEL, "OnComplete callback is null");
        return;
    }
    onComplete_(reason, total);
}
