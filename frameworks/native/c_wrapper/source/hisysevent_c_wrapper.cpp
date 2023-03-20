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

#include "hisysevent_c_wrapper.h"

#include "securec.h"
#include "hisysevent_record_convertor.h"
#include "hisysevent_rust_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void ConvertParamWrapper(const HiSysEventParamWrapper src[], HiSysEventParam dest[], size_t size)
{
    for (size_t i = 0; i < size; i++) {
        HiSysEventParamWrapper wrapper = src[i];
        for (size_t j = 0; j < MAX_LENGTH_OF_PARAM_NAME && wrapper.paramName[j] != '\0'; j++) {
            dest[i].name[j] = wrapper.paramName[j];
        }
        dest[i].t = HiSysEventParamType(wrapper.paramType);
        dest[i].v = wrapper.paramValue;
        dest[i].arraySize = wrapper.arraySize;
    }
}

static void SplitStringToArray(const char src[], size_t srcMaxLen, char dest[][MAX_LENGTH_OF_EVENT_NAME],
    size_t destSize)
{
    size_t curPos = 0; // curnt position is initialized to 0.
    size_t destItemIndex = 0; // array item index is initialized to 0.
    size_t sliceBegin = 0; // slice begin position is initialized to 0.
    size_t sliceEnd = 1; // slice end position is initialized to1.
    while (curPos < srcMaxLen && src[curPos] != '\0') {
        if (src[curPos] != '|') {
            curPos++;
            continue;
        }
        sliceEnd = curPos - 1; // slice end position set to be the position of last charactor before charactor '|'.
        int reSliceOffset = 2; // "1|2" the offset between charactor 1 and 2 is always 2.
        if (sliceEnd - sliceBegin + 1 > MAX_LENGTH_OF_EVENT_NAME) {
            sliceBegin = sliceEnd + reSliceOffset; // ignore event name with invalid length.
            curPos++;
            continue;
        }
        if (memcpy_s(dest[destItemIndex], sliceEnd - sliceBegin + 1, src + sliceBegin,
            sliceEnd - sliceBegin + 1) != EOK) {
            sliceBegin = sliceEnd + reSliceOffset;
            curPos++;
            continue; // item copy failed, continue to copy next one.
        }
        sliceBegin = sliceEnd + reSliceOffset;
        curPos++;
        destItemIndex++;
        if (destItemIndex >= destSize) {
            break;
        }
    }
    if (curPos >= srcMaxLen || src[curPos] == '\0') {
        sliceEnd = curPos - 1; // slice end position set to be the position of last charactor before charactor '|'.
    }
    if (destItemIndex >= destSize ||
        sliceEnd - sliceBegin + 1 > MAX_LENGTH_OF_EVENT_NAME) {
        return;
    }
    if (memcpy_s(dest[destItemIndex], sliceEnd - sliceBegin + 1, src + sliceBegin,
        sliceEnd - sliceBegin + 1) != EOK) {
        return; // item copy failed, ignore directly.
    }
    return;
}

static inline void ConvertQueryRuleWrapper(const HiSysEventQueryRuleWrapper src[], HiSysEventQueryRule dest[],
    const size_t size)
{
    for (size_t i = 0; i < size; i++) {
        HiSysEventQueryRuleWrapper wrapper = src[i];
        for (size_t j = 0; (j < MAX_LENGTH_OF_EVENT_DOMAIN) && (wrapper.domain[j] != '\0'); j++) {
            dest[i].domain[j] = wrapper.domain[j];
        }
        SplitStringToArray(wrapper.eventList, MAX_NUMBER_OF_EVENT_LIST_WRAPPER, dest[i].eventList,
            wrapper.eventListSize);
        dest[i].eventListSize = wrapper.eventListSize;
        dest[i].condition = wrapper.condition;
    }
}

int HiSysEventWriteWrapper(const char* func, uint64_t line, const char* domain, const char* name,
    int type, const HiSysEventParamWrapper paramWrappers[], const size_t size)
{
    HiSysEventParam params[size];
    ConvertParamWrapper(paramWrappers, params, size);
    return HiSysEvent_Write(func, line, domain, name, HiSysEventEventType(type), params, size);
}

int HiSysEventAddWatcherWrapper(HiSysEventRustWatcherC* watcher, const HiSysEventWatchRule rules[],
    const size_t ruleSize)
{
    return OhHiSysEventAddRustWatcher(watcher, rules, ruleSize);
}

int HiSysEventRemoveWatcherWrapper(HiSysEventRustWatcherC* watcher)
{
    return OhHiSysEventRemoveRustWatcher(watcher);
}

int HiSysEventQueryWrapper(HiSysEventQueryArg* arg, const HiSysEventQueryRuleWrapper queryRules[],
    const size_t ruleSize, HiSysEventRustQuerierC* querier)
{
    HiSysEventQueryRule rules[ruleSize];
    ConvertQueryRuleWrapper(queryRules, rules, ruleSize);
    return OhHiSysEventRustQuery(arg, rules, ruleSize, querier);
}

HiSysEventRecordC GetHiSysEventRecordByIndexWrapper(const HiSysEventRecordC records[], const uint32_t total,
    const uint32_t index)
{
    if (index >= total) {
        HiSysEventRecordC record = {};
        return record;
    }
    return records[index];
}

HiSysEventRustWatcherC* CreateRustEventWatcher(OnRustCb onEventRustCb,
    OnEventWrapperCb onEventWrapperCb, OnRustCb onServiceDiedRustCb,
    OnServiceDiedWrapperCb onServiceDiedWrapperCb)
{
    if (onEventRustCb == nullptr || onEventWrapperCb == nullptr ||
        onServiceDiedRustCb == nullptr || onServiceDiedWrapperCb == nullptr) {
        return nullptr;
    }
    HiSysEventRustWatcherC* watcher = new(std::nothrow) HiSysEventRustWatcherC;
    watcher->onEventRustCb = onEventRustCb;
    watcher->onEventWrapperCb = onEventWrapperCb;
    watcher->onServiceDiedRustCb = onServiceDiedRustCb;
    watcher->onServiceDiedWrapperCb = onServiceDiedWrapperCb;
    watcher->status = STATUS_NORMAL;
    return watcher;
}

void RecycleRustEventWatcher(HiSysEventRustWatcherC* watcher)
{
    OhHiSysEventRecycleRustWatcher(watcher);
}

HiSysEventRustQuerierC* CreateRustEventQuerier(OnRustCb onQueryRustCb,
    OnQueryWrapperCb onQueryWrapperCb, OnRustCb onCompleteRustCb,
    OnCompleteWrapperCb onCompleteWrapperCb)
{
    if (onQueryRustCb == nullptr || onQueryWrapperCb == nullptr ||
        onCompleteRustCb == nullptr || onCompleteWrapperCb == nullptr) {
        return nullptr;
    }
    HiSysEventRustQuerierC* querier = new(std::nothrow) HiSysEventRustQuerierC;
    querier->onQueryRustCb = onQueryRustCb;
    querier->onQueryWrapperCb = onQueryWrapperCb;
    querier->onCompleteRustCb = onCompleteRustCb;
    querier->onCompleteWrapperCb = onCompleteWrapperCb;
    querier->status = STATUS_NORMAL;
    return querier;
}

void RecycleRustEventQuerier(HiSysEventRustQuerierC* querier)
{
    OhHiSysEventRecycleRustQuerier(querier);
}

#ifdef __cplusplus
}
#endif