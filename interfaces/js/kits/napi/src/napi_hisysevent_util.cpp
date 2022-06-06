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

#include "napi_hisysevent_util.h"

#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "NAPI_HISYSEVENT_UTIL" };
constexpr uint32_t BUF_SIZE = 10240;
constexpr int SYS_EVENT_INFO_PARAM = 0;
constexpr char DOMAIN_ATTR[] = "domain";
constexpr char NAME_ATTR[] = "name";
constexpr char EVENT_TYPE_ATTR[] = "eventType";
constexpr char PARAMS_ATTR[] = "params";
const std::string INVALID_KEY_TYPE_ARR[] = {
    "[object Object]",
    "null",
    "()",
    ","
};

napi_valuetype GetValueType(const napi_env env, const napi_value& value)
{
    napi_valuetype valueType = napi_undefined;
    napi_status ret = napi_typeof(env, value, &valueType);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse the type of napi value.");
    }
    return valueType;
}

bool CheckValueTypeValidity(const napi_env env, const napi_value& jsObj,
    const napi_valuetype typeName)
{
    napi_valuetype valueType = GetValueType(env, jsObj);
    if (valueType != typeName) {
        HiLog::Error(LABEL, "napi value type not match: valueType=%{public}d, typeName=%{public}d.",
            valueType, typeName);
        return false;
    }
    return true;
}

bool CheckValueIsArray(const napi_env env, const napi_value& jsObj)
{
    if (!CheckValueTypeValidity(env, jsObj, napi_valuetype::napi_object)) {
        return false;
    }
    bool isArray = false;
    napi_status ret = napi_is_array(env, jsObj, &isArray);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to check array napi value.");
    }
    return isArray;
}

bool ParseBoolValue(const napi_env env, const napi_value& value)
{
    bool boolValue = true;
    napi_status ret = napi_get_value_bool(env, value, &boolValue);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse napi value of boolean type.");
    }
    return boolValue;
}

double ParseNumberValue(const napi_env env, const napi_value& value)
{
    double numValue = 0;
    napi_status ret = napi_get_value_double(env, value, &numValue);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse napi value of number type.");
    }
    return numValue;
}

double ParseBigIntValue(const napi_env env, const napi_value& value)
{
    uint64_t bigIntValue = 0;
    bool lossless = true;
    napi_status ret = napi_get_value_bigint_uint64(env, value, &bigIntValue, &lossless);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse napi value of big int type.");
    }
    return static_cast<double>(bigIntValue);
}

std::string ParseStringValue(const napi_env env, const napi_value& value)
{
    char buf[BUF_SIZE] = {0};
    size_t bufLength = 0;
    napi_status ret = napi_get_value_string_utf8(env, value, buf, BUF_SIZE, &bufLength);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse napi value of string type.");
    }
    std::string dest = std::string {buf};
    return dest;
}

bool CheckKeyTypeString(const std::string& str)
{
    bool ret = true;
    for (auto invalidType : INVALID_KEY_TYPE_ARR) {
        if (str.find(invalidType) != std::string::npos) {
            ret = false;
            break;
        }
    }
    return ret;
}

napi_value GetPropertyByName(const napi_env env, const napi_value& object,
    const std::string& propertyName)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, object, propertyName.c_str(), &result));
    return result;
}

std::string GetStringTypeAttribute(const napi_env env, const napi_value& object,
    const std::string& propertyName)
{
    napi_value propertyValue = GetPropertyByName(env, object, propertyName);
    napi_valuetype type = GetValueType(env, propertyValue);
    if (type != napi_valuetype::napi_string) {
        HiLog::Error(LABEL, "type is not napi_string.");
        return "";
    }
    return ParseStringValue(env, propertyValue);
}

int32_t ParseInt32Value(const napi_env env, const napi_value& value)
{
    int32_t int32Value = 0;
    napi_status ret = napi_get_value_int32(env, value, &int32Value);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "failed to parse napi value of int32_t type.");
    }
    return int32Value;
}

int32_t GetInt32TypeAttribute(const napi_env env, const napi_value& object,
    const std::string& propertyName)
{
    napi_value propertyValue = GetPropertyByName(env, object, propertyName);
    napi_valuetype type = GetValueType(env, propertyValue);
    if (type != napi_valuetype::napi_number) {
        HiLog::Error(LABEL, "type is not napi_number.");
        return HiSysEvent::EventType::FAULT;
    }
    return ParseInt32Value(env, propertyValue);
}

void AppendBoolArrayData(const napi_env env, HiSysEventInfo& info, const std::string key,
    const napi_value array, size_t len)
{
    std::vector<bool> values;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of array.");
            continue;
        }
        if (CheckValueTypeValidity(env, element, napi_valuetype::napi_boolean)) {
            values.push_back(ParseBoolValue(env, element));
        }
    }
    info.boolArrayParams[key] = values;
}

void AppendNumberArrayData(const napi_env env, HiSysEventInfo& info, const std::string key,
    const napi_value array, size_t len)
{
    std::vector<double> values;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of array.");
            continue;
        }
        if (CheckValueTypeValidity(env, element, napi_valuetype::napi_number)) {
            values.push_back(ParseNumberValue(env, element));
        }
    }
    info.doubleArrayParams[key] = values;
}

void AppendBigIntArrayData(const napi_env env, HiSysEventInfo& info, const std::string key,
    const napi_value array, size_t len)
{
    std::vector<double> values;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of array.");
            continue;
        }
        if (CheckValueTypeValidity(env, element, napi_valuetype::napi_bigint)) {
            values.push_back(ParseBigIntValue(env, element));
        }
    }
    info.doubleArrayParams[key] = values;
}

