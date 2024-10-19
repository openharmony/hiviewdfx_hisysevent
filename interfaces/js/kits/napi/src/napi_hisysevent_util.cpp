/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include <tuple>
#include <variant>

#include "def.h"
#include "hilog/log.h"
#include "ipc_skeleton.h"
#include "json/json.h"
#include "ret_code.h"
#include "ret_def.h"
#include "stringfilter.h"
#include "tokenid_kit.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "NAPI_HISYSEVENT_UTIL"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr uint32_t JS_STR_PARM_LEN_LIMIT = 1024 * 10; // 10k
constexpr uint32_t BUF_SIZE = 1024 * 11; // 11k
constexpr int SYS_EVENT_INFO_PARAM_INDEX = 0;
constexpr long long DEFAULT_TIME_STAMP = -1;
constexpr long long DEFAULT_SEQ = 0;
constexpr int32_t DEFAULT_MAX_EVENTS = 1000;
constexpr char PARAMS_ATTR[] = "params";
constexpr char TAG_ATTR[] = "tag";
constexpr char RULE_TYPE_ATTR[] = "ruleType";
constexpr char BEGIN_TIME_ATTR[] = "beginTime";
constexpr char END_TIME_ATTR[] = "endTime";
constexpr char MAX_EVENTS_ATTR[] = "maxEvents";
constexpr char BEGIN_SEQ_ATTR[] = "fromSeq";
constexpr char END_SEQ_ATTR[] = "toSeq";
constexpr char NAMES_ATTR[] = "names";
constexpr char CONDITION_ATTR[] = "condition";
constexpr char DOMAIN__KEY[] = "domain_";
constexpr char NAME__KEY[] = "name_";
constexpr char TYPE__KEY[] = "type_";
using I64ParseResult = std::pair<bool, int64_t>;
using U64ParseResult = std::pair<bool, uint64_t>;
using ParsedItem = std::variant<uint64_t, int64_t, double>;
enum ParsedItemIndex {
    U64_INDEX = 0,
    I64_INDEX,
    DOUBLE_INDEX,
};

enum ArrayParseStatus {
    AS_U64_ARR,
    AS_I64_ARR,
    AS_DOUBLE_ARR,
};

const std::string INVALID_KEY_TYPE_ARR[] = {
    "[object Object]",
    "null",
    "()",
    ","
};

template<typename T>
void AppendParamsToEventInfo(std::vector<ParsedItem>& src,
    HiSysEventInfo& info, const std::string& key)
{
    std::vector<std::decay_t<T>> dest;
    for (auto& item : src) {
        if (item.index() == U64_INDEX) {
            dest.emplace_back(static_cast<std::decay_t<T>>(std::get<U64_INDEX>(item)));
        } else if (item.index() == I64_INDEX) {
            dest.emplace_back(static_cast<std::decay_t<T>>(std::get<I64_INDEX>(item)));
        } else {
            dest.emplace_back(static_cast<std::decay_t<T>>(std::get<DOUBLE_INDEX>(item)));
        }
    }
    info.params[key] = dest;
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

bool IsFloatingNumber(double originalVal)
{
    int64_t i64Val = static_cast<int64_t>(originalVal);
    double i64ToDVal = static_cast<double>(i64Val);
    return fabs(i64ToDVal - originalVal) > 1e-9; // a minumum value
}

napi_valuetype GetValueType(const napi_env env, const napi_value& value)
{
    napi_valuetype valueType = napi_undefined;
    napi_status ret = napi_typeof(env, value, &valueType);
    if (ret != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to parse the type of napi value.");
    }
    return valueType;
}

bool IsValueTypeValid(const napi_env env, const napi_value& jsObj,
    const napi_valuetype typeName)
{
    napi_valuetype valueType = GetValueType(env, jsObj);
    if (valueType != typeName) {
        HILOG_DEBUG(LOG_CORE, "napi value type not match: valueType=%{public}d, typeName=%{public}d.",
            valueType, typeName);
        return false;
    }
    return true;
}

bool CheckValueIsArray(const napi_env env, const napi_value& jsObj)
{
    if (!IsValueTypeValid(env, jsObj, napi_valuetype::napi_object)) {
        return false;
    }
    bool isArray = false;
    napi_status ret = napi_is_array(env, jsObj, &isArray);
    if (ret != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to check array napi value.");
    }
    return isArray;
}

bool ParseBoolValue(const napi_env env, const napi_value& value, bool defalutValue = false)
{
    bool boolValue = defalutValue;
    napi_status ret = napi_get_value_bool(env, value, &boolValue);
    if (ret != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to parse napi value of boolean type.");
    }
    return boolValue;
}

double ParseNumberValue(const napi_env env, const napi_value& value, double defaultValue = 0.0)
{
    double numValue = defaultValue;
    napi_status ret = napi_get_value_double(env, value, &numValue);
    if (ret != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to parse napi value of number type.");
    }
    return numValue;
}

std::string ParseStringValue(const napi_env env, const napi_value& value, std::string defaultValue = "")
{
    char buf[BUF_SIZE] = {0};
    size_t bufLength = 0;
    napi_status status = napi_get_value_string_utf8(env, value, buf, BUF_SIZE - 1, &bufLength);
    if (status != napi_ok) {
        HILOG_DEBUG(LOG_CORE, "failed to parse napi value of string type.");
        return defaultValue;
    }
    std::string dest = std::string {buf};
    return dest;
}

I64ParseResult ParseSignedBigIntValue(const napi_env env, const napi_value& value)
{
    I64ParseResult ret = std::make_pair(false, 0); // default result
    int64_t val = 0;
    bool lossless = true;
    napi_status status = napi_get_value_bigint_int64(env, value, &val, &lossless);
    if (status != napi_ok) {
        HILOG_DEBUG(LOG_CORE, "failed to parse int64_t number.");
        return ret;
    }
    ret.first = lossless;
    ret.second = val;
    return ret;
}

U64ParseResult ParseUnsignedBigIntValue(const napi_env env, const napi_value& value)
{
    U64ParseResult ret = std::make_pair(false, 0); // default result
    uint64_t val = 0;
    bool lossless = true;
    napi_status status = napi_get_value_bigint_uint64(env, value, &val, &lossless);
    if (status != napi_ok) {
        HILOG_DEBUG(LOG_CORE, "failed to parse uint64_t number.");
        return ret;
    }
    ret.first = lossless;
    ret.second = val;
    return ret;
}

void ParseBigIntResultValue(const napi_env env, const napi_value& value, U64ParseResult& u64Ret,
    I64ParseResult& i64Ret)
{
    u64Ret = ParseUnsignedBigIntValue(env, value);
    if (!u64Ret.first) {
        i64Ret = ParseSignedBigIntValue(env, value);
    }
}

std::string GetTagAttribute(const napi_env env, const napi_value& object, std::string defaultValue = "")
{
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, object, TAG_ATTR);
    if (IsValueTypeValid(env, propertyValue, napi_valuetype::napi_null) ||
        IsValueTypeValid(env, propertyValue, napi_valuetype::napi_undefined)) {
        return defaultValue;
    }
    if (IsValueTypeValid(env, propertyValue, napi_valuetype::napi_string)) {
        return ParseStringValue(env, propertyValue, defaultValue);
    }
    NapiHiSysEventUtil::ThrowParamTypeError(env, TAG_ATTR, "string");
    HILOG_ERROR(LOG_CORE, "type of listener tag is not napi_string.");
    return defaultValue;
}

