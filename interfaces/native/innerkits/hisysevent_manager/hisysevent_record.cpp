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

#include "hisysevent_record.h"

#include <sstream>

#include "hilog/log.h"
#include "hisysevent_value.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_RECORD"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int DEFAULT_INT_VAL = 0;
constexpr uint64_t DEFAULT_UINT64_VAL = 0;
constexpr int64_t DEFAULT_INT64_VAL = 0;
constexpr double DEFAULT_DOUBLE_VAL = 0.0;
constexpr double DOUBLE_CONVERT_FACTOR = 2.0;
constexpr Json::UInt64 BIT = 2;
constexpr Json::UInt64 BIT_AND_VAL = 1;

#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
template <typename T, typename U>
static inline bool InValidRange(double d, T min, U max)
{
    return d >= static_cast<double>(min) && d <= static_cast<double>(max);
}
#else
static inline double int64ToDouble(Json::UInt64 value)
{
    return static_cast<double>(Json::Int64(value / BIT)) * DOUBLE_CONVERT_FACTOR +
        static_cast<double>(Json::Int64(value & BIT_AND_VAL));
}
template <typename T> static inline double int64ToDouble(T value)
{
    return static_cast<double>(value);
}
template <typename T, typename U>
static inline bool InValidRange(double d, T min, U max)
{
    return d >= int64ToDouble(min) && d <= int64ToDouble(max);
}
#endif
}

std::string HiSysEventRecord::GetDomain() const
{
    return GetStringValueByKey("domain_");
}

std::string HiSysEventRecord::GetEventName() const
{
    return GetStringValueByKey("name_");
}

HiSysEvent::EventType HiSysEventRecord::GetEventType() const
{
    return HiSysEvent::EventType(GetIntValueByKey("type_"));
}

uint64_t HiSysEventRecord::GetTime() const
{
    return GetUInt64ValueByKey("time_");
}

std::string HiSysEventRecord::GetTimeZone() const
{
    return GetStringValueByKey("tz_");
}

int64_t HiSysEventRecord::GetPid() const
{
    return GetInt64ValueByKey("pid_");
}

int64_t HiSysEventRecord::GetTid() const
{
    return GetInt64ValueByKey("tid_");
}

int64_t HiSysEventRecord::GetUid() const
{
    return GetInt64ValueByKey("uid_");
}

uint64_t HiSysEventRecord::GetTraceId() const
{
    std::string hexStr = GetStringValueByKey("traceid_");
    uint64_t traceId = 0; // default trace id is 0
    std::stringstream ss;
    ss << hexStr;
    ss >> std::hex >> traceId;
    return traceId;
}

uint64_t HiSysEventRecord::GetSpanId() const
{
    return GetUInt64ValueByKey("spanid_");
}

uint64_t HiSysEventRecord::GetPspanId() const
{
    return GetUInt64ValueByKey("pspanid_");
}

int HiSysEventRecord::GetTraceFlag() const
{
    return static_cast<int>(GetInt64ValueByKey("trace_flag_"));
}

std::string HiSysEventRecord::GetLevel() const
{
    return GetStringValueByKey("level_");
}

std::string HiSysEventRecord::GetTag() const
{
    return GetStringValueByKey("tag_");
}

void HiSysEventRecord::GetParamNames(std::vector<std::string>& params) const
{
    jsonVal_->GetParamNames(params);
}

std::string HiSysEventRecord::AsJson() const
{
    return jsonStr_;
}

int HiSysEventRecord::GetIntValueByKey(const std::string key) const
{
    return static_cast<int>(GetInt64ValueByKey(key));
}

int64_t HiSysEventRecord::GetInt64ValueByKey(const std::string key) const
{
    int64_t value = 0;
    (void)GetParamValue(key, value);
    return value;
}

uint64_t HiSysEventRecord::GetUInt64ValueByKey(const std::string key) const
{
    uint64_t value = 0;
    (void)GetParamValue(key, value);
    return value;
}

std::string HiSysEventRecord::GetStringValueByKey(const std::string key) const
{
    std::string value;
    (void)GetParamValue(key, value);
    return value;
}

int HiSysEventRecord::GetParamValue(const std::string& param, int64_t& value) const
{
    return GetParamValue(param,
        [this] (JsonValue val) {
            return !(this->IsInt64ValueType(val));
        },
        [&value] (JsonValue src) {
            value = src->AsInt64();
        });
}

int HiSysEventRecord::GetParamValue(const std::string& param, uint64_t& value) const
{
    return GetParamValue(param,
        [this] (JsonValue val) {
            return !(this->IsUInt64ValueType(val));
        },
        [&value] (JsonValue src) {
            value = src->AsUInt64();
        });
}

