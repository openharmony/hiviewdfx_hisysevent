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

#ifndef HISYSEVENT_MANAGER_H
#define HISYSEVENT_MANAGER_H

#include <string>
#include <vector>

#include "hisysevent_query_callback_base.h"
#include "hisysevent_subscribe_callback_native.h"

namespace OHOS {
namespace HiviewDFX {
struct QueryArg {
    long long beginTime;
    long long endTime;
    int maxEvents;
    QueryArg(const long long begin, const long long end, const int max)
        : beginTime(begin), endTime(end), maxEvents(max) {};
    QueryArg() {}
};

struct ListenerRule {
    uint32_t ruleType {0};
    std::string domain;
    std::string eventName;
    ListenerRule(const uint32_t type, const std::string& domainIn, const std::string& event)
        : ruleType(type), domain(domainIn), eventName(event) {}
    ListenerRule() {}
};

struct QueryRule {
    uint32_t ruleType {0};
    std::string domain;
    std::vector<std::string> eventList;
    QueryRule(const uint32_t type, const std::string& domainIn, const std::vector<std::string>& events)
        : ruleType(type), domain(domainIn), eventList(events) {}
    QueryRule() {}
};

class HiSysEventManager {
public:
    HiSysEventManager() = default;
    static int AddEventListener(std::shared_ptr<HiSysEventSubscribeCallBackBase> listener,
        std::vector<struct ListenerRule>& rules);
    static void RemoveListener(std::shared_ptr<HiSysEventSubscribeCallBackBase> listener);
    static bool QueryHiSysEvent(struct QueryArg& queryArg,
        std::vector<struct QueryRule>& queryRules,
        std::shared_ptr<HiSysEventQueryCallBackBase> queryCallBack);
    static bool SetDebugMode(std::shared_ptr<HiSysEventSubscribeCallBackBase> listener, bool mode);
    ~HiSysEventManager() {}
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_MANAGER_H
