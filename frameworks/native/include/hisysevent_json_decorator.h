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

#ifndef HISYSEVENT_JSON_DECORATOR_H
#define HISYSEVENT_JSON_DECORATOR_H

#include "json/json.h"

#include <functional>
#include <unordered_map>

namespace OHOS {
namespace HiviewDFX {
enum Validity {
    KEY_INVALID,
    VALUE_INVALID,
    KV_BOTH_VALID
};

using BASE_INFO_HANDLER = std::function<bool(const Json::Value&)>;
using EXTENSIVE_INFO_HANDLER = std::function<bool(const Json::Value&, const Json::Value&)>;
using DECORATE_MARKS = std::unordered_map<std::string, Validity>;

class HiSysEventJsonDecorator {
public:
    HiSysEventJsonDecorator();
    std::string DecorateEventJsonStr(const std::string& origin);

private:
    bool CheckAttrDecorationNeed(const Json::Value& eventJson, const std::string& key,
        const Json::Value& standard);
    Validity CheckAttrValidity(const Json::Value& eventJson, const std::string& key,
        const Json::Value& standard);
    Validity CheckLevelValidity(const Json::Value& baseInfo);
    bool CheckEventDecorationNeed(const Json::Value& eventJson, BASE_INFO_HANDLER baseJsonInfoHandler,
        EXTENSIVE_INFO_HANDLER extensiveJsonInfoHandler);
    void Decorate(std::stringstream& ss, const Json::Value& origin, Validity validity, std::string& key);
    std::string DecorateJsonStr(const Json::Value& origin, const std::string& standard,
        DECORATE_MARKS marks);
    bool JudgeDataType(const std::string& dataType, const Json::Value& eventJson);
    std::string TranslateJsonToStr(const Json::Value& json);

private:
    Json::Value root;
    bool isRootValid;
    DECORATE_MARKS decoratedMarks;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_JSON_DECORATOR_H