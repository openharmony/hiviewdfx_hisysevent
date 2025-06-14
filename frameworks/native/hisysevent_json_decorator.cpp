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

#include "hisysevent_json_decorator.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>

#include "hilog/log.h"
#include "hisysevent.h"
#include "json_flatten_parser.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_JSON_DECORATOR"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char ARRY_SIZE[] = "arrsize";
constexpr char DECORATE_PREFIX[] = "\033[31m";
constexpr char DECORATE_SUFFIX[] = "\033[0m";
constexpr char HISYSEVENT_YAML_DEF_JSON_PATH[] = "/data/system/hiview/unzip_configs/sys_event_def/hisysevent.def";
constexpr char LEVEL[] = "level";
constexpr char LEVEL_[] = "level_";
constexpr char TYPE[] = "type";
constexpr int BASE_INDEX = 0;
constexpr int DOMAIN_INDEX = 1;
constexpr int NAME_INDEX = 2;
const char* INNER_BUILD_KEYS[] = {
    "__BASE", "domain_", "name_", "type_", "level_", "tag_",
    "time_", "tz_", "pid_", "tid_", "uid_", "traceid_", "log_",
    "id_", "spanid_", "pspanid_", "trace_flag_", "info_", "seq_",
    "period_seq_"};
const std::string VALID_LEVELS[] = { "CRITICAL", "MINOR" };
const std::map<std::string, int> EVENT_TYPE_MAP = {{"FAULT", 1}, {"STATISTIC", 2}, {"SECURITY", 3}, {"BEHAVIOR", 4} };

int GetIntFromJson(cJSON* jsonObj, const std::string& key)
{
    cJSON* intJson = cJSON_GetObjectItemCaseSensitive(jsonObj, key.c_str());
    if (!cJSON_IsNumber(intJson)) {
        return 0;
    }
    return static_cast<int>(cJSON_GetNumberValue(intJson));
}

std::string GetStringFromJson(cJSON* jsonObj, const std::string& key)
{
    cJSON* strJson = cJSON_GetObjectItemCaseSensitive(jsonObj, key.c_str());
    if (!cJSON_IsString(strJson)) {
        return "";
    }
    return std::string(cJSON_GetStringValue(strJson));
}
}

HiSysEventJsonDecorator::HiSysEventJsonDecorator()
{
    std::ifstream fin(HISYSEVENT_YAML_DEF_JSON_PATH);
    if (!fin.is_open()) {
        HILOG_ERROR(LOG_CORE, "failed to open json file: %{public}s.", HISYSEVENT_YAML_DEF_JSON_PATH);
        return;
    }
    std::stringstream buf;
    buf << fin.rdbuf(); // read all content of def file
    jsonRoot_ = cJSON_Parse(buf.str().c_str());
    if (jsonRoot_ == nullptr || !cJSON_IsObject(jsonRoot_)) {
        HILOG_ERROR(LOG_CORE, "parse json file failed, please check the style of json file: %{public}s.",
            HISYSEVENT_YAML_DEF_JSON_PATH);
        return;
    }
    isJsonRootValid_ = true;
}

HiSysEventJsonDecorator::~HiSysEventJsonDecorator()
{
    if (jsonRoot_ != nullptr) {
        cJSON_Delete(jsonRoot_);
    }
}

bool HiSysEventJsonDecorator::CheckAttrDecorationNeed(cJSON* eventJson, const std::string& key,
    cJSON* standard)
{
    auto ret = CheckAttrValidity(eventJson, key, standard);
    decoratedMarks_[key] = ret;
    return ret != Validity::KV_BOTH_VALID;
}