std::string GetStringTypeAttribute(const napi_env env, const napi_value& object,
    const std::string& propertyName, std::string defaultValue = "")
{
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, object, propertyName);
    if (!IsValueTypeValid(env, propertyValue, napi_valuetype::napi_string)) {
        NapiHiSysEventUtil::ThrowParamTypeError(env, propertyName, "string");
        HILOG_DEBUG(LOG_CORE, "type is not napi_string.");
        return defaultValue;
    }
    return ParseStringValue(env, propertyValue, defaultValue);
}

long long GetLonglongTypeAttribute(const napi_env env, const napi_value& val, long long defaultValue = 0)
{
    bool isNumberType = IsValueTypeValid(env, val, napi_valuetype::napi_number);
    bool isBigIntType = IsValueTypeValid(env, val, napi_valuetype::napi_bigint);
    if (!isNumberType && !isBigIntType) {
        HILOG_DEBUG(LOG_CORE, "type is not napi_number or napi_bigint.");
        return defaultValue;
    }
    if (isNumberType) {
        return static_cast<long long>(ParseNumberValue(env, val, defaultValue));
    }
    I64ParseResult ret = ParseSignedBigIntValue(env, val);
    return static_cast<long long>(ret.second);
}

int32_t ParseInt32Value(const napi_env env, const napi_value& value, int32_t defaultValue = 0)
{
    int32_t int32Value = 0;
    napi_status ret = napi_get_value_int32(env, value, &int32Value);
    if (ret != napi_ok) {
        HILOG_DEBUG(LOG_CORE, "failed to parse napi value of number type.");
        return defaultValue;
    }
    return int32Value;
}

int32_t GetInt32TypeAttribute(const napi_env env, const napi_value& object,
    const std::string& propertyName, int32_t defaultValue = 0)
{
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, object, propertyName);
    if (!IsValueTypeValid(env, propertyValue, napi_valuetype::napi_number)) {
        NapiHiSysEventUtil::ThrowParamTypeError(env, propertyName, "number");
        HILOG_DEBUG(LOG_CORE, "type is not napi_number.");
        return defaultValue;
    }
    return ParseInt32Value(env, propertyValue);
}

void AppendBoolArrayData(const napi_env env, HiSysEventInfo& info, const std::string& key,
    const napi_value array, size_t len)
{
    std::vector<bool> values;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of bool array.");
            continue;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_boolean)) {
            values.emplace_back(ParseBoolValue(env, element));
        }
    }
    info.params[key] = values;
}

void AppendNumberArrayData(const napi_env env, HiSysEventInfo& info, const std::string& key,
    const napi_value array, size_t len)
{
    std::vector<ParsedItem> parsedArray;
    ArrayParseStatus pStatus = AS_U64_ARR;
    double parsedVal = 0;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok || !IsValueTypeValid(env, element, napi_valuetype::napi_number)) {
            HILOG_ERROR(LOG_CORE,
                "failed to get the element from array or type not match.");
            continue;
        }
        parsedVal = ParseNumberValue(env, element);
        if (IsFloatingNumber(parsedVal)) {
            pStatus = AS_DOUBLE_ARR;
            parsedArray.emplace_back(parsedVal);
            continue;
        }
        if (parsedVal < 0 && pStatus != AS_DOUBLE_ARR) {
            pStatus = AS_I64_ARR;
            parsedArray.emplace_back(static_cast<int64_t>(parsedVal));
            continue;
        }
        parsedArray.emplace_back(static_cast<uint64_t>(parsedVal));
    }
    if (pStatus == AS_DOUBLE_ARR) {
        AppendParamsToEventInfo<double>(parsedArray, info, key);
    } else if (pStatus == AS_I64_ARR) {
        AppendParamsToEventInfo<int64_t>(parsedArray, info, key);
    } else {
        AppendParamsToEventInfo<uint64_t>(parsedArray, info, key);
    }
}

