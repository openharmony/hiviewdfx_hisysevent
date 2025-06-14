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

#include "hisysevent_record.h"
#include "cJSON.h"

#include <functional>
#include <unordered_map>

namespace OHOS {
namespace HiviewDFX {
enum Validity {
    KEY_INVALID,
    VALUE_INVALID,
    KV_BOTH_VALID
};

using BaseInfoHandler = std::function<bool(cJSON*)>;
using ExtensiveInfoHander = std::function<bool(cJSON*, cJSON*)>;
using DecorateMarks = std::unordered_map<std::string, Validity>;

class HiSysEventJsonDecorator {
public:
    HiSysEventJsonDecorator();
    ~HiSysEventJsonDecorator();

public:
    std::string DecorateEventJsonStr(const HiSysEventRecord& record);

private:
    bool CheckAttrDecorationNeed(cJSON* eventJson, const std::string& key,
        cJSON* standard);
    Validity CheckAttrValidity(cJSON* eventJson, const std::string& key,
        cJSON* standard);
    Validity CheckLevelValidity(cJSON* baseInfo);
    bool CheckEventDecorationNeed(cJSON* eventJson, BaseInfoHandler baseJsonInfoHandler,
        ExtensiveInfoHander extensiveJsonInfoHandler);
    std::string Decorate(Validity validity, std::string& key, std::string& value);
    std::string DecorateJsonStr(const std::string& standard, DecorateMarks marks);
    bool JudgeDataType(const std::string& dataType, cJSON* eventJson);

private:
    cJSON* jsonRoot_;
    DecorateMarks decoratedMarks_;
    bool isJsonRootValid_ = false;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_JSON_DECORATOR_H