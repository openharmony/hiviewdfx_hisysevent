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

#define MAX_EVENT_LIST_LEN 339

#define STATUS_MEM_FREED 0
#define STATUS_MEM_NEED_RECYCLE 1
#define STATUS_NORMAL 2

// rust ffi border redefinition adapts for struct HiSysEventParam.
struct HiSysEventParamWrapper {
    char paramName[MAX_LENGTH_OF_PARAM_NAME];
    int paramType;
    HiSysEventParamValue paramValue;
    unsigned int arraySize;
};
typedef struct HiSysEventParamWrapper HiSysEventParamWrapper;

struct HiSysEventQueryRuleWrapper {
    char domain[MAX_LENGTH_OF_EVENT_DOMAIN];
    char eventList[MAX_EVENT_LIST_LEN];
    unsigned int eventListSize;
    char* condition;
};
typedef struct HiSysEventQueryRuleWrapper HiSysEventQueryRuleWrapper;

typedef const void* OnRustCb;
typedef void (*OnEventWrapperCb) (OnRustCb, HiSysEventRecordC);
typedef void (*OnServiceDiedWrapperCb) (OnRustCb);
typedef void (*OnQueryWrapperCb)(OnRustCb, HiSysEventRecordC[], unsigned int);
typedef void (*OnCompleteWrapperCb)(OnRustCb, int, int);

struct HiSysEventRustWatcherC {
    OnRustCb onEventRustCb;
    OnEventWrapperCb onEventWrapperCb;
    OnRustCb onServiceDiedRustCb;
    OnServiceDiedWrapperCb onServiceDiedWrapperCb;
    int status;
};
typedef struct HiSysEventRustWatcherC HiSysEventRustWatcherC;

struct HiSysEventRustQuerierC {
    OnRustCb onQueryRustCb;
    OnQueryWrapperCb onQueryWrapperCb;
    OnRustCb onCompleteRustCb;
    OnCompleteWrapperCb onCompleteWrapperCb;
    int status;
};
typedef struct HiSysEventRustQuerierC HiSysEventRustQuerierC;

// rust ffi border redefinition adapts for function HiSysEvent_Write.
int HiSysEventWriteWrapper(const char* func, unsigned long long line, const char* domain, const char* name,
    int type, const HiSysEventParamWrapper params[], unsigned int size);

// rust ffi border redefinition adapts for function OH_HiSysEvent_Add_Watcher.
int HiSysEventAddWatcherWrapper(HiSysEventRustWatcherC* watcher, const HiSysEventWatchRule rules[],
    unsigned int ruleSize);

// rust ffi border redefinition adapts for function OH_HiSysEvent_Remove_Watcher.
int HiSysEventRemoveWatcherWrapper(HiSysEventRustWatcherC* watcher);

// rust ffi border redefinition adapts for function OH_HiSysEvent_Query.
int HiSysEventQueryWrapper(HiSysEventQueryArg* arg, const HiSysEventQueryRuleWrapper rules[],
    unsigned int ruleSize, HiSysEventRustQuerierC* querier);

// rust ffi border function
HiSysEventRecordC GetHiSysEventRecordByIndexWrapper(const HiSysEventRecordC records[], unsigned int total,
    unsigned int index);

// rust ffi border function
HiSysEventRustWatcherC* CreateRustEventWatcher(OnRustCb onEventRustCb, OnEventWrapperCb onEventWrapperCb,
    OnRustCb onServiceDiedRustCb, OnServiceDiedWrapperCb onServiceDiedWrapperCb);

// rust ffi border function
void RecycleRustEventWatcher(HiSysEventRustWatcherC* watcher);

// rust ffi border function
HiSysEventRustQuerierC* CreateRustEventQuerier(OnRustCb onQueryRustCb, OnQueryWrapperCb onQueryWrapperCb,
    OnRustCb onCompleteRustCb, OnCompleteWrapperCb onCompleteWrapperCb);

// rust ffi border function
void RecycleRustEventQuerier(HiSysEventRustQuerierC* querier);

#ifdef __cplusplus
}
#endif

#endif //HIVIEWDFX_HISYSEVENT_C_WRAPPER_H