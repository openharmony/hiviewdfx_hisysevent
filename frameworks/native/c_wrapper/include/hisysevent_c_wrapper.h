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

#ifndef HIVIEWDFX_HISYSEVENT_C_WRAPPER_H
#define HIVIEWDFX_HISYSEVENT_C_WRAPPER_H

#include "hisysevent_c.h"
#include "hisysevent_manager_c.h"
#include "hisysevent_record_c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NUMBER_OF_EVENT_LIST_WRAPPER 594

// rust ffi border redefinition adapts for struct HiSysEventParam.
struct HiSysEventParamWrapper {
    char paramName[MAX_LENGTH_OF_PARAM_NAME];
    int paramType;
    HiSysEventParamValue paramValue;
    size_t arraySize;
};
typedef struct HiSysEventParamWrapper HiSysEventParamWrapper;

struct HiSysEventQueryRuleWrapper {
    char domain[MAX_LENGTH_OF_EVENT_DOMAIN];
    char eventList[MAX_NUMBER_OF_EVENT_LIST_WRAPPER];
    int eventListSize;
    char* condition;
};
typedef struct HiSysEventQueryRuleWrapper HiSysEventQueryRuleWrapper;

// rust ffi border redefinition adapts for function HiSysEvent_Write.
int HiSysEventWriteWrapper(const char* func, uint64_t line, const char* domain, const char* name,
    int type, const HiSysEventParamWrapper params[], const size_t size);

// rust ffi border redefinition adapts for function OH_HiSysEvent_Add_Watcher.
int HiSysEventAddWatcherWrapper(HiSysEventWatcher* watcher, HiSysEventWatchRule rules[], const size_t ruleSize);

// rust ffi border redefinition adapts for function OH_HiSysEvent_Remove_Watcher.
int HiSysEventRemoveWatcherWrapper(HiSysEventWatcher* watcher);

// rust ffi border redefinition adapts for function OH_HiSysEvent_Query.
int HiSysEventQueryWrapper(const HiSysEventQueryArg* arg, HiSysEventQueryRuleWrapper rules[], const size_t ruleSize,
    HiSysEventQueryCallback* callback);

// rust ffi border function
HiSysEventRecordC GetHiSysEventRecordByIndexWrapper(HiSysEventRecordC records[], int total, int index);

#ifdef __cplusplus
}
#endif

#endif //HIVIEWDFX_HISYSEVENT_C_WRAPPER_H