void AppendStringArrayData(const napi_env env, HiSysEventInfo& info, const std::string key,
    const napi_value array, size_t len)
{
    std::vector<std::string> values;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of number array.");
            continue;
        }
        if (CheckValueTypeValidity(env, element, napi_valuetype::napi_string)) {
            values.push_back(ParseStringValue(env, element));
        }
    }
    info.stringArrayParams[key] = values;
}

void AddArrayParamToEventInfo(const napi_env env, HiSysEventInfo& info, const std::string& key, napi_value& array)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, array, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the length of this array.");
        return;
    }
    if (len == 0) {
        HiLog::Warn(LABEL, "length of this array is 0.");
        return;
    }
    napi_value firstItem;
    status = napi_get_element(env, array, 0, &firstItem);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the first element of array.");
        return;
    }
    napi_valuetype type;
    status = napi_typeof(env, firstItem, &type);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the type of the first element in array.");
        return;
    }
    switch (type) {
        case napi_valuetype::napi_boolean:
            AppendBoolArrayData(env, info, key, array, len);
            HiLog::Debug(LABEL, "AppendBoolArrayData: %{public}s.", key.c_str());
            break;
        case napi_valuetype::napi_number:
            AppendNumberArrayData(env, info, key, array, len);
            HiLog::Debug(LABEL, "AppendNumberArrayData: %{public}s.", key.c_str());
            break;
        case napi_valuetype::napi_bigint:
            AppendBigIntArrayData(env, info, key, array, len);
            HiLog::Debug(LABEL, "AppendBigIntArrayData: %{public}s.", key.c_str());
            break;
        case napi_valuetype::napi_string:
            AppendStringArrayData(env, info, key, array, len);
            HiLog::Debug(LABEL, "AppendStringArrayData: %{public}s.", key.c_str());
            break;
        default:
            break;
    }
}

void AddParamToEventInfo(const napi_env env, HiSysEventInfo& info, const std::string& key, napi_value& value)
{
    if (CheckValueIsArray(env, value)) {
        AddArrayParamToEventInfo(env, info, key, value);
        return;
    }
    napi_valuetype type = GetValueType(env, value);
    switch (type) {
        case napi_valuetype::napi_boolean:
            info.boolParams[key] = ParseBoolValue(env, value);
            break;
        case napi_valuetype::napi_number:
            info.doubleParams[key] = ParseNumberValue(env, value);
            break;
        case napi_valuetype::napi_string:
            info.stringParams[key] = ParseStringValue(env, value);
            break;
        case napi_valuetype::napi_bigint:
            info.boolParams[key] = ParseBigIntValue(env, value);
            break;
        default:
            break;
    }
}

void GetObjectTypeAttribute(const napi_env env, const napi_value& object,
    const std::string& propertyName, HiSysEventInfo& info)
{
    napi_value propertyValue = GetPropertyByName(env, object, propertyName);
    napi_valuetype type = GetValueType(env, propertyValue);
    if (type != napi_valuetype::napi_object) {
        HiLog::Error(LABEL, "type is not napi_object.");
        return;
    }

    napi_value keyArr = nullptr;
    napi_status status = napi_get_property_names(env, propertyValue, &keyArr);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to parse names of the property.");
        return;
    }
    uint32_t len = 0;
    status = napi_get_array_length(env, keyArr, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the length of the kv pairs.");
        return;
    }
    for (uint32_t i = 0; i < len; i++) {
        napi_value key = nullptr;
        napi_get_element(env, keyArr, i, &key);

        napi_valuetype valueType = GetValueType(env, key);
        if (valueType != napi_valuetype::napi_string) {
            HiLog::Warn(LABEL, "this param would be discarded because of invalid type of the key.");
            continue;
        }
        char buf[BUF_SIZE] = {0};
        size_t valueLen = 0;
        napi_get_value_string_utf8(env, key, buf, BUF_SIZE - 1, &valueLen);
        if (!CheckKeyTypeString(buf)) {
            HiLog::Warn(LABEL, "this param would be discarded because of invalid format of the key.");
            continue;
        }
        napi_value val = GetPropertyByName(env, propertyValue, buf);
        AddParamToEventInfo(env, info, buf, val);
    }
}
}

void NapiHiSysEventUtil::ParseHiSysEventInfo(const napi_env env, napi_value* param,
    size_t paramNum, HiSysEventInfo& info)
{
    if (paramNum <= SYS_EVENT_INFO_PARAM) {
        return;
    }
    info.domain = GetStringTypeAttribute(env, param[SYS_EVENT_INFO_PARAM], DOMAIN_ATTR);
    HiLog::Debug(LABEL, "domain is %{public}s.", info.domain.c_str());
    info.name = GetStringTypeAttribute(env, param[SYS_EVENT_INFO_PARAM], NAME_ATTR);
    HiLog::Debug(LABEL, "name is %{public}s.", info.name.c_str());
    info.eventType = HiSysEvent::EventType(GetInt32TypeAttribute(env,
        param[SYS_EVENT_INFO_PARAM], EVENT_TYPE_ATTR));
    HiLog::Debug(LABEL, "eventType is %{public}d.", info.eventType);
    GetObjectTypeAttribute(env, param[SYS_EVENT_INFO_PARAM], PARAMS_ATTR, info);
}
} // namespace HiviewDFX
} // namespace OHOS