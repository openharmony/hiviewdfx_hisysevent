/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "hisysevent_delegate.h"

#include <memory>

#include "file_util.h"
#include "hilog/log.h"
#include "hisysevent_listener_proxy.h"
#include "hisysevent_query_proxy.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "query_argument.h"
#include "ret_code.h"
#ifdef STORAGE_SERVICE_ENABLE
#include "storage_acl.h"
#endif
#include "sys_event_service_proxy.h"
#include "system_ability_definition.h"

using namespace std;
#ifdef STORAGE_SERVICE_ENABLE
using namespace OHOS::StorageDaemon;
#endif

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_DELEGATE"

namespace OHOS {
namespace HiviewDFX {

namespace {
const std::string EVENT_DIR = "/data/storage/el2/base/cache/hiview/event";
#ifdef STORAGE_SERVICE_ENABLE
const std::string BASE_DIR = "/data/storage/el2/base";
const std::string CACHE_DIR = "/data/storage/el2/base/cache";
const std::string HIVIEW_DIR = "/data/storage/el2/base/cache/hiview";
const std::string PARENT_DIR_PERMISSION = "g:1201:x";
const std::string SUB_DIR_PERMISSION = "g:1201:rwx";
constexpr int ACL_SUCC = 0;
#endif
}

int32_t HiSysEventDelegate::AddListener(const std::shared_ptr<HiSysEventBaseListener> listener,
    const std::vector<ListenerRule>& rules)
{
    auto service = GetSysEventService();
    if (service == nullptr) {
        HILOG_ERROR(LOG_CORE, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    std::vector<SysEventRule> eventRules;
    ConvertListenerRule(rules, eventRules);

    if (!spListenerCallBack) {
        spListenerCallBack = new OHOS::HiviewDFX::HiSysEventListenerProxy(listener);
    }

    SysEventServiceProxy sysEventService(service);
    service->RemoveDeathRecipient(spListenerCallBack->GetCallbackDeathRecipient());
    return sysEventService.AddListener(eventRules, spListenerCallBack);
}

int32_t HiSysEventDelegate::RemoveListener(const std::shared_ptr<HiSysEventBaseListener> listener)
{
    if (!spListenerCallBack) {
        return ERR_LISTENER_NOT_EXIST;
    }
    auto service = GetSysEventService();
    if (service == nullptr) {
        HILOG_ERROR(LOG_CORE, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    SysEventServiceProxy sysEventService(service);
    return sysEventService.RemoveListener(spListenerCallBack);
}

int32_t HiSysEventDelegate::SetDebugMode(const std::shared_ptr<HiSysEventBaseListener> listener,
    const bool mode)
{
    if (!spListenerCallBack) {
        return ERR_LISTENER_NOT_EXIST;
    }
    auto service = GetSysEventService();
    if (service == nullptr) {
        HILOG_ERROR(LOG_CORE, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    SysEventServiceProxy sysEventService(service);
    return sysEventService.SetDebugMode(spListenerCallBack, mode);
}

int32_t HiSysEventDelegate::Query(const struct QueryArg& arg,
    const std::vector<QueryRule>& rules,
    const std::shared_ptr<HiSysEventBaseQueryCallback> callback) const
{
    auto service = GetSysEventService();
    if (service == nullptr) {
        HILOG_ERROR(LOG_CORE, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }

    std::vector<SysEventQueryRule> hospRules;
    ConvertQueryRule(rules, hospRules);

    sptr<HiSysEventQueryProxy> spCallBack(new OHOS::HiviewDFX::HiSysEventQueryProxy(callback));

    SysEventServiceProxy sysEventService(service);
    QueryArgument queryArgument(arg.beginTime, arg.endTime, arg.maxEvents, arg.fromSeq, arg.toSeq);
    return sysEventService.Query(queryArgument, hospRules, spCallBack);
}

int64_t HiSysEventDelegate::Export(const struct QueryArg& arg, const std::vector<QueryRule>& rules) const
{
    auto service = GetSysEventService();
    if (service == nullptr) {
        HILOG_ERROR(LOG_CORE, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    auto res = CreateHiviewDir();
    if (!res) {
        HILOG_ERROR(LOG_CORE, "Fail to create hiview dir.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    res = SetDirPermission();
    if (!res) {
        HILOG_ERROR(LOG_CORE, "Fail to set ACL permission.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    std::vector<SysEventQueryRule> hospRules;
    ConvertQueryRule(rules, hospRules);
    SysEventServiceProxy sysEventService(service);
    QueryArgument queryArgument(arg.beginTime, arg.endTime, arg.maxEvents, arg.fromSeq, arg.toSeq);
    return sysEventService.Export(queryArgument, hospRules);
}

int64_t HiSysEventDelegate::Subscribe(const std::vector<QueryRule>& rules) const
{
    auto service = GetSysEventService();
    if (service == nullptr) {
        HILOG_ERROR(LOG_CORE, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }

    auto res = CreateHiviewDir();
    if (!res) {
        HILOG_ERROR(LOG_CORE, "Fail to create hiview dir.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    res = SetDirPermission();
    if (!res) {
        HILOG_ERROR(LOG_CORE, "Fail to set ACL permission.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }

    std::vector<SysEventQueryRule> hospRules;
    ConvertQueryRule(rules, hospRules);

    SysEventServiceProxy sysEventService(service);
    return sysEventService.AddSubscriber(hospRules);
}

int32_t HiSysEventDelegate::Unsubscribe() const
{
    auto service = GetSysEventService();
    if (service == nullptr) {
        HILOG_ERROR(LOG_CORE, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    SysEventServiceProxy sysEventService(service);
    return sysEventService.RemoveSubscriber();
}

HiSysEventDelegate::~HiSysEventDelegate()
{
}

void HiSysEventDelegate::ConvertListenerRule(const std::vector<ListenerRule>& rules,
    std::vector<SysEventRule>& sysRules) const
{
    for_each(rules.cbegin(), rules.cend(), [&sysRules](const ListenerRule& rule) {
        if (rule.GetTag().empty()) {
            sysRules.emplace_back(rule.GetDomain(), rule.GetEventName(), rule.GetRuleType(), rule.GetEventType());
        } else {
            sysRules.emplace_back(rule.GetTag(), rule.GetRuleType(), rule.GetEventType());
        }
    });
}

void HiSysEventDelegate::ConvertQueryRule(const std::vector<QueryRule>& rules,
    std::vector<SysEventQueryRule>& sysRules) const
{
    for_each(rules.cbegin(), rules.cend(), [&sysRules](const QueryRule &rule) {
        std::vector<std::string> events;
        auto eventList = rule.GetEventList();
        for_each(eventList.cbegin(), eventList.cend(), [&](const std::string &event) {
            events.push_back(event);
        });
        sysRules.emplace_back(rule.GetDomain(), events, rule.GetRuleType(), rule.GetEventType(), rule.GetCondition());
    });
}

void HiSysEventDelegate::BinderFunc()
{
    IPCSkeleton::JoinWorkThread();
}

sptr<IRemoteObject> HiSysEventDelegate::GetSysEventService() const
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        return nullptr;
    }
    return sam->CheckSystemAbility(DFX_SYS_EVENT_SERVICE_ABILITY_ID);
}

bool HiSysEventDelegate::CreateHiviewDir() const
{
    if (FileUtil::IsFileExists(EVENT_DIR)) {
        return true;
    }
    if (!FileUtil::ForceCreateDirectory(EVENT_DIR)) {
        HILOG_ERROR(LOG_CORE, "failed to create event dir, errno=%{public}d.", errno);
        return false;
    }
    return true;
}

bool HiSysEventDelegate::SetDirPermission() const
{
#ifdef STORAGE_SERVICE_ENABLE
    int aclBaseRet = AclSetAccess(BASE_DIR, PARENT_DIR_PERMISSION);
    if (aclBaseRet != ACL_SUCC) {
        HILOG_ERROR(LOG_CORE, "Set ACL failed , baseDirPath= %{public}s ret = %{public}d!!!!",
            BASE_DIR.c_str(), aclBaseRet);
        return false;
    }
    int aclCacheRet = AclSetAccess(CACHE_DIR, PARENT_DIR_PERMISSION);
    if (aclCacheRet != ACL_SUCC) {
        HILOG_ERROR(LOG_CORE, "Set ACL failed , cacheDirPath= %{public}s ret = %{public}d!!!!",
            CACHE_DIR.c_str(), aclCacheRet);
        return false;
    }
    int aclHiviewRet = AclSetAccess(HIVIEW_DIR, PARENT_DIR_PERMISSION);
    if (aclHiviewRet != ACL_SUCC) {
        HILOG_ERROR(LOG_CORE, "Set ACL failed , hiviewDirPath= %{public}s ret = %{public}d!!!!",
            HIVIEW_DIR.c_str(), aclHiviewRet);
        return false;
    }
    int aclRet = AclSetAccess(EVENT_DIR, SUB_DIR_PERMISSION);
    if (aclRet != ACL_SUCC) {
        HILOG_ERROR(LOG_CORE, "Set ACL failed , eventDirPath= %{public}s ret = %{public}d!!!!",
            EVENT_DIR.c_str(), aclRet);
        return false;
    }
    return true;
#else
    return false;
#endif // STORAGE_SERVICE_ENABLE
}

} // namespace HiviewDFX
} // namespace OHOS