void AppendBigIntArrayData(const napi_env env, HiSysEventInfo& info, const std::string& key,
    const napi_value array, size_t len)
{
    std::vector<ParsedItem> parsedArray;
    ArrayParseStatus pStatus = AS_U64_ARR;
    napi_value element;
    napi_status status;
    U64ParseResult u64Ret = std::make_pair(false, 0); //default value
    I64ParseResult i64Ret = std::make_pair(false, 0); //default value
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok || !IsValueTypeValid(env, element, napi_valuetype::napi_bigint)) {
            HILOG_ERROR(LOG_CORE,
                "failed to get the element from array or type not match.");
            continue;
        }
        ParseBigIntResultValue(env, element, u64Ret, i64Ret);
        if (u64Ret.first) {
            parsedArray.emplace_back(u64Ret.second);
            continue;
        }
        pStatus = AS_I64_ARR;
        parsedArray.emplace_back(i64Ret.second);
    }
    if (pStatus == AS_I64_ARR) {
        AppendParamsToEventInfo<int64_t>(parsedArray, info, key);
        return;
    }
    AppendParamsToEventInfo<uint64_t>(parsedArray, info, key);
}

void AppendStringArrayData(const napi_env env, HiSysEventInfo& info, const std::string& key,
    const napi_value array, size_t len)
{
    std::vector<std::string> values;
    napi_value element;
    napi_status status;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of string array.");
            continue;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_string)) {
            values.emplace_back(ParseStringValue(env, element));
        }
    }
    info.params[key] = values;
}

void AddArrayParamToEventInfo(const napi_env env, HiSysEventInfo& info, const std::string& key, napi_value& array)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, array, &len);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of param array.");
        return;
    }
    if (len == 0) {
        HILOG_WARN(LOG_CORE, "array is empty.");
        return;
    }
    napi_value firstItem;
    status = napi_get_element(env, array, 0, &firstItem);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get the first element in array.");
        return;
    }
    napi_valuetype type;
    status = napi_typeof(env, firstItem, &type);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get the type of the first element in array.");
        return;
    }
    switch (type) {
        case napi_valuetype::napi_boolean:
            AppendBoolArrayData(env, info, key, array, len);
            HILOG_DEBUG(LOG_CORE, "AppendBoolArrayData: %{public}s.", key.c_str());
            break;
        case napi_valuetype::napi_number:
            AppendNumberArrayData(env, info, key, array, len);
            HILOG_DEBUG(LOG_CORE, "AppendNumberArrayData: %{public}s.", key.c_str());
            break;
        case napi_valuetype::napi_bigint:
            AppendBigIntArrayData(env, info, key, array, len);
            HILOG_DEBUG(LOG_CORE, "AppendBigIntArrayData: %{public}s.", key.c_str());
            break;
        case napi_valuetype::napi_string:
            AppendStringArrayData(env, info, key, array, len);
            HILOG_DEBUG(LOG_CORE, "AppendStringArrayData: %{public}s.", key.c_str());
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
    double parsedVal = 0;
    U64ParseResult u64Ret = std::make_pair(false, 0); //default value
    I64ParseResult i64Ret = std::make_pair(false, 0); //default value
    switch (type) {
        case napi_valuetype::napi_boolean:
            HILOG_DEBUG(LOG_CORE, "AddBoolParamToEventInfo: %{public}s.", key.c_str());
            info.params[key] = ParseBoolValue(env, value);
            break;
        case napi_valuetype::napi_number:
            HILOG_DEBUG(LOG_CORE, "AddNumberParamToEventInfo: %{public}s.", key.c_str());
            parsedVal = ParseNumberValue(env, value);
            if (IsFloatingNumber(parsedVal)) {
                info.params[key] = parsedVal;
            } else if (parsedVal < 0) {
                info.params[key] = static_cast<int64_t>(parsedVal);
            } else {
                info.params[key] = static_cast<uint64_t>(parsedVal);
            }
            break;
        case napi_valuetype::napi_string:
            HILOG_DEBUG(LOG_CORE, "AddStringParamToEventInfo: %{public}s.", key.c_str());
            info.params[key] = ParseStringValue(env, value);
            break;
        case napi_valuetype::napi_bigint:
            HILOG_DEBUG(LOG_CORE, "AddBigIntParamToEventInfo: %{public}s.", key.c_str());
            ParseBigIntResultValue(env, value, u64Ret, i64Ret);
            if (u64Ret.first) {
                info.params[key] = u64Ret.second;
            } else {
                info.params[key] = i64Ret.second;
            }
            break;
        default:
            break;
    }
}

