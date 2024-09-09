/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hisysevent_easy.h"

#include <stddef.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "easy_def.h"
#include "easy_event_builder.h"
#include "easy_socket_writer.h"
#include "easy_util.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char CUSTOMIZED_PARAM_KEY[] = "DATA";
static const int SEC_2_MILLIS = 1000;
static const int MILLS_2_NANOS = 1000000;

static int64_t GetTimestamp()
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    int64_t time = tp.tv_sec * SEC_2_MILLIS;
    time += tp.tv_nsec / MILLS_2_NANOS;
    return time;
}

static uint8_t ParseTimeZone(long tz)
{
    static long allTimeZones[] = {
        3600, 7200, 10800, 11880, 14400, 18000, 21600,
        25200, 28800, 32400, 33480, 36000, 39600, 43200,
        0, -3600, -7200, -10800, -11880, -14400, 15480,
        -18000, -19080, -19620, -21600, -22680, -25200, -28800,
        -30420, -32400, -33480, -36000, -37080, -39600, -43200,
        -44820, -46800, -50400
    };
    uint8_t ret = 14; // 14 is the index of "+0000" in array
    for (uint8_t index = 0; index < (sizeof(allTimeZones) / sizeof(long)); ++index) {
        if (allTimeZones[index] == tz) {
            ret = index;
            break;
        }
    }
    return ret;
}

static int CheckEventType(uint8_t eventType)
{
    if ((eventType < EASY_EVENT_TYPE_FAULT) || (eventType > EASY_EVENT_TYPE_BEHAVIOR)) {
        return ERR_TYPE_INVALID;
    }
    return SUCCESS;
}

static int InitEventHeader(struct HiSysEventEasyHeader* header, const char* domain, const char* name,
    const uint8_t eventType)
{
    if ((MemoryCopy((uint8_t*)(header->domain), DOMAIN_ARRAY_LEN, (uint8_t*)domain, strlen(domain)) != SUCCESS) ||
        (MemoryCopy((uint8_t*)(header->name), NAME_ARRAY_LEN, (uint8_t*)name, strlen(name)) != SUCCESS)) {
        return ERR_MEM_OPT_FAILED;
    }
    header->type = eventType - 1; // only 2 bits to store event type
    header->timestamp = GetTimestamp();
    tzset();
    header->timeZone = ParseTimeZone(timezone);
    header->pid = (uint32_t)getpid();
    header->tid = (uint32_t)gettid();
    header->uid = (uint32_t)getuid();
    header->isTraceOpened = 0; // no need to allocate memory for trace info.
    return SUCCESS;
}

int HiSysEventEasyWrite(const char* domain, const char* name, enum HiSysEventEasyType eventType, const char* data)
{
    if ((domain == NULL) || (strlen(domain) > MAX_DOMAIN_LENGTH)) {
        return ERR_DOMAIN_INVALID;
    }
    if ((name == NULL) || (strlen(name) > MAX_EVENT_NAME_LENGTH)) {
        return ERR_NAME_INVALID;
    }
    int ret = CheckEventType(eventType);
    if (ret != SUCCESS) {
        return ret;
    }
    uint8_t eventBuffer[EVENT_BUFF_LEN] = { 0 };
    size_t offset = 0;
    // applend block size
    *((int32_t*)eventBuffer) = EVENT_BUFF_LEN;
    offset += sizeof(int32_t);
    // append header, only two bits to store event type in memory
    struct HiSysEventEasyHeader header;
    ret = MemoryInit((uint8_t*)(&header), sizeof(struct HiSysEventEasyHeader));
    if (ret != SUCCESS) {
        return ret;
    }
    ret = InitEventHeader(&header, domain, name, eventType);
    if (ret != SUCCESS) {
        return ret;
    }
    ret = AppendHeader(eventBuffer, EVENT_BUFF_LEN, &offset, &header);
    if (ret != SUCCESS) {
        return ret;
    }
    // append param count, only one cutomized parameter
    if (offset + sizeof(int32_t) > EVENT_BUFF_LEN) {
        return ERR_EVENT_BUF_INVALID;
    }
    *((int32_t*)(eventBuffer + offset)) = 1;
    offset += sizeof(int32_t);
    ret = AppendStringParam(eventBuffer, EVENT_BUFF_LEN, &offset, CUSTOMIZED_PARAM_KEY, data);
    if (ret != SUCCESS) {
        return ret;
    }
    // write event to socket
    ret = Write(eventBuffer, EVENT_BUFF_LEN);
    if (ret != SUCCESS) {
        return ret;
    }
    return SUCCESS;
}

#ifdef __cplusplus
}
#endif