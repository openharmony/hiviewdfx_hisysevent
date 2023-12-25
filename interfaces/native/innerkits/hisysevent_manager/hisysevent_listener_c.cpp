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

#include "hisysevent_listener_c.h"

#include "hilog/log.h"
#include "hisysevent_record_convertor.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_LISTENER_C"

namespace {
using OHOS::HiviewDFX::HiSysEventRecordConvertor;
}

void HiSysEventListenerC::OnEvent(std::shared_ptr<OHOS::HiviewDFX::HiSysEventRecord> sysEvent)
{
    if (onEvent_ == nullptr || sysEvent == nullptr) {
        HILOG_ERROR(LOG_CORE, "OnEvent callback or sys event is null.");
        return;
    }
    HiSysEventRecordC record = {};
    HiSysEventRecordConvertor::ConvertRecord(*sysEvent, record);
    onEvent_(record);
}

void HiSysEventListenerC::OnServiceDied()
{
    if (onServiceDied_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "OnServiceDied callback is null.");
        return;
    }
    onServiceDied_();
}