void GetObjectTypeAttribute(const napi_env env, const napi_value& object,
    const std::string& propertyName, HiSysEventInfo& info)
{
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, object, propertyName);
    if (!IsValueTypeValid(env, propertyValue, napi_valuetype::napi_object)) {
        HILOG_DEBUG(LOG_CORE, "type is not napi_object.");
        return;
    }
    napi_value keyArr = nullptr;
    napi_status status = napi_get_property_names(env, propertyValue, &keyArr);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to parse property names of a js object.");
        return;
    }
    uint32_t len = 0;
    status = napi_get_array_length(env, keyArr, &len);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of the key-value pairs.");
        return;
    }
    for (uint32_t i = 0; i < len; i++) {
        napi_value key = nullptr;
        napi_get_element(env, keyArr, i, &key);
        if (!IsValueTypeValid(env, key, napi_valuetype::napi_string)) {
            HILOG_WARN(LOG_CORE, "this param would be discarded because of invalid type of the key.");
            continue;
        }
        char buf[BUF_SIZE] = {0};
        size_t valueLen = 0;
        napi_get_value_string_utf8(env, key, buf, BUF_SIZE - 1, &valueLen);
        if (!CheckKeyTypeString(buf)) {
            HILOG_WARN(LOG_CORE, "this param would be discarded because of invalid format of the key.");
            continue;
        }
        napi_value val = NapiHiSysEventUtil::GetPropertyByName(env, propertyValue, buf);
        AddParamToEventInfo(env, info, buf, val);
    }
}

void ParseStringArray(const napi_env env, napi_value& arrayValue, std::vector<std::string>& arrayDest)
{
    if (!CheckValueIsArray(env, arrayValue)) {
        HILOG_ERROR(LOG_CORE, "try to parse a array from a napi value without array type");
        return;
    }
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, arrayValue, &len);
    if (status != napi_ok) {
        return;
    }
    napi_value element;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, arrayValue, i, &element);
        if (status != napi_ok) {
            return;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_string)) {
            std::string str = ParseStringValue(env, element);
            arrayDest.emplace_back(str);
        }
    }
}

ListenerRule ParseListenerRule(const napi_env env, const napi_value& jsObj)
{
    if (!IsValueTypeValid(env, jsObj, napi_valuetype::napi_object)) {
        return ListenerRule("", RuleType::WHOLE_WORD);
    }
    std::string domain = GetStringTypeAttribute(env, jsObj, NapiHiSysEventUtil::DOMAIN_ATTR);
    HILOG_DEBUG(LOG_CORE, "domain is %{public}s.", domain.c_str());
    std::string name = GetStringTypeAttribute(env, jsObj, NapiHiSysEventUtil::NAME_ATTR);
    HILOG_DEBUG(LOG_CORE, "name is %{public}s.", name.c_str());
    int32_t ruleType = GetInt32TypeAttribute(env, jsObj, RULE_TYPE_ATTR, RuleType::WHOLE_WORD);
    HILOG_DEBUG(LOG_CORE, "ruleType is %{public}d.", ruleType);
    std::string tag = GetTagAttribute(env, jsObj);
    HILOG_DEBUG(LOG_CORE, "tag is %{public}s.", tag.c_str());
    return ListenerRule(domain, name, tag, RuleType(ruleType));
}

bool IsQueryRuleValid(const napi_env env, const QueryRule& rule)
{
    auto domain = rule.GetDomain();
    if (!StringFilter::GetInstance().IsValidName(domain, MAX_DOMAIN_LENGTH)) {
        NapiHiSysEventUtil::ThrowErrorByRet(env, NapiInnerError::ERR_INVALID_DOMAIN_IN_QUERY_RULE);
        return false;
    }
    auto names = rule.GetEventList();
    if (std::any_of(names.begin(), names.end(), [] (auto& name) {
        return !StringFilter::GetInstance().IsValidName(name, MAX_EVENT_NAME_LENGTH);
    })) {
        NapiHiSysEventUtil::ThrowErrorByRet(env, NapiInnerError::ERR_INVALID_EVENT_NAME_IN_QUERY_RULE);
        return false;
    }
    return true;
}

QueryRule ParseQueryRule(const napi_env env, napi_value& jsObj)
{
    std::vector<std::string> names;
    if (!IsValueTypeValid(env, jsObj, napi_valuetype::napi_object)) {
        return QueryRule("", names);
    }
    std::string domain = GetStringTypeAttribute(env, jsObj, NapiHiSysEventUtil::DOMAIN_ATTR);
    HILOG_DEBUG(LOG_CORE, "domain is %{public}s.", domain.c_str());
    napi_value propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, jsObj, NAMES_ATTR);
    ParseStringArray(env, propertyValue, names);
    propertyValue = NapiHiSysEventUtil::GetPropertyByName(env, jsObj, CONDITION_ATTR);
    std::string condition = ParseStringValue(env, propertyValue);
    HILOG_DEBUG(LOG_CORE, "condition is %{public}s.", condition.c_str());
    return QueryRule(domain, names, RuleType::WHOLE_WORD, 0, condition);
}

void SetNamedProperty(const napi_env env, napi_value& object, const std::string& propertyName,
    napi_value& propertyValue)
{
    napi_status status = napi_set_named_property(env, object, propertyName.c_str(), propertyValue);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "set property %{public}s failed.", propertyName.c_str());
    }
}

bool IsBaseInfoKey(const std::string& propertyName)
{
    return propertyName == DOMAIN__KEY || propertyName == NAME__KEY || propertyName == TYPE__KEY;
}

