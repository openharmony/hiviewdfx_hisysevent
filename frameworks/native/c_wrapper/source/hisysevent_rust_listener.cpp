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

#include "hisysevent_rust_listener.h"

#include "hilog/log.h"
#include "hisysevent_record_convertor.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_RUST_LISTENER"

namespace {
using OHOS::HiviewDFX::HiSysEventRecordConvertor;
}

HiSysEventRustListener::HiSysEventRustListener(HiSysEventRustWatcherC* watcher)
    : watcher_(watcher)
{
}

HiSysEventRustListener::~HiSysEventRustListener()
{
    RecycleWatcher(watcher_);
}

void HiSysEventRustListener::OnEvent(std::shared_ptr<OHOS::HiviewDFX::HiSysEventRecord> sysEvent)
{
    if (watcher_ == nullptr || sysEvent == nullptr) {
        HILOG_ERROR(LOG_CORE, "OnEvent callback or sys event is null.");
        return;
    }
    HiSysEventRecordC record;
    HiSysEventRecordConvertor::ConvertRecord(*sysEvent, record);
    watcher_->onEventWrapperCb(watcher_->onEventRustCb, record);
}

void HiSysEventRustListener::OnServiceDied()
{
    if (watcher_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "OnServiceDied callback is null.");
        return;
    }
    watcher_->onServiceDiedWrapperCb(watcher_->onServiceDiedRustCb);
}

void HiSysEventRustListener::RecycleWatcher(HiSysEventRustWatcherC* watcher)
{
    std::lock_guard<std::mutex> lock(listenerMutex_);
    if ((watcher == nullptr) || (watcher->status == STATUS_MEM_FREED)) {
        return;
    }
    if (watcher->status == STATUS_MEM_NEED_RECYCLE) {
        watcher->status = STATUS_MEM_FREED;
        delete watcher;
        return;
    }
    watcher->status = STATUS_MEM_NEED_RECYCLE;
}