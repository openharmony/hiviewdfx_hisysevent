/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_HIVIEWDFX_SYS_EVENT_RULE_H
#define OHOS_HIVIEWDFX_SYS_EVENT_RULE_H

#include "parcel.h"
#include "rule_type.h"

namespace OHOS {
namespace HiviewDFX {
class SysEventRule : public Parcelable {
public:
    SysEventRule() {};
    SysEventRule(const std::string& domain, const std::string& eventName,
        const std::string& tag, uint32_t ruleType = RuleType::WHOLE_WORD)
        : domain(domain), eventName(eventName), tag(tag), ruleType(ruleType) {}
    SysEventRule(const std::string& domain, const std::string& eventName,
        uint32_t ruleType = RuleType::WHOLE_WORD)
        : SysEventRule(domain, eventName, "", ruleType) {}
    SysEventRule(const std::string& tag, uint32_t ruleType = RuleType::WHOLE_WORD)
        : SysEventRule("", "", tag, ruleType) {}
    ~SysEventRule() = default;

    bool Marshalling(Parcel& parcel) const override;
    static SysEventRule* Unmarshalling(Parcel& parcel);

    std::string domain;
    std::string eventName;
    std::string tag;
    uint32_t ruleType = RuleType::WHOLE_WORD;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_SYS_EVENT_RULE_H