std::string TranslateKeyToAttrName(const std::string& key)
{
    if (key == DOMAIN__KEY) {
        return NapiHiSysEventUtil::DOMAIN_ATTR;
    }
    if (key == NAME__KEY) {
        return NapiHiSysEventUtil::NAME_ATTR;
    }
    if (key == TYPE__KEY) {
        return NapiHiSysEventUtil::EVENT_TYPE_ATTR;
    }
    return "";
}

void AppendBaseInfo(const napi_env env, napi_value& sysEventInfo, const std::string& key, Json::Value& value)
{
    if ((key == DOMAIN__KEY || key == NAME__KEY) && value.isString()) {
        NapiHiSysEventUtil::AppendStringPropertyToJsObject(env, TranslateKeyToAttrName(key),
            value.asString(), sysEventInfo);
    }
    if (key == TYPE__KEY && value.isInt()) {
        NapiHiSysEventUtil::AppendInt32PropertyToJsObject(env, TranslateKeyToAttrName(key),
            static_cast<int32_t>(value.asInt()), sysEventInfo);
    }
}

void CreateBoolValue(const napi_env env, bool value, napi_value& val)
{
    napi_status status = napi_get_boolean(env, value, &val);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get create napi value of bool type.");
    }
}

void CreateDoubleValue(const napi_env env, double value, napi_value& val)
{
    napi_status status = napi_create_double(env, value, &val);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get create napi value of double type.");
    }
}

void CreateUint32Value(const napi_env env, uint32_t value, napi_value& val)
{
    napi_status status = napi_create_uint32(env, value, &val);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get create napi value of uint32 type.");
    }
}

bool CreateParamItemTypeValue(const napi_env env, Json::Value& jsonValue, napi_value& value)
{
    if (jsonValue.isBool()) {
        CreateBoolValue(env, jsonValue.asBool(), value);
        return true;
    }
    if (jsonValue.isInt()) {
        NapiHiSysEventUtil::CreateInt32Value(env, static_cast<int32_t>(jsonValue.asInt()), value);
        return true;
    }
    if (jsonValue.isUInt()) {
        CreateUint32Value(env, static_cast<uint32_t>(jsonValue.asUInt()), value);
        return true;
    }
#ifdef JSON_HAS_INT64
    if (jsonValue.isInt64() && jsonValue.type() != Json::ValueType::uintValue) {
        NapiHiSysEventUtil::CreateInt64Value(env, jsonValue.asInt64(), value);
        return true;
    }
    if (jsonValue.isUInt64() && jsonValue.type() != Json::ValueType::intValue) {
        NapiHiSysEventUtil::CreateUInt64Value(env, jsonValue.asUInt64(), value);
        return true;
    }
#endif
    if (jsonValue.isDouble()) {
        CreateDoubleValue(env, jsonValue.asDouble(), value);
        return true;
    }
    if (jsonValue.isString()) {
        NapiHiSysEventUtil::CreateStringValue(env, jsonValue.asString(), value);
        return true;
    }
    return false;
}

void AppendArrayParams(const napi_env env, napi_value& params, const std::string& key, Json::Value& value)
{
    size_t len = value.size();
    napi_value array = nullptr;
    napi_create_array_with_length(env, len, &array);
    for (size_t i = 0; i < len; i++) {
        napi_value item;
        if (!CreateParamItemTypeValue(env, value[static_cast<int>(i)], item)) {
            continue;
        }
        napi_set_element(env, array, i, item);
    }
    SetNamedProperty(env, params, key, array);
}

void AppendParamsInfo(const napi_env env, napi_value& params, const std::string& key, Json::Value& jsonValue)
{
    if (jsonValue.isArray()) {
        AppendArrayParams(env, params, key, jsonValue);
        return;
    }
    napi_value property = nullptr;
    if (!CreateParamItemTypeValue(env, jsonValue, property)) {
        return;
    }
    SetNamedProperty(env, params, key, property);
}
}

napi_value NapiHiSysEventUtil::GetPropertyByName(const napi_env env, const napi_value& object,
    const std::string& propertyName)
{
    napi_value result = nullptr;
    napi_status status = napi_get_named_property(env, object, propertyName.c_str(), &result);
    if (status != napi_ok) {
        HILOG_DEBUG(LOG_CORE, "failed to parse property named %{public}s from JS object.", propertyName.c_str());
    }
    return result;
}

void NapiHiSysEventUtil::ParseHiSysEventInfo(const napi_env env, napi_value* param,
    size_t paramNum, HiSysEventInfo& info)
{
    if (paramNum <= SYS_EVENT_INFO_PARAM_INDEX) {
        return;
    }
    if (!IsValueTypeValid(env, param[SYS_EVENT_INFO_PARAM_INDEX], napi_valuetype::napi_object)) {
        NapiHiSysEventUtil::ThrowParamTypeError(env, "info", "object");
        return;
    }
    info.domain = GetStringTypeAttribute(env, param[SYS_EVENT_INFO_PARAM_INDEX], NapiHiSysEventUtil::DOMAIN_ATTR);
    HILOG_DEBUG(LOG_CORE, "domain is %{public}s.", info.domain.c_str());
    info.name = GetStringTypeAttribute(env, param[SYS_EVENT_INFO_PARAM_INDEX], NapiHiSysEventUtil::NAME_ATTR);
    HILOG_DEBUG(LOG_CORE, "name is %{public}s.", info.name.c_str());
    info.eventType = HiSysEvent::EventType(GetInt32TypeAttribute(env,
        param[SYS_EVENT_INFO_PARAM_INDEX], EVENT_TYPE_ATTR, HiSysEvent::EventType::FAULT));
    HILOG_DEBUG(LOG_CORE, "eventType is %{public}d.", info.eventType);
    GetObjectTypeAttribute(env, param[SYS_EVENT_INFO_PARAM_INDEX], PARAMS_ATTR, info);
}