Validity HiSysEventJsonDecorator::CheckAttrValidity(cJSON* eventJson, const std::string& key,
    cJSON* standard)
{
    cJSON* standardObjItem = cJSON_GetObjectItemCaseSensitive(standard, key.c_str());
    cJSON* eventObjItem = cJSON_GetObjectItemCaseSensitive(eventJson, key.c_str());
    if (standardObjItem == nullptr || eventObjItem == nullptr) {
        return Validity::KEY_INVALID;
    }
    bool ret = false;
    if (cJSON_HasObjectItem(standardObjItem, ARRY_SIZE)) {
        if (!cJSON_IsArray(eventObjItem) ||
            cJSON_GetArraySize(eventObjItem) > GetIntFromJson(standardObjItem, ARRY_SIZE)) {
            return Validity::VALUE_INVALID;
        }
        int arraySize = cJSON_GetArraySize(eventObjItem);
        ret = (arraySize == 0) ||
            JudgeDataType(GetStringFromJson(standardObjItem, TYPE), cJSON_GetArrayItem(eventObjItem, 0));
        return ret ? Validity::KV_BOTH_VALID : Validity::VALUE_INVALID;
    }
    ret = JudgeDataType(GetStringFromJson(standardObjItem, TYPE), eventObjItem);
    return ret ? Validity::KV_BOTH_VALID : Validity::VALUE_INVALID;
}

Validity HiSysEventJsonDecorator::CheckLevelValidity(cJSON* baseInfo)
{
    cJSON* levelObjItem = cJSON_GetObjectItemCaseSensitive(baseInfo, LEVEL);
    if (levelObjItem == nullptr || !cJSON_IsString(levelObjItem)) {
        HILOG_ERROR(LOG_CORE, "level not defined in __BASE");
        return Validity::KEY_INVALID;
    }
    std::string levelDes = cJSON_GetStringValue(levelObjItem);
    if (std::any_of(std::begin(VALID_LEVELS), std::end(VALID_LEVELS), [&levelDes] (auto& level) {
        return level == levelDes;
    })) {
        return Validity::KV_BOTH_VALID;
    }
    return Validity::VALUE_INVALID;
}

bool HiSysEventJsonDecorator::CheckEventDecorationNeed(cJSON* eventJson,
    BaseInfoHandler baseJsonInfoHandler, ExtensiveInfoHander extensiveJsonInfoHandler)
{
    if (!isJsonRootValid_ || !cJSON_IsObject(jsonRoot_) || !cJSON_IsObject(eventJson)) {
        return true;
    }
    std::string domain = GetStringFromJson(eventJson, INNER_BUILD_KEYS[DOMAIN_INDEX]);
    std::string name = GetStringFromJson(eventJson, INNER_BUILD_KEYS[NAME_INDEX]);
    cJSON* definedDomain = cJSON_GetObjectItemCaseSensitive(jsonRoot_, domain.c_str());
    cJSON* definedName = cJSON_GetObjectItemCaseSensitive(definedDomain, name.c_str());
    if (definedDomain == nullptr || !cJSON_IsObject(definedDomain) ||
        definedName == nullptr || !cJSON_IsObject(definedName)) {
        return true;
    }
    cJSON* baseInfoItem = cJSON_GetObjectItemCaseSensitive(definedName, INNER_BUILD_KEYS[BASE_INDEX]);
    return baseJsonInfoHandler(baseInfoItem) || extensiveJsonInfoHandler(eventJson, definedName);
}

std::string HiSysEventJsonDecorator::Decorate(Validity validity, std::string& key, std::string& value)
{
    std::stringstream ss;
    switch (validity) {
        case Validity::KEY_INVALID:
            ss << "\"" << DECORATE_PREFIX << key << DECORATE_SUFFIX << "\":" << value;
            break;
        case Validity::VALUE_INVALID:
            ss << "\"" << key << "\":" << DECORATE_PREFIX << value << DECORATE_SUFFIX;
            break;
        case Validity::KV_BOTH_VALID:
            ss << "\"" << key << "\":" << value;
            break;
        default:
            break;
    }
    std::string ret = ss.str();
    ss.clear();
    return ret;
}

