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
                value.emplace_back(static_cast<int64_t>(cJSON_GetNumberValue(src->Index(i))));
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
                value.emplace_back(static_cast<uint64_t>(cJSON_GetNumberValue(src->Index(i))));
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
                value.emplace_back(cJSON_GetNumberValue(src->Index(i)));
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
                value.emplace_back(cJSON_GetStringValue(src->Index(i)));
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
        HILOG_DEBUG(LOG_CORE, "value type with key named \"%{public}s\" is not match.",
            param.c_str());
        return ERR_TYPE_NOT_MATCH;
    }
    assignFunc(parsedVal);
    return VALUE_PARSED_SUCCEED;
}

bool HiSysEventRecord::IsInt64ValueType(const JsonValue val) const
{
    return val->IsNumeric();
}

bool HiSysEventRecord::IsUInt64ValueType(const JsonValue val) const
{
    return val->IsNumeric();
}

bool HiSysEventRecord::IsDoubleValueType(const JsonValue val) const
{
    return val->IsNumeric();
}

bool HiSysEventRecord::IsStringValueType(const JsonValue val) const
{
    return val->IsString();
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
    jsonVal_ = cJSON_Parse(jsonStr.c_str());
    if (jsonVal_ == nullptr || !cJSON_IsObject(jsonVal_)) {
        HILOG_ERROR(LOG_CORE, "parse json file failed, please check the style of json string: %{public}s.",
            jsonStr.c_str());
        return;
    }
    hasInitialized_ = true;
    needDeleteJsonVal = true;
}

bool HiSysEventValue::HasInitialized() const
{
    return hasInitialized_;
}

HiSysEventValue::~HiSysEventValue()
{
    if (needDeleteJsonVal && jsonVal_ != nullptr) {
        cJSON_Delete(jsonVal_);
    }
}

void HiSysEventValue::GetParamNames(std::vector<std::string>& params) const
{
    if (!hasInitialized_ || !cJSON_IsObject(jsonVal_)) {
        return;
    }
    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, jsonVal_) {
        if (item == nullptr) {
            continue;
        }
        params.emplace_back(item->string);
    }
}

bool HiSysEventValue::IsArray() const
{
    if (!hasInitialized_) {
        return false;
    }
    return cJSON_IsArray(jsonVal_);
}

bool HiSysEventValue::IsMember(const std::string key) const
{
    if (!hasInitialized_ || !cJSON_IsObject(jsonVal_)) {
        return false;
    }
    return cJSON_HasObjectItem(jsonVal_, key.c_str());
}

bool HiSysEventValue::IsInt64() const
{
    return cJSON_IsNumber(jsonVal_);
}

bool HiSysEventValue::IsUInt64() const
{
    return cJSON_IsNumber(jsonVal_);
}

bool HiSysEventValue::IsDouble() const
{
    return cJSON_IsNumber(jsonVal_);
}

bool HiSysEventValue::IsString() const
{
    return cJSON_IsString(jsonVal_);
}

bool HiSysEventValue::IsBool() const
{
    if (!hasInitialized_) {
        return false;
    }
    return cJSON_IsBool(jsonVal_);
}

bool HiSysEventValue::IsNull() const
{
    if (!hasInitialized_) {
        return false;
    }
    return cJSON_IsNull(jsonVal_);
}

bool HiSysEventValue::IsNumeric() const
{
    if (!hasInitialized_) {
        return false;
    }
    return cJSON_IsNumber(jsonVal_);
}

cJSON* HiSysEventValue::Index(const int index) const
{
    if (!hasInitialized_ || index < 0 || !cJSON_IsArray(jsonVal_)
        || (index >= cJSON_GetArraySize(jsonVal_))) {
        return nullptr;
    }
    return cJSON_GetArrayItem(jsonVal_, index);
}
cJSON* HiSysEventValue::GetParamValue(const std::string& key) const
{
    if (!hasInitialized_ || !cJSON_IsObject(jsonVal_)) {
        return nullptr;
    }
    return cJSON_GetObjectItem(jsonVal_, key.c_str());
}

int HiSysEventValue::Size() const
{
    if (!hasInitialized_ || !cJSON_IsArray(jsonVal_)) {
        return DEFAULT_INT_VAL;
    }
    return cJSON_GetArraySize(jsonVal_);
}

int64_t HiSysEventValue::AsInt64() const
{
    if (!hasInitialized_ || !cJSON_IsNumber(jsonVal_)) {
        return DEFAULT_INT64_VAL;
    }
    return static_cast<int64_t>(cJSON_GetNumberValue(jsonVal_));
}

uint64_t HiSysEventValue::AsUInt64() const
{
    if (!hasInitialized_ || !cJSON_IsNumber(jsonVal_)) {
        return DEFAULT_UINT64_VAL;
    }
    return static_cast<uint64_t>(cJSON_GetNumberValue(jsonVal_));
}

double HiSysEventValue::AsDouble() const
{
    if (!hasInitialized_ || !cJSON_IsNumber(jsonVal_)) {
        return DEFAULT_DOUBLE_VAL;
    }
    return cJSON_GetNumberValue(jsonVal_);
}

std::string HiSysEventValue::AsString() const
{
    if (!hasInitialized_ || !cJSON_IsString(jsonVal_)) {
        return "";
    }
    return cJSON_GetStringValue(jsonVal_);
}
} // HiviewDFX
} // OHOS