int HiSysEventRecord::GetParamValue(const std::string& param, double& value) const
{
    return GetParamValue(param,
        [this] (JsonValue val) {
            return !(this->IsDoubleValueType(val));
        },
        [&value] (JsonValue src) {
            value = src->AsDouble();
        });
}

int HiSysEventRecord::GetParamValue(const std::string& param, std::string& value) const
{
    return GetParamValue(param,
        [this] (JsonValue val) {
            return !(this->IsStringValueType(val));
        },
        [&value] (JsonValue src) {
            value = src->AsString();
        });
}

int HiSysEventRecord::GetParamValue(const std::string& param, std::vector<int64_t>& value) const
{
    return GetParamValue(param,
        [this] (JsonValue val) {
            return !(this->IsArray(val, [this] (const JsonValue val) {
                    return this->IsInt64ValueType(val);
                }));
        },
        [&value] (JsonValue src) {
            int arraySize = src->Size();
            for (int i = 0; i < arraySize; i++) {
                value.emplace_back(src->Index(i).asInt64());
            }
        });
}

int HiSysEventRecord::GetParamValue(const std::string& param, std::vector<uint64_t>& value) const
{
    return GetParamValue(param,
        [this] (JsonValue val) {
            return !(this->IsArray(val, [this] (const JsonValue val) {
                    return this->IsUInt64ValueType(val);
                }));
        },
        [&value] (JsonValue src) {
            int arraySize = src->Size();
            for (int i = 0; i < arraySize; i++) {
                value.emplace_back(src->Index(i).asUInt64());
            }
        });
}

int HiSysEventRecord::GetParamValue(const std::string& param, std::vector<double>& value) const
{
    return GetParamValue(param,
        [this] (JsonValue val) {
            return !(this->IsArray(val, [this] (const JsonValue val) {
                    return this->IsDoubleValueType(val);
                }));
        },
        [&value] (JsonValue src) {
            int arraySize = src->Size();
            for (int i = 0; i < arraySize; i++) {
                value.emplace_back(src->Index(i).asDouble());
            }
        });
}

int HiSysEventRecord::GetParamValue(const std::string& param, std::vector<std::string>& value) const
{
    return GetParamValue(param,
        [this] (JsonValue val) {
            return !(this->IsArray(val, [this] (const JsonValue val) {
                    return this->IsStringValueType(val);
                }));
        },
        [&value] (JsonValue src) {
            int arraySize = src->Size();
            for (int i = 0; i < arraySize; i++) {
                value.emplace_back(src->Index(i).asString());
            }
        });
}

void HiSysEventRecord::ParseJsonStr(const std::string jsonStr)
{
    jsonVal_ = std::make_shared<HiSysEventValue>(jsonStr);
    jsonStr_ = jsonStr;
}

int HiSysEventRecord::GetParamValue(const std::string& param, const TypeFilter filterFunc,
    const ValueAssigner assignFunc) const
{
    if (!jsonVal_->HasInitialized()) {
        HILOG_DEBUG(LOG_CORE, "this hisysevent record is not initialized");
        return ERR_INIT_FAILED;
    }
    if (!jsonVal_->IsMember(param)) {
        HILOG_DEBUG(LOG_CORE, "key named \"%{public}s\" is not found in json.",
            param.c_str());
        return ERR_KEY_NOT_EXIST;
    }
    auto parsedVal = std::make_shared<HiSysEventValue>(jsonVal_->GetParamValue(param));
    if (filterFunc(parsedVal)) {
        HILOG_DEBUG(LOG_CORE, "value type with key named \"%{public}s\" is %{public}d, not match.",
            param.c_str(), parsedVal->Type());
        return ERR_TYPE_NOT_MATCH;
    }
    assignFunc(parsedVal);
    return VALUE_PARSED_SUCCEED;
}

bool HiSysEventRecord::IsInt64ValueType(const JsonValue val) const
{
    return val->IsInt64() || val->IsNull() || val->IsBool();
}

bool HiSysEventRecord::IsUInt64ValueType(const JsonValue val) const
{
    return val->IsUInt64() || val->IsNull() || val->IsBool();
}

bool HiSysEventRecord::IsDoubleValueType(const JsonValue val) const
{
    return val->IsDouble() || val->IsNull() || val->IsBool();
}

bool HiSysEventRecord::IsStringValueType(const JsonValue val) const
{
    return val->IsNull() || val->IsBool() || val->IsNumeric() || val->IsString();
}

bool HiSysEventRecord::IsArray(const JsonValue val, const TypeFilter filterFunc) const
{
    if (!val->IsArray()) {
        return false;
    }
    if (val->Size() > 0) {
        return filterFunc(std::make_shared<HiSysEventValue>(val->Index(0)));
    }
    return (val->Size() == 0);
}

