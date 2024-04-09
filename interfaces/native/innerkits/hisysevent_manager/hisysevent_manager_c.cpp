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

#include "hisysevent_manager_c.h"

#include <map>
#include <mutex>
#include <string>

#include "hisysevent_base_manager.h"
#include "hisysevent_listener_c.h"
#include "hisysevent_query_callback_c.h"
#include "ret_code.h"

namespace {
using OHOS::HiviewDFX::HiSysEventBaseManager;
using OHOS::HiviewDFX::HiSysEventBaseQueryCallback;
using QueryArgCls = OHOS::HiviewDFX::QueryArg;
using QueryRuleCls = OHOS::HiviewDFX::QueryRule;
using OHOS::HiviewDFX::RuleType::WHOLE_WORD;
using OHOS::HiviewDFX::ERR_QUERY_RULE_INVALID;
using ListenerRuleCls = OHOS::HiviewDFX::ListenerRule;
using OHOS::HiviewDFX::HiSysEventBaseListener;
using OHOS::HiviewDFX::IPC_CALL_SUCCEED;
using OHOS::HiviewDFX::ERR_LISTENER_NOT_EXIST;
using OHOS::HiviewDFX::RuleType;

static std::map<std::pair<OnEventFunc, OnServiceDiedFunc>, std::shared_ptr<HiSysEventBaseListener>> watchers;
std::mutex g_mapMutex;

int HiSysEventQuery(const HiSysEventQueryArg& arg, HiSysEventQueryRule rules[], size_t ruleSize,
    HiSysEventQueryCallback& callback)
{
    std::vector<QueryRuleCls> queryRules;
    for (size_t i = 0; i < ruleSize; ++i) {
        if (strlen(rules[i].domain) == 0 || rules[i].eventListSize == 0) {
            return ERR_QUERY_RULE_INVALID;
        }
        std::vector<std::string> eventList;
        for (size_t j = 0; j < rules[i].eventListSize; ++j) {
            eventList.emplace_back(rules[i].eventList[j]);
        }
        std::string cond = rules[i].condition == nullptr ? "" : rules[i].condition;
        queryRules.emplace_back(rules[i].domain, eventList, WHOLE_WORD, 0, cond);
    }
    QueryArgCls argCls(arg.beginTime, arg.endTime, arg.maxEvents);
    auto callbackC = std::make_shared<HiSysEventQueryCallbackC>(callback.OnQuery, callback.OnComplete);
    return HiSysEventBaseManager::Query(argCls, queryRules, std::make_shared<HiSysEventBaseQueryCallback>(callbackC));
}

int HiSysEventAddWatcher(HiSysEventWatcher& watcher, HiSysEventWatchRule rules[], size_t ruleSize)
{
    std::vector<ListenerRuleCls> listenerRules;
    for (size_t i = 0; i < ruleSize; ++i) {
        listenerRules.emplace_back(rules[i].domain, rules[i].name, rules[i].tag, RuleType(rules[i].ruleType),
            static_cast<uint32_t>(rules[i].eventType));
    }
    auto listenerC = std::make_shared<HiSysEventBaseListener>(
        std::make_shared<HiSysEventListenerC>(watcher.OnEvent, watcher.OnServiceDied));
    auto ret = HiSysEventBaseManager::AddListener(listenerC, listenerRules);
    if (ret != IPC_CALL_SUCCEED) {
        return ret;
    }
    watchers[std::make_pair(watcher.OnEvent, watcher.OnServiceDied)] = listenerC;
    return ret;
}

int HiSysEventRemoveWatcher(HiSysEventWatcher& watcher)
{
    auto watcherKey = std::make_pair(watcher.OnEvent, watcher.OnServiceDied);
    auto watcherIter = watchers.find(watcherKey);
    if (watcherIter == watchers.end()) {
        return ERR_LISTENER_NOT_EXIST;
    }
    auto ret = HiSysEventBaseManager::RemoveListener(watcherIter->second);
    std::lock_guard<std::mutex> lock(g_mapMutex);
    if (ret == IPC_CALL_SUCCEED) {
        watchers.erase(watcherIter->first);
    }
    return ret;
}
}

#ifdef __cplusplus
extern "C" {
#endif

int OH_HiSysEvent_Query(const HiSysEventQueryArg* arg, HiSysEventQueryRule rules[], size_t ruleSize,
    HiSysEventQueryCallback* callback)
{
    if (arg == nullptr) {
        return OHOS::HiviewDFX::ERR_QUERY_ARG_NULL;
    }
    if (callback == nullptr || callback->OnQuery == nullptr || callback->OnComplete == nullptr) {
        return OHOS::HiviewDFX::ERR_QUERY_CALLBACK_NULL;
    }
    return HiSysEventQuery(*arg, rules, ruleSize, *callback);
}

int OH_HiSysEvent_Add_Watcher(HiSysEventWatcher* watcher, HiSysEventWatchRule rules[], size_t ruleSize)
{
    if (watcher == nullptr || watcher->OnEvent == nullptr || watcher->OnServiceDied == nullptr) {
        return OHOS::HiviewDFX::ERR_LISTENER_NOT_EXIST;
    }
    return HiSysEventAddWatcher(*watcher, rules, ruleSize);
}

int OH_HiSysEvent_Remove_Watcher(HiSysEventWatcher* watcher)
{
    if (watcher == nullptr || watcher->OnEvent == nullptr || watcher->OnServiceDied == nullptr) {
        return OHOS::HiviewDFX::ERR_LISTENER_NOT_EXIST;
    }
    return HiSysEventRemoveWatcher(*watcher);
}

#ifdef __cplusplus
}
#endif