std::string HiSysEventJsonDecorator::DecorateEventJsonStr(const HiSysEventRecord& record)
{
    std::string origin = record.AsJson();
    decoratedMarks_.clear(); // reset marked keys.
    if (!isJsonRootValid_) {
        HILOG_ERROR(LOG_CORE, "root json value is not valid, failed to decorate.");
        return origin;
    }

    cJSON* eventJson = cJSON_Parse(origin.c_str());
    if (eventJson == nullptr) {
        return origin;
    }
    if (!cJSON_IsObject(eventJson)) {
        cJSON_Delete(eventJson);
        return origin;
    }
    auto needDecorate = CheckEventDecorationNeed(eventJson,
        [this] (cJSON* definedBase) {
            auto levelValidity = CheckLevelValidity(definedBase);
            decoratedMarks_[LEVEL_] = levelValidity;
            return levelValidity != Validity::KV_BOTH_VALID;
        }, [this] (cJSON* eventJson, cJSON* definedName) {
                bool ret = false;
                cJSON* item = nullptr;
                cJSON_ArrayForEach(item, eventJson) {
                    if (item == nullptr) {
                        continue;
                    }
                    std::string key(item->string);
                    if (std::find_if(std::cbegin(INNER_BUILD_KEYS), std::cend(INNER_BUILD_KEYS),
                        [&ret, &eventJson, &key, &definedName] (const char* ele) {
                            return (key.compare(ele) == 0);
                        }) == std::cend(INNER_BUILD_KEYS)) {
                        ret = this->CheckAttrDecorationNeed(eventJson, key, definedName) || ret;
                    }
                }
                return ret;
            });
    if (!needDecorate) {
        HILOG_DEBUG(LOG_CORE, "no need to decorate this event json string.");
        return origin;
    }
    HILOG_DEBUG(LOG_CORE, "all invalid key or value will be high-lighted with red color.");
    return DecorateJsonStr(origin, decoratedMarks_);
}

std::string HiSysEventJsonDecorator::DecorateJsonStr(const std::string& origin, DecorateMarks marks)
{
    if (marks.empty()) {
        return origin;
    }
    JsonFlattenParser parser(origin);
    return parser.Print([this, &marks] (KV& kv) -> std::string {
        auto iter = marks.find(kv.first);
        return this->Decorate((iter == marks.end() ? Validity::KV_BOTH_VALID : iter->second), kv.first, kv.second);
    });
}

bool HiSysEventJsonDecorator::JudgeDataType(const std::string &dataType, cJSON* eventJson)
{
    if (dataType.compare("BOOL") == 0) {
        return cJSON_IsBool(eventJson);
    } else if ((dataType.compare("INT8") == 0) || (dataType.compare("INT16") == 0) ||
        (dataType.compare("INT32") == 0) || (dataType.compare("INT64") == 0)) {
        if (!cJSON_IsNumber(eventJson)) {
            return false;
        }
        double num = cJSON_GetNumberValue(eventJson);
        return (num <= std::numeric_limits<int64_t>::max()) && (num >= std::numeric_limits<int64_t>::lowest());
    } else if ((dataType.compare("UINT8") == 0) || (dataType.compare("UINT16") == 0) ||
        (dataType.compare("UINT32") == 0) || (dataType.compare("UINT64") == 0)) {
        if (!cJSON_IsNumber(eventJson)) {
            return false;
        }
        double num = cJSON_GetNumberValue(eventJson);
        return (num <= std::numeric_limits<uint64_t>::max()) && (num >= std::numeric_limits<uint64_t>::lowest());
    } else if ((dataType.compare("FLOAT") == 0) || (dataType.compare("DOUBLE") == 0)) {
        return cJSON_IsNumber(eventJson);
    } else if (dataType.compare("STRING") == 0) {
        return cJSON_IsString(eventJson);
    } else {
        return false;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