bool NapiHiSysEventUtil::HasStrParamLenOverLimit(HiSysEventInfo& info)
{
    return any_of(info.params.begin(), info.params.end(), [] (auto& item) {
        if (item.second.index() != STR && item.second.index() != STR_ARR) {
            return false;
        }
        if (item.second.index() == STR) {
            return std::get<STR>(item.second).size() > JS_STR_PARM_LEN_LIMIT;
        }
        auto allStr = std::get<STR_ARR>(item.second);
        return any_of(allStr.begin(), allStr.end(), [] (auto& item) {
            return item.size() > JS_STR_PARM_LEN_LIMIT;
        });
    });
}

void NapiHiSysEventUtil::CreateHiSysEventInfoJsObject(const napi_env env, const std::string& jsonStr,
    napi_value& sysEventInfo)
{
    Json::Value eventJson;
#ifdef JSONCPP_VERSION_STRING
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    std::unique_ptr<Json::CharReader> const reader(jsonRBuilder.newCharReader());
    JSONCPP_STRING errs;
    if (!reader->parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), &eventJson, &errs)) {
#else
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(jsonStr, eventJson)) {
#endif
        HILOG_ERROR(LOG_CORE, "parse event detail info failed, please check the style of json infomation: %{public}s",
            jsonStr.c_str());
        return;
    }
    if (!eventJson.isObject()) {
        HILOG_ERROR(LOG_CORE, "event json parsed isn't a json object");
        return;
    }
    napi_create_object(env, &sysEventInfo);
    napi_value params = nullptr;
    napi_create_object(env, &params);
    auto eventNameList = eventJson.getMemberNames();
    for (auto it = eventNameList.cbegin(); it != eventNameList.cend(); it++) {
        auto propertyName = *it;
        if (IsBaseInfoKey(propertyName)) {
            AppendBaseInfo(env, sysEventInfo, propertyName, eventJson[propertyName]);
        } else {
            AppendParamsInfo(env, params, propertyName, eventJson[propertyName]);
        }
    }
    SetNamedProperty(env, sysEventInfo, PARAMS_ATTR, params);
}

void NapiHiSysEventUtil::CreateJsSysEventInfoArray(const napi_env env, const std::vector<std::string>& originValues,
    napi_value& array)
{
    auto len = originValues.size();
    for (size_t i = 0; i < len; i++) {
        napi_value item;
        CreateHiSysEventInfoJsObject(env, originValues[i], item);
        napi_status status = napi_set_element(env, array, i, item);
        if (status != napi_ok) {
            HILOG_DEBUG(LOG_CORE, "napi_set_element failed");
        }
    }
}

void NapiHiSysEventUtil::AppendStringPropertyToJsObject(const napi_env env, const std::string& key,
    const std::string& value, napi_value& jsObj)
{
    napi_value property = nullptr;
    NapiHiSysEventUtil::CreateStringValue(env, value, property);
    SetNamedProperty(env, jsObj, key, property);
}

void NapiHiSysEventUtil::AppendInt32PropertyToJsObject(const napi_env env, const std::string& key,
    const int32_t& value, napi_value& jsObj)
{
    napi_value property = nullptr;
    NapiHiSysEventUtil::CreateInt32Value(env, value, property);
    SetNamedProperty(env, jsObj, key, property);
}

int32_t NapiHiSysEventUtil::ParseListenerRules(const napi_env env, napi_value& array,
    std::vector<ListenerRule>& listenerRules)
{
    if (!CheckValueIsArray(env, array)) {
        ThrowParamTypeError(env, "rules", "array");
        return ERR_LISTENER_RULES_TYPE_NOT_ARRAY;
    }
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, array, &len);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_value firstItem;
    status = napi_get_element(env, array, 0, &firstItem);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_valuetype type;
    status = napi_typeof(env, firstItem, &type);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_value element;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            return ERR_NAPI_PARSED_FAILED;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_object)) {
            listenerRules.emplace_back(ParseListenerRule(env, element));
        }
    }
    return NAPI_SUCCESS;
}

int32_t NapiHiSysEventUtil::ParseQueryRules(const napi_env env, napi_value& array, std::vector<QueryRule>& queryRules)
{
    if (!CheckValueIsArray(env, array)) {
        ThrowParamTypeError(env, "rules", "array");
        return ERR_QUERY_RULES_TYPE_NOT_ARRAY;
    }
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, array, &len);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_value firstItem;
    status = napi_get_element(env, array, 0, &firstItem);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_valuetype type;
    status = napi_typeof(env, firstItem, &type);
    if (status != napi_ok) {
        return ERR_NAPI_PARSED_FAILED;
    }
    napi_value element;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, array, i, &element);
        if (status != napi_ok) {
            return ERR_NAPI_PARSED_FAILED;
        }
        if (IsValueTypeValid(env, element, napi_valuetype::napi_object)) {
            auto queryRule = ParseQueryRule(env, element);
            if (IsQueryRuleValid(env, queryRule)) {
                queryRules.emplace_back(queryRule);
            }
        }
    }
    return NAPI_SUCCESS;
}

