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

static inline void ConvertStringToArray(char src[], char dest[], int index)
{
    int begin = index - 1;
    int end = index;
    int pos = 0;
    int cur = 0;
    int beginPos = 0;
    int endPos = 1;
    while (pos != end && src[cur] != '\0') {
        if (src[cur] != '|') {
            cur++;
            continue;
        }
        pos++;
        if (pos == begin) {
            beginPos = cur + 1;
        }
        if (pos == end) {
            endPos = cur - 1;
        }
        cur++;
    }
    if (src[cur] == '\0') {
        endPos = cur - 1;
    }
    for (int i = 0; i < endPos - beginPos + 1; i++) {
        dest[i] = src[beginPos + i];
    }
    dest[endPos - beginPos + 1] = '\0';
}

static inline void ConvertQueryRuleWrapper(const HiSysEventQueryRuleWrapper src[], HiSysEventQueryRule dest[],
    size_t size)
{
    for (size_t i = 0; i < size; i++) {
        HiSysEventQueryRuleWrapper wrapper = src[i];
        for (size_t j = 0; j < MAX_LENGTH_OF_EVENT_DOMAIN && wrapper.domain[j] != '\0'; j++) {
            dest[i].domain[j] = wrapper.domain[j];
        }
        
        for (int j = 1; j <= wrapper.eventListSize; j++) {
            ConvertStringToArray(wrapper.eventList, dest[i].eventList[j - 1], wrapper.eventListSize);
        }
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

int HiSysEventAddWatcherWrapper(HiSysEventWatcher* watcher, HiSysEventWatchRule rules[], const size_t ruleSize)
{
    return OH_HiSysEvent_Add_Watcher(watcher, rules, ruleSize);
}

int HiSysEventRemoveWatcherWrapper(HiSysEventWatcher* watcher)
{
    return OH_HiSysEvent_Remove_Watcher(watcher);
}

int HiSysEventQueryWrapper(const HiSysEventQueryArg* arg, HiSysEventQueryRuleWrapper queryRules[],
    const size_t ruleSize, HiSysEventQueryCallback* callback)
{
    HiSysEventQueryRule rules[ruleSize];
    ConvertQueryRuleWrapper(queryRules, rules, ruleSize);
    return OH_HiSysEvent_Query(arg, rules, ruleSize, callback);
}

HiSysEventRecordC GetHiSysEventRecordByIndexWrapper(HiSysEventRecordC records[], int total, int index)
{
    if (index >= total) {
        index = 0;
    }
    return records[index];
}