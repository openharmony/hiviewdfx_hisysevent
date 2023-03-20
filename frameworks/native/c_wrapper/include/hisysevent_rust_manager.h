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

#ifndef HIVIEWDFX_HISYSEVENT_RUST_MANAGER_H
#define HIVIEWDFX_HISYSEVENT_RUST_MANAGER_H

#include "hisysevent_c_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

int OhHiSysEventRustQuery(HiSysEventQueryArg* queryArg, const HiSysEventQueryRule queryRules[],
    const size_t ruleSize, HiSysEventRustQuerierC* querier);

int OhHiSysEventAddRustWatcher(HiSysEventRustWatcherC* watcher, const HiSysEventWatchRule watchRules[],
    const size_t ruleSize);

int OhHiSysEventRemoveRustWatcher(HiSysEventRustWatcherC* watcher);

void OhHiSysEventRecycleRustWatcher(HiSysEventRustWatcherC* watcher);

void OhHiSysEventRecycleRustQuerier(HiSysEventRustQuerierC* querier);

#ifdef __cplusplus
}
#endif

#endif // HIVIEWDFX_HISYSEVENT_RUST_MANAGER_H