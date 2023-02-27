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

#ifndef HISYSEVENT_RECORD_CONVERTOR_H
#define HISYSEVENT_RECORD_CONVERTOR_H

#include "hisysevent_record.h"
#include "hisysevent_record_c.h"

namespace OHOS {
namespace HiviewDFX {
using HiSysEventRecordCls = OHOS::HiviewDFX::HiSysEventRecord;

class HiSysEventRecordConvertor {
public:
    static void InitRecord(HiSysEventRecordC& record);
    static void DeleteRecord(HiSysEventRecordC& record);
    static void DeleteRecords(HiSysEventRecordC** records, size_t len);
    static int ConvertRecord(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct);

private:
    static int ConvertDomain(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct);
    static int ConvertEventName(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct);
    static int ConvertTimeZone(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct);
    static int ConvertLevel(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct);
    static int ConvertTag(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct);
    static int ConvertJsonStr(const HiSysEventRecordCls& recordObj, HiSysEventRecordC& recordStruct);
};

}
}

#endif // HISYSEVENT_RECORD_CONVERTOR_H