void HiSysEventValue::ParseJsonStr(const std::string jsonStr)
{
#ifdef JSONCPP_VERSION_STRING
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    std::unique_ptr<Json::CharReader> const reader(jsonRBuilder.newCharReader());
    JSONCPP_STRING errs;
    if (!reader->parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), &jsonVal_, &errs)) {
#else
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(jsonStr, jsonVal_)) {
#endif
        HILOG_ERROR(LOG_CORE, "parse json file failed, please check the style of json string: %{public}s.",
            jsonStr.c_str());
        return;
    }
    hasInitialized_ = true;
}

bool HiSysEventValue::HasInitialized() const
{
    return hasInitialized_;
}

void HiSysEventValue::GetParamNames(std::vector<std::string>& params) const
{
    if (!hasInitialized_ || (jsonVal_.type() != Json::ValueType::nullValue &&
        jsonVal_.type() != Json::ValueType::objectValue)) {
        return;
    }
    params = jsonVal_.getMemberNames();
}

bool HiSysEventValue::IsArray() const
{
    if (!hasInitialized_) {
        return false;
    }
    return jsonVal_.isArray();
}

bool HiSysEventValue::IsMember(const std::string key) const
{
    if (!hasInitialized_ || !jsonVal_.isObject()) {
        return false;
    }
    return jsonVal_.isMember(key);
}

bool HiSysEventValue::IsInt64() const
{
    if (!hasInitialized_) {
        return false;
    }
    return jsonVal_.isInt64();
}

bool HiSysEventValue::IsUInt64() const
{
    if (!hasInitialized_) {
        return false;
    }
    return jsonVal_.isUInt64();
}

bool HiSysEventValue::IsDouble() const
{
    if (!hasInitialized_) {
        return false;
    }
    return jsonVal_.isDouble();
}

bool HiSysEventValue::IsString() const
{
    if (!hasInitialized_) {
        return false;
    }
    return jsonVal_.isString();
}

bool HiSysEventValue::IsBool() const
{
    if (!hasInitialized_) {
        return false;
    }
    return jsonVal_.isBool();
}

bool HiSysEventValue::IsNull() const
{
    if (!hasInitialized_) {
        return false;
    }
    return jsonVal_.isNull();
}

bool HiSysEventValue::IsNumeric() const
{
    if (!hasInitialized_) {
        return false;
    }
    return jsonVal_.isNumeric();
}

Json::Value HiSysEventValue::Index(const int index) const
{
    if (!hasInitialized_ || index < 0 ||
        (jsonVal_.type() != Json::ValueType::nullValue &&
        jsonVal_.type() != Json::ValueType::arrayValue)) {
        return Json::Value(Json::ValueType::nullValue);
    }
    return jsonVal_[index];
}
Json::Value HiSysEventValue::GetParamValue(const std::string& key) const
{
    if (!hasInitialized_ ||
        (jsonVal_.type() != Json::ValueType::nullValue &&
        jsonVal_.type() != Json::ValueType::objectValue)) {
        return Json::Value(Json::ValueType::nullValue);
    }
    return jsonVal_[key];
}

int HiSysEventValue::Size() const
{
    if (!hasInitialized_) {
        return DEFAULT_INT_VAL;
    }
    return jsonVal_.size();
}

int64_t HiSysEventValue::AsInt64() const
{
#ifdef JSON_HAS_INT64
    if (!hasInitialized_ ||
        (jsonVal_.type() == Json::ValueType::uintValue && !jsonVal_.isInt64()) ||
        (jsonVal_.type() == Json::ValueType::realValue &&
        !InValidRange(jsonVal_.asDouble(), Json::Value::minInt64, Json::Value::maxInt64))) {
        return DEFAULT_INT64_VAL;
    }
    return jsonVal_.asInt64();
#else
    return DEFAULT_INT64_VAL;
#endif
}

uint64_t HiSysEventValue::AsUInt64() const
{
#ifdef JSON_HAS_INT64
    if (!hasInitialized_ ||
        (jsonVal_.type() == Json::ValueType::intValue && !jsonVal_.isUInt64()) ||
        (jsonVal_.type() == Json::ValueType::realValue &&
        !InValidRange(jsonVal_.asDouble(), 0, Json::Value::maxUInt64))) {
        return DEFAULT_UINT64_VAL;
    }
    return jsonVal_.asUInt64();
#else
    return DEFAULT_UINT64_VAL;
#endif
}

double HiSysEventValue::AsDouble() const
{
    if (!hasInitialized_) {
        return DEFAULT_DOUBLE_VAL;
    }
    return jsonVal_.asDouble();
}

std::string HiSysEventValue::AsString() const
{
    if (!hasInitialized_) {
        return "";
    }
    return jsonVal_.asString();
}

Json::ValueType HiSysEventValue::Type() const
{
    if (!hasInitialized_) {
        return Json::ValueType::nullValue;
    }
    return jsonVal_.type();
}
} // HiviewDFX
} // OHOS