int32_t NapiHiSysEventUtil::ParseQueryArg(const napi_env env, napi_value& jsObj, QueryArg& queryArg)
{
    if (!IsValueTypeValid(env, jsObj, napi_valuetype::napi_object)) {
        ThrowParamTypeError(env, "queryArg", "object");
        return ERR_QUERY_ARG_TYPE_INVALID;
    }
    auto beginTimeVal = NapiHiSysEventUtil::GetPropertyByName(env, jsObj, BEGIN_TIME_ATTR);
    auto endTimeVal = NapiHiSysEventUtil::GetPropertyByName(env, jsObj, END_TIME_ATTR);
    auto fromSeqVal = NapiHiSysEventUtil::GetPropertyByName(env, jsObj, BEGIN_SEQ_ATTR);
    auto toSeqVal = NapiHiSysEventUtil::GetPropertyByName(env, jsObj, END_SEQ_ATTR);
    if ((IsNullOrUndefined(env, beginTimeVal) || IsNullOrUndefined(env, endTimeVal)) &&
        (IsNullOrUndefined(env, fromSeqVal) || IsNullOrUndefined(env, toSeqVal))) {
        ThrowParamTypeError(env, "queryArg's member", "not null or undefined");
        return ERR_QUERY_ARG_TYPE_INVALID;
    }

    auto beginTime = GetLonglongTypeAttribute(env, beginTimeVal, DEFAULT_TIME_STAMP);
    queryArg.beginTime = beginTime < 0 ? 0 : beginTime;
    auto endTime = GetLonglongTypeAttribute(env, endTimeVal, DEFAULT_TIME_STAMP);
    queryArg.endTime = endTime < 0 ? std::numeric_limits<long long>::max() : endTime;
    queryArg.fromSeq = GetLonglongTypeAttribute(env, fromSeqVal, DEFAULT_SEQ);
    queryArg.toSeq = GetLonglongTypeAttribute(env, toSeqVal, DEFAULT_SEQ);

    auto maxEvents = GetInt32TypeAttribute(env, jsObj, MAX_EVENTS_ATTR, DEFAULT_MAX_EVENTS);
    queryArg.maxEvents = maxEvents < 0 ? std::numeric_limits<int>::max() : maxEvents;

    return NAPI_SUCCESS;
}

void NapiHiSysEventUtil::CreateNull(const napi_env env, napi_value& ret)
{
    napi_status status = napi_get_null(env, &ret);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create napi value of null.");
    }
}

void NapiHiSysEventUtil::CreateInt32Value(const napi_env env, int32_t value, napi_value& ret)
{
    napi_status status = napi_create_int32(env, value, &ret);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create napi value of int32 type.");
    }
}

void NapiHiSysEventUtil::CreateInt64Value(const napi_env env, int64_t value, napi_value& ret)
{
    napi_status status = napi_create_bigint_int64(env, value, &ret);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create napi value of int64_t type.");
    }
}

void NapiHiSysEventUtil::CreateUInt64Value(const napi_env env, uint64_t value, napi_value& ret)
{
    napi_status status = napi_create_bigint_uint64(env, value, &ret);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create napi value of uint64_t type.");
    }
}

void NapiHiSysEventUtil::CreateStringValue(const napi_env env, std::string value, napi_value& ret)
{
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &ret);
    if (status != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create napi value of string type.");
    }
}

void NapiHiSysEventUtil::ThrowParamMandatoryError(napi_env env, const std::string paramName)
{
    ThrowError(env, NapiError::ERR_PARAM_CHECK, "Parameter error. The " + paramName + " parameter is mandatory.");
}

void NapiHiSysEventUtil::ThrowParamTypeError(napi_env env, const std::string paramName, std::string paramType)
{
    ThrowError(env, NapiError::ERR_PARAM_CHECK, "Parameter error. The type of " + paramName + " must be "
        + paramType + ".");
}

void NapiHiSysEventUtil::ThrowSystemAppPermissionError(napi_env env)
{
    ThrowError(env, NapiError::ERR_NON_SYS_APP_PERMISSION, "Permission denied. "
        "System api can be invoked only by system applications.");
}

napi_value NapiHiSysEventUtil::CreateError(napi_env env, int32_t code, const std::string& msg)
{
    napi_value err = nullptr;
    napi_value napiCode = nullptr;
    NapiHiSysEventUtil::CreateStringValue(env, std::to_string(code), napiCode);
    napi_value napiStr = nullptr;
    NapiHiSysEventUtil::CreateStringValue(env, msg, napiStr);
    if (napi_create_error(env, napiCode, napiStr, &err) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create napi error");
    }
    return err;
}

void NapiHiSysEventUtil::ThrowError(napi_env env, const int32_t code, const std::string& msg)
{
    if (napi_throw_error(env, std::to_string(code).c_str(), msg.c_str()) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to throw err, code=%{public}d, msg=%{public}s.", code, msg.c_str());
    }
}

