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

#include "application_context.h"
#include "context.h"
#include "file_util.h"
#include "hilog/log.h"
#include "hisysevent_listener_proxy.h"
#include "hisysevent_query_proxy.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "query_argument.h"
#include "ret_code.h"
#include "storage_acl.h"
#include "sys_event_service_proxy.h"
#include "system_ability_definition.h"

using namespace std;
using namespace OHOS::StorageDaemon;

namespace OHOS {
namespace HiviewDFX {

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HiView-HiSysEventDelegate" };
const std::string HIVIEW_DIR = "/hiview/";
const std::string EVENT_DIR = "/event/";
const std::string PARENT_DIR_PERMISSION = "g:1201:x";
const std::string SUB_DIR_PERMISSION = "g:1201:rwx";
constexpr int ACL_SUCC = 0;
}

int32_t HiSysEventDelegate::AddListener(const std::shared_ptr<HiSysEventBaseListener> listener,
    const std::vector<ListenerRule>& rules)
{
    auto service = GetSysEventService();
    if (service == nullptr) {
        HiLog::Error(LABEL, "Fail to get service.");
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
        HiLog::Error(LABEL, "Fail to get service.");
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
        HiLog::Error(LABEL, "Fail to get service.");
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
        HiLog::Error(LABEL, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }

    std::vector<SysEventQueryRule> hospRules;
    ConvertQueryRule(rules, hospRules);

    sptr<HiSysEventQueryProxy> spCallBack =
        new OHOS::HiviewDFX::HiSysEventQueryProxy(callback);

    SysEventServiceProxy sysEventService(service);
    QueryArgument queryArgument(arg.beginTime, arg.endTime, arg.maxEvents, arg.fromSeq, arg.toSeq);
    return sysEventService.Query(queryArgument, hospRules, spCallBack);
}

int64_t HiSysEventDelegate::Export(const struct QueryArg& arg, const std::vector<QueryRule>& rules) const
{
    auto service = GetSysEventService();
    if (service == nullptr) {
        HiLog::Error(LABEL, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    auto res = CreateHiviewDir();
    if (!res) {
        HiLog::Error(LABEL, "Fail to create hiview dir.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    res = SetDirPermission();
    if (!res) {
        HiLog::Error(LABEL, "Fail to set ACL permission.");
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
        HiLog::Error(LABEL, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }

    auto res = CreateHiviewDir();
    if (!res) {
        HiLog::Error(LABEL, "Fail to create hiview dir.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    res = SetDirPermission();
    if (!res) {
        HiLog::Error(LABEL, "Fail to set ACL permission.");
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
        HiLog::Error(LABEL, "Fail to get service.");
        return ERR_SYS_EVENT_SERVICE_NOT_FOUND;
    }
    SysEventServiceProxy sysEventService(service);
    return sysEventService.RemoveSubscriber();
}

HiSysEventDelegate::~HiSysEventDelegate()
{
    HiLog::Info(LABEL, "HiSysEventDelegate destructor");
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
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        HiLog::Error(LABEL, "GetHiViewDir Context is null.");
        return false;
    }
    if (context->GetCacheDir().empty()) {
        HiLog::Error(LABEL, "GetHiViewDir The files dir obtained from context is empty.");
        return false;
    }
    std::string eventDirPath = context->GetCacheDir() + HIVIEW_DIR + EVENT_DIR;
    if (FileUtil::IsFileExists(eventDirPath)) {
        return true;
    }
    if (!FileUtil::ForceCreateDirectory(eventDirPath)) {
        HiLog::Error(LABEL, "failed to create event dir, errno=%{public}d.", errno);
        return false;
    }
    return true;
}

bool HiSysEventDelegate::SetDirPermission() const
{
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        HiLog::Error(LABEL, "GetHiViewDir Context is null.");
        return false;
    }
    if (context->GetCacheDir().empty()) {
        HiLog::Error(LABEL, "GetHiViewDir The files dir obtained from context is empty.");
        return false;
    }
    std::string baseDirPath = context->GetBaseDir();
    std::string cacheDirPath = context->GetCacheDir();
    std::string hiviewDirPath = context->GetCacheDir() + HIVIEW_DIR;
    std::string eventDirPath = context->GetCacheDir() + HIVIEW_DIR + EVENT_DIR;

    int aclBaseRet = AclSetAccess(baseDirPath, PARENT_DIR_PERMISSION);
    if (aclBaseRet != ACL_SUCC) {
        HiLog::Error(LABEL, "Set ACL failed , baseDirPath= %{public}s ret = %{public}d!!!!",
            baseDirPath.c_str(), aclBaseRet);
        return false;
    }
    int aclCacheRet = AclSetAccess(cacheDirPath, PARENT_DIR_PERMISSION);
    if (aclCacheRet != ACL_SUCC) {
        HiLog::Error(LABEL, "Set ACL failed , cacheDirPath= %{public}s ret = %{public}d!!!!",
            cacheDirPath.c_str(), aclCacheRet);
        return false;
    }
    int aclHiviewRet = AclSetAccess(hiviewDirPath, PARENT_DIR_PERMISSION);
    if (aclHiviewRet != ACL_SUCC) {
        HiLog::Error(LABEL, "Set ACL failed , hiviewDirPath= %{public}s ret = %{public}d!!!!",
            hiviewDirPath.c_str(), aclHiviewRet);
        return false;
    }
    int aclRet = AclSetAccess(eventDirPath, SUB_DIR_PERMISSION);
    if (aclRet != ACL_SUCC) {
        HiLog::Error(LABEL, "Set ACL failed , eventDirPath= %{public}s ret = %{public}d!!!!",
            eventDirPath.c_str(), aclRet);
        return false;
    }
    return true;
}

} // namespace HiviewDFX
} // namespace OHOS
