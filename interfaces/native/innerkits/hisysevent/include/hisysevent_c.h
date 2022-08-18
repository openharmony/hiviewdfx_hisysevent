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
#ifndef HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_HISYSEVENT_C_H
#define HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_HISYSEVENT_C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief define parameter value type
 */
enum HiSysEventParamType {
    HISYSEVENT_INVALID = 0,
    HISYSEVENT_BOOL = 1,
    HISYSEVENT_INT8 = 2,
    HISYSEVENT_UINT8 = 3,
    HISYSEVENT_INT16 = 4,
    HISYSEVENT_UINT16 = 5,
    HISYSEVENT_INT32 = 6,
    HISYSEVENT_UINT32 = 7,
    HISYSEVENT_INT64 = 8,
    HISYSEVENT_UINT64 = 9,
    HISYSEVENT_FLOAT = 10,
    HISYSEVENT_DOUBLE = 11,
    HISYSEVENT_STRING = 12,
    HISYSEVENT_BOOL_ARRAY = 13,
    HISYSEVENT_INT8_ARRAY = 14,
    HISYSEVENT_UINT8_ARRAY = 15,
    HISYSEVENT_INT16_ARRAY = 16,
    HISYSEVENT_UINT16_ARRAY = 17,
    HISYSEVENT_INT32_ARRAY = 18,
    HISYSEVENT_UINT32_ARRAY = 19,
    HISYSEVENT_INT64_ARRAY = 20,
    HISYSEVENT_UINT64_ARRAY = 21,
    HISYSEVENT_FLOAT_ARRAY = 22,
    HISYSEVENT_DOUBLE_ARRAY = 23,
    HISYSEVENT_STRING_ARRAY = 24
};
typedef enum HiSysEventParamType HiSysEventParamType;

/**
 * @brief define type of parameter value
 */
union HiSysEventParamValue {
    bool b;
    int8_t i8;
    uint8_t ui8;
    int16_t i16;
    uint16_t ui16;
    int32_t i32;
    uint32_t ui32;
    int64_t i64;
    uint64_t ui64;
    float f;
    double d;
    char *s;
    void *array;
};
typedef union HiSysEventParamValue HiSysEventParamValue;

/**
 * @brief define parameter name with value
 */
struct HiSysEventParam {
    char name[33];
    HiSysEventParamType t;
    HiSysEventParamValue v;
    size_t arraySize;
};
typedef struct HiSysEventParam HiSysEventParam;

/**
 * @brief event type
 */
enum HiSysEventEventType {
    HISYSEVENT_FAULT = 1,
    HISYSEVENT_STATISTIC = 2,
    HISYSEVENT_SECURITY = 3,
    HISYSEVENT_BEHAVIOR = 4
};
typedef enum HiSysEventEventType HiSysEventEventType;

/**
 * @brief write sysevnt to hiview
 * @param domain event domain
 * @param name   event name
 * @param type   event type
 * @param params parameter of event
 * @param size the size of paramter list
 * @return int  less than 0 fail to send hiview, 0: successful, large than 0 discard some event parameter
 */
int OH_HiSysEvent_Write(const char* domain, const char* name, HiSysEventEventType type,
    HiSysEventParam params[], size_t size);

#ifdef __cplusplus
}
#endif
#endif // HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_HISYSEVENT_C_H