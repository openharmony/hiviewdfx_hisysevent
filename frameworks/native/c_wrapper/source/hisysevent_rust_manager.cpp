/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "hisysevent_c_wrapper.h"

#include <string>
#include <map>
#include <mutex>

#include "hisysevent_base_manager.h"
#include "hisysevent_rust_listener.h"
#include "hisysevent_rust_querier.h"
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

static std::map<std::pair<OnRustCb, OnRustCb>, std::shared_ptr<HiSysEventBaseListener>> g_baseWatchers;
static std::mutex g_baseWatchersMutex;
static std::map<std::pair<OnRustCb, OnRustCb>, std::shared_ptr<HiSysEventRustListener>> g_watchers;
static std::mutex g_watchersMutex;
static std::map<std::pair<OnRustCb, OnRustCb>, std::shared_ptr<HiSysEventRustQuerier>> g_queriers;
static std::mutex g_queriersMutex;

int HiSysEventQuery(HiSysEventQueryArg* arg, const HiSysEventQueryRule rules[], const size_t ruleSize,
    HiSysEventRustQuerierC* querier)
{
    if (querier == nullptr || querier->status != STATUS_NORMAL) {
        return ERR_LISTENER_NOT_EXIST;
    }
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
    int64_t beginTime = 0;
    int64_t endTime = 0;
    int32_t maxEvents = 0;
    if (arg != nullptr) {
        beginTime = arg->beginTime;
        endTime = arg->endTime;
        maxEvents = arg->maxEvents;
    }
    QueryArgCls argCls(beginTime, endTime, maxEvents);
    auto querierRust = std::make_shared<HiSysEventRustQuerier>(querier);
    auto baseQuerierRust = std::make_shared<HiSysEventBaseQueryCallback>(querierRust);
    auto ret = HiSysEventBaseManager::Query(argCls, queryRules, baseQuerierRust);
    if (ret == IPC_CALL_SUCCEED) {
        std::lock_guard<std::mutex> lock(g_queriersMutex);
        g_queriers[std::make_pair(querier->onQueryRustCb, querier->onCompleteRustCb)] = querierRust;
    }
    return ret;
}

int HiSysEventAddWatcher(HiSysEventRustWatcherC* watcher, const HiSysEventWatchRule rules[],
    const size_t ruleSize)
{
    if (watcher == nullptr || watcher->status != STATUS_NORMAL) {
        return ERR_LISTENER_NOT_EXIST;
    }
    std::vector<ListenerRuleCls> watchRules;
    for (size_t i = 0; i < ruleSize; ++i) {
        watchRules.emplace_back(rules[i].domain, rules[i].name, rules[i].tag,
            RuleType(rules[i].ruleType), static_cast<uint32_t>(rules[i].eventType));
    }
    auto listenerRust = std::make_shared<HiSysEventRustListener>(watcher);
    auto baseListenerRust = std::make_shared<HiSysEventBaseListener>(listenerRust);
    auto ret = HiSysEventBaseManager::AddListener(baseListenerRust, watchRules);
    if (ret != IPC_CALL_SUCCEED) {
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(g_baseWatchersMutex);
        g_baseWatchers[std::make_pair(watcher->onEventRustCb, watcher->onServiceDiedRustCb)] = baseListenerRust;
    }
    {
        std::lock_guard<std::mutex> lock(g_watchersMutex);
        g_watchers[std::make_pair(watcher->onEventRustCb, watcher->onServiceDiedRustCb)] = listenerRust;
    }
    return ret;
}

int HiSysEventRemoveWatcher(HiSysEventRustWatcherC* watcher)
{
    if (watcher == nullptr || watcher->status != STATUS_NORMAL) {
        return ERR_LISTENER_NOT_EXIST;
    }
    auto watcherKey = std::make_pair(watcher->onEventRustCb, watcher->onServiceDiedRustCb);
    auto watcherIter = g_baseWatchers.find(watcherKey);
    if (watcherIter == g_baseWatchers.end()) {
        return ERR_LISTENER_NOT_EXIST;
    }
    auto ret = HiSysEventBaseManager::RemoveListener(watcherIter->second);
    if (ret != IPC_CALL_SUCCEED) {
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(g_baseWatchersMutex);
        g_baseWatchers.erase(watcherIter->first);
    }
    return ret;
}

void HiSysEventRecycleWatcher(HiSysEventRustWatcherC* watcher)
{
    if (watcher == nullptr) {
        return;
    }
    auto watcherKey = std::make_pair(watcher->onEventRustCb, watcher->onServiceDiedRustCb);
    auto watcherIter = g_watchers.find(watcherKey);
    if (watcherIter == g_watchers.end()) {
        return;
    }
    auto listener = watcherIter->second;
    if (listener != nullptr) {
        listener->RecycleWatcher(watcher);
    }
    {
        std::lock_guard<std::mutex> lock(g_watchersMutex);
        g_watchers.erase(watcherIter->first);
    }
}

void HiSysEventRecycleQuerier(HiSysEventRustQuerierC* querier)
{
    if (querier == nullptr) {
        return;
    }
    auto querierKey = std::make_pair(querier->onQueryRustCb, querier->onCompleteRustCb);
    auto querierIter = g_queriers.find(querierKey);
    if (querierIter == g_queriers.end()) {
        return;
    }
    auto callback = querierIter->second;
    if (callback != nullptr) {
        callback->RecycleQuerier(querier);
    }
    {
        std::lock_guard<std::mutex> lock(g_queriersMutex);
        g_queriers.erase(querierIter->first);
    }
}
}

#ifdef __cplusplus
extern "C" {
#endif

int OhHiSysEventAddRustWatcher(HiSysEventRustWatcherC* watcher, const HiSysEventWatchRule watchRules[],
    const size_t ruleSize)
{
    return HiSysEventAddWatcher(watcher, watchRules, ruleSize);
}

int OhHiSysEventRemoveRustWatcher(HiSysEventRustWatcherC* watcher)
{
    return HiSysEventRemoveWatcher(watcher);
}

int OhHiSysEventRustQuery(HiSysEventQueryArg* queryArg, const HiSysEventQueryRule queryRules[],
    const size_t ruleSize, HiSysEventRustQuerierC* querier)
{
    return HiSysEventQuery(queryArg, queryRules, ruleSize, querier);
}

void OhHiSysEventRecycleRustWatcher(HiSysEventRustWatcherC* watcher)
{
    HiSysEventRecycleWatcher(watcher);
}

void OhHiSysEventRecycleRustQuerier(HiSysEventRustQuerierC* querier)
{
    HiSysEventRecycleQuerier(querier);
}

#ifdef __cplusplus
}
#endif