std::pair<int32_t, std::string> NapiHiSysEventUtil::GetErrorDetailByRet(napi_env env, const int32_t retCode)
{
    HILOG_DEBUG(LOG_CORE, "original result code is %{public}d.", retCode);
    const std::unordered_map<int32_t, std::pair<int32_t, std::string>> errMap = {
        // common
        {ERR_NO_PERMISSION, {NapiError::ERR_PERMISSION_CHECK,
            "Permission denied. An attempt was made to read sysevent forbidden"
            " by permission: ohos.permission.READ_DFX_SYSEVENT."}},
        // write refer
        {ERR_DOMAIN_NAME_INVALID, {NapiError::ERR_INVALID_DOMAIN, "Invalid event domain"}},
        {ERR_EVENT_NAME_INVALID, {NapiError::ERR_INVALID_EVENT_NAME, "Invalid event name"}},
        {ERR_DOES_NOT_INIT, {NapiError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_OVER_SIZE, {NapiError::ERR_CONTENT_OVER_LIMIT, "The event length exceeds the limit"}},
        {ERR_KEY_NAME_INVALID, {NapiError::ERR_INVALID_PARAM_NAME, "Invalid event parameter"}},
        {ERR_VALUE_LENGTH_TOO_LONG, {NapiError::ERR_STR_LEN_OVER_LIMIT,
            "The size of the event parameter of the string type exceeds the limit"}},
        {ERR_KEY_NUMBER_TOO_MUCH, {NapiError::ERR_PARAM_COUNT_OVER_LIMIT,
            "The number of event parameters exceeds the limit"}},
        {ERR_ARRAY_TOO_MUCH, {NapiError::ERR_ARRAY_SIZE_OVER_LIMIT,
            "The number of event parameters of the array type exceeds the limit"}},
        // ipc common
        {ERR_SYS_EVENT_SERVICE_NOT_FOUND, {NapiError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_CAN_NOT_WRITE_DESCRIPTOR, {NapiError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_CAN_NOT_WRITE_PARCEL, {NapiError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_CAN_NOT_WRITE_REMOTE_OBJECT, {NapiError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_CAN_NOT_SEND_REQ, {NapiError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_CAN_NOT_READ_PARCEL, {NapiError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_SEND_FAIL, {NapiError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        // add watcher
        {ERR_TOO_MANY_WATCHERS, {NapiError::ERR_WATCHER_COUNT_OVER_LIMIT,
            "The number of watchers exceeds the limit"}},
        {ERR_TOO_MANY_WATCH_RULES, {NapiError::ERR_WATCH_RULE_COUNT_OVER_LIMIT,
            "The number of watch rules exceeds the limit"}},
        // remove watcher
        {ERR_LISTENER_NOT_EXIST, {NapiError::ERR_WATCHER_NOT_EXIST, "The watcher does not exist"}},
        {ERR_NAPI_LISTENER_NOT_FOUND, {NapiError::ERR_WATCHER_NOT_EXIST, "The watcher does not exist"}},
        // query refer
        {ERR_TOO_MANY_QUERY_RULES, {NapiError::ERR_QUERY_RULE_COUNT_OVER_LIMIT,
            "The number of query rules exceeds the limit"}},
        {ERR_TOO_MANY_CONCURRENT_QUERIES, {NapiError::ERR_CONCURRENT_QUERY_COUNT_OVER_LIMIT,
            "The number of concurrent queries exceeds the limit"}},
        {ERR_QUERY_TOO_FREQUENTLY, {NapiError::ERR_QUERY_TOO_FREQUENTLY, "The query frequency exceeds the limit"}},
        {NapiInnerError::ERR_INVALID_DOMAIN_IN_QUERY_RULE,
            {NapiError::ERR_INVALID_QUERY_RULE, "Invalid query rule"}},
        {ERR_QUERY_RULE_INVALID, {NapiError::ERR_INVALID_QUERY_RULE, "Invalid query rule"}},
        {NapiInnerError::ERR_INVALID_EVENT_NAME_IN_QUERY_RULE,
            {NapiError::ERR_INVALID_QUERY_RULE, "Invalid query rule"}},
        // export
        {ERR_EXPORT_FREQUENCY_OVER_LIMIT, {NapiError::ERR_QUERY_TOO_FREQUENTLY,
            "The query frequency exceeds the limit"}},
        // add subscriber
        {ERR_TOO_MANY_EVENTS, {NapiError::ERR_QUERY_RULE_COUNT_OVER_LIMIT,
            "The number of query rules exceeds the limit"}},
        // remove subscriber
        {ERR_REMOVE_SUBSCRIBE, {NapiError::ERR_REMOVE_SUBSCRIBE, "Unsubscription failed"}},
    };
    return errMap.find(retCode) == errMap.end() ?
        std::make_pair(NapiError::ERR_ENV_ABNORMAL, "Abnormal environment") : errMap.at(retCode);
}

napi_value NapiHiSysEventUtil::CreateErrorByRet(napi_env env, const int32_t retCode)
{
    auto detail = GetErrorDetailByRet(env, retCode);
    return CreateError(env, detail.first, detail.second);
}

void NapiHiSysEventUtil::ThrowErrorByRet(napi_env env, const int32_t retCode)
{
    auto detail = GetErrorDetailByRet(env, retCode);
    ThrowError(env, detail.first, detail.second);
}

bool NapiHiSysEventUtil::IsSystemAppCall()
{
    uint64_t tokenId = IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

bool NapiHiSysEventUtil::IsNullOrUndefined(napi_env env, const napi_value& val)
{
    return IsValueTypeValid(env, val, napi_valuetype::napi_null) ||
        IsValueTypeValid(env, val, napi_valuetype::napi_undefined);
}
} // namespace HiviewDFX
} // namespace OHOS