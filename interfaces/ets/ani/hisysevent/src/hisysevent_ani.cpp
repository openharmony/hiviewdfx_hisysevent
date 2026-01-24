/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <array>
#include <iostream>

#include "hisysevent_ani.h"
#include "ani_hisysevent_querier.h"
#include "ani_hisysevent_listener.h"
#include "ani_callback_context.h"
#include "def.h"
#include "hisysevent_base_manager.h"
#include "hisysevent_rules.h"
#include "hilog/log.h"
#include "hilog/log_cpp.h"
#include "hisysevent_ani_util.h"
#include "hisysevent.h"
#include "ret_code.h"
#include "ret_def.h"
#include "rule_type.h"
#include "write_controller.h"
#include "stringfilter.h"

using namespace OHOS::HiviewDFX;
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "ANI_HISYSEVENT"
namespace {
using ANI_LISTENER_PAIR = std::pair<pid_t, std::shared_ptr<AniHiSysEventListener>>;
using ANI_QUERIER_PAIR = std::pair<pid_t, std::shared_ptr<AniHiSysEventQuerier>>;
constexpr long long DEFAULT_TIME_STAMP = -1;
constexpr int DEFAULT_EVENT_COUNT = 1000;
constexpr int TIME_STAMP_LENGTH = 13;
std::mutex g_listenerMapMutex;
std::unordered_map<ani_ref, ANI_LISTENER_PAIR> listeners;
std::mutex g_querierMapMutex;
std::unordered_map<ani_ref, ANI_QUERIER_PAIR> queriers;
constexpr int64_t DEFAULT_LINE_NUM = -1;
constexpr int FUNC_NAME_INDEX = 1;
constexpr int LINE_INFO_INDEX = 2;
constexpr int LINE_INDEX = 1;
constexpr char CALL_FUNC_INFO_DELIMITER = ' ';
constexpr char CALL_LINE_INFO_DELIMITER = ':';
constexpr char PATH_DELIMITER = '/';
const std::pair<const char*, AniArgsType> OBJECT_TYPE[] = {
    {CLASS_NAME_BOOLEAN, AniArgsType::ANI_BOOLEAN},
    {CLASS_NAME_INT, AniArgsType::ANI_INT},
    {CLASS_NAME_DOUBLE, AniArgsType::ANI_DOUBLE},
    {CLASS_NAME_STRING, AniArgsType::ANI_STRING},
    {CLASS_NAME_BIGINT, AniArgsType::ANI_BIGINT},
};
}

static AniArgsType GetArgType(ani_env *env, ani_object elementObj)
{
    if (HiSysEventAniUtil::IsRefUndefined(env, static_cast<ani_ref>(elementObj))) {
        return AniArgsType::ANI_UNDEFINED;
    }
    for (const auto& objType : OBJECT_TYPE) {
        ani_class cls {};
        if (ANI_OK != env->FindClass(objType.first, &cls)) {
            continue;
        }
        ani_boolean isInstance = ANI_FALSE;
        if (ANI_OK != env->Object_InstanceOf(elementObj, cls, &isInstance)) {
            HILOG_ERROR(LOG_CORE, "check instance failed for type '%{public}s'", objType.first);
            continue;
        }
        if (static_cast<bool>(isInstance)) {
            return objType.second;
        }
    }
    return AniArgsType::ANI_UNKNOWN;
}

static bool IsValidParamType(AniArgsType type)
{
    return (type > static_cast<int32_t>(AniArgsType::ANI_UNKNOWN) &&
            type < static_cast<int32_t>(AniArgsType::ANI_UNDEFINED));
}

static AniArgsType GetArrayType(ani_env *env, ani_array arrayRef)
{
    ani_size index = 0;
    ani_ref valueRef {};
    if (ANI_OK != env->Array_Get(arrayRef, index, &valueRef)) {
        HILOG_ERROR(LOG_CORE, "fail to get first element in array.");
        return AniArgsType::ANI_UNKNOWN;
    }
    return GetArgType(env, static_cast<ani_object>(valueRef));
}

static bool AddArrayParamToEventInfoExec(ani_env *env, const std::string& key, ani_ref arrayRef,
    AniArgsType arrayType, HiSysEventInfo& info)
{
    switch (arrayType) {
        case AniArgsType::ANI_BOOLEAN: {
            std::vector<bool> bools;
            if (HiSysEventAniUtil::GetBooleans(env, arrayRef, bools)) {
                info.params[key] = bools;
                return true;
            }
            break;
        }
        case AniArgsType::ANI_INT: {
            std::vector<double> doubles;
            if (HiSysEventAniUtil::GetIntsToDoubles(env, arrayRef, doubles)) {
                info.params[key] = doubles;
                return true;
            }
            break;
        }
        case AniArgsType::ANI_DOUBLE: {
            std::vector<double> doubles;
            if (HiSysEventAniUtil::GetDoubles(env, arrayRef, doubles)) {
                info.params[key] = doubles;
                return true;
            }
            break;
        }
        case AniArgsType::ANI_STRING: {
            std::vector<std::string> strs;
            if (HiSysEventAniUtil::GetStrings(env, arrayRef, strs)) {
                info.params[key] = strs;
                return true;
            }
            break;
        }
        case AniArgsType::ANI_BIGINT:{
            std::vector<int64_t> bigints;
            if (HiSysEventAniUtil::GetBigints(env, arrayRef, bigints)) {
                info.params[key] = bigints;
                return true;
            }
            break;
        }
        default:
            HILOG_ERROR(LOG_CORE, "Unexpected type");
            return false;
    }
    return false;
}

static bool AddArrayParamToEventInfo(ani_env *env, const std::string& key, ani_ref arrayRef, HiSysEventInfo& info)
{
    AniArgsType arrayType = GetArrayType(env, static_cast<ani_array>(arrayRef));
    if (!IsValidParamType(arrayType)) {
        return false;
    }
    return AddArrayParamToEventInfoExec(env, key, arrayRef, arrayType, info);
}

static bool AddParamToEventInfo(ani_env *env, const std::string& key, ani_ref value, HiSysEventInfo& info)
{
    AniArgsType type = GetArgType(env, static_cast<ani_object>(value));
    if (!IsValidParamType(type)) {
        return false;
    }
    switch (type) {
        case AniArgsType::ANI_BOOLEAN:
            info.params[key] = HiSysEventAniUtil::ParseBoolValue(env, value);
            break;
        case AniArgsType::ANI_INT:
            info.params[key] = static_cast<double>(HiSysEventAniUtil::ParseIntValue(env, value));
            break;
        case AniArgsType::ANI_DOUBLE:
            info.params[key] = HiSysEventAniUtil::ParseNumberValue(env, value);
            break;
        case AniArgsType::ANI_STRING:
            info.params[key] = HiSysEventAniUtil::ParseStringValue(env, value);
            break;
        case AniArgsType::ANI_BIGINT:
            info.params[key] = HiSysEventAniUtil::ParseBigintValue(env, value);
            break;
        default:
            HILOG_ERROR(LOG_CORE, "Unexpected type");
            return false;
    }
    return true;
}

static bool AddParamsToEventInfo(ani_env *env, std::pair<std::string, ani_ref> param, HiSysEventInfo& eventInfo)
{
    if (HiSysEventAniUtil::IsArray(env, static_cast<ani_object>(param.second))) {
        if (!AddArrayParamToEventInfo(env, param.first, param.second, eventInfo)) {
            return false;
        }
    } else if (!AddParamToEventInfo(env, param.first, param.second, eventInfo)) {
        return false;
    }
    return true;
}

static bool ParseParamsInSysEventInfo(ani_env *env, ani_ref params, HiSysEventInfo& eventInfo)
{
    if (HiSysEventAniUtil::IsRefUndefined(env, params)) {
        return true;
    }
    std::map<std::string, ani_ref> paramsMap = HiSysEventAniUtil::ParseRecord(env, static_cast<ani_object>(params));
    for (const auto &param : paramsMap) {
        if (!AddParamsToEventInfo(env, param, eventInfo)) {
            return false;
        }
    }
    return true;
}

static ListenerRule ParseListenerRule(ani_env *env, const ani_object value)
{
    ani_ref domainRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(value, "domain", &domainRef)) {
        HILOG_ERROR(LOG_CORE, "get property domain failed");
        return ListenerRule("", RuleType::WHOLE_WORD);
    }
    std::string domain = HiSysEventAniUtil::ParseStringValue(env, domainRef);
    ani_ref nameRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(value, "name", &nameRef)) {
        HILOG_ERROR(LOG_CORE, "get property name failed");
        return ListenerRule("", RuleType::WHOLE_WORD);
    }
    std::string name = HiSysEventAniUtil::ParseStringValue(env, nameRef);
    ani_ref tagRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(value, "tag", &tagRef)) {
        HILOG_ERROR(LOG_CORE, "get property tag failed");
        return ListenerRule("", RuleType::WHOLE_WORD);
    }
    std::string tag = "";
    if (!HiSysEventAniUtil::IsRefUndefined(env, tagRef)) {
        tag = HiSysEventAniUtil::ParseStringValue(env, tagRef);
    }
    ani_ref ruleTypeRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(value, "ruleType", &ruleTypeRef)) {
        HILOG_ERROR(LOG_CORE, "get property ruleType failed");
        return ListenerRule("", RuleType::WHOLE_WORD);
    }
    ani_enum_item ruleTypeItem = static_cast<ani_enum_item>(ruleTypeRef);
    int32_t ruleTypeValue;
    if (ANI_OK != env->EnumItem_GetValue_Int(ruleTypeItem, &ruleTypeValue)) {
        HILOG_ERROR(LOG_CORE, "get enum failed");
        return ListenerRule("", RuleType::WHOLE_WORD);
    }
    int32_t ruleType = static_cast<RuleType>(ruleTypeValue);
    return ListenerRule(domain, name, tag, RuleType(ruleType));
}

static void ParseWatcher(ani_env *env, ani_object watcher, std::vector<ListenerRule>& listenerRules)
{
    ani_ref rulesRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(watcher, "rules", &rulesRef)) {
        HILOG_ERROR(LOG_CORE, "get property rules failed");
        return;
    }
    ani_size size;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array>(rulesRef), &size)) {
        HILOG_ERROR(LOG_CORE, "get array failed");
        return;
    }
    for (ani_size i = 0; i < size ; i++) {
        ani_ref value;
        auto status = env->Array_Get(static_cast<ani_array>(rulesRef), i, &value);
        if (status != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "get %{public}zu item from array failed", i);
            return;
        }
        listenerRules.emplace_back(ParseListenerRule(env, static_cast<ani_object>(value)));
    }
}

static QueryRule ParseQueryRule(ani_env *env, const ani_object value)
{
    ani_ref namesRef {};
    std::vector<std::string> names;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(value, "names", &namesRef)) {
        HILOG_ERROR(LOG_CORE, "get property names failed");
        return QueryRule("", names);
    }
    ani_size size;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array>(namesRef), &size)) {
        HILOG_ERROR(LOG_CORE, "get array failed");
        return QueryRule("", names);
    }
    for (ani_size i = 0; i < size ; i++) {
        ani_ref value;
        auto status = env->Array_Get(static_cast<ani_array>(namesRef), i, &value);
        if (status != ANI_OK) {
            return QueryRule("", names);
        }
        std::string result = HiSysEventAniUtil::ParseStringValue(env, value);
        names.emplace_back(HiSysEventAniUtil::ParseStringValue(env, value));
    }
    ani_ref domainRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(value, "domain", &domainRef)) {
        HILOG_ERROR(LOG_CORE, "get property domain failed");
        return QueryRule("", names);
    }
    std::string domain = HiSysEventAniUtil::ParseStringValue(env, domainRef);
    ani_ref conditionRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(value, "condition", &conditionRef)) {
        HILOG_ERROR(LOG_CORE, "get property condition failed");
    }
    std::string condition = "";
    if (!HiSysEventAniUtil::IsRefUndefined(env, conditionRef)) {
        condition = HiSysEventAniUtil::ParseStringValue(env, conditionRef);
    }
    return QueryRule(domain, names, RuleType::WHOLE_WORD, 0, condition);
}

static bool IsQueryRuleValid(ani_env *env, const QueryRule& rule)
{
    auto domain = rule.GetDomain();
    if (!StringFilter::GetInstance().IsValidName(domain, MAX_DOMAIN_LENGTH)) {
        HiSysEventAniUtil::ThrowErrorByRet(env, AniInnerError::ERR_INVALID_DOMAIN_IN_QUERY_RULE);
        return false;
    }
    auto names = rule.GetEventList();
    if (std::any_of(names.begin(), names.end(), [] (auto& name) {
        return !StringFilter::GetInstance().IsValidName(name, MAX_EVENT_NAME_LENGTH);
    })) {
        HiSysEventAniUtil::ThrowErrorByRet(env, AniInnerError::ERR_INVALID_EVENT_NAME_IN_QUERY_RULE);
        return false;
    }
    return true;
}

static int32_t ParseQueryRules(ani_env *env, ani_array rulesAni, std::vector<QueryRule> &rules)
{
    ani_size size;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array>(rulesAni), &size)) {
        HILOG_ERROR(LOG_CORE, "get array failed");
        return ERR_ANI_PARSED_FAILED;
    }
    for (ani_size i = 0; i < size ; i++) {
        ani_ref value;
        auto status = env->Array_Get(static_cast<ani_array>(rulesAni), i, &value);
        if (status != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "get %{public}zu item from array failed", i);
            return ERR_ANI_PARSED_FAILED;
        }
        auto queryRule = ParseQueryRule(env, static_cast<ani_object>(value));
        if (IsQueryRuleValid(env, queryRule)) {
            rules.emplace_back(queryRule);
        } else {
            return ERR_ANI_PARSED_FAILED;
        }
    }
    return ANI_SUCCESS;
}

static void ParseQueryArg(ani_env *env, ani_object queryArgAni, QueryArg &queryArg)
{
    ani_long beginTimeRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Long(queryArgAni, "beginTime", &beginTimeRef)) {
        HILOG_ERROR(LOG_CORE, "get property beginTime failed,");
    }
    queryArg.beginTime = static_cast<int64_t>(beginTimeRef);
    ani_long endTimeRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Long(queryArgAni, "endTime", &endTimeRef)) {
        HILOG_ERROR(LOG_CORE, "get property endTime failed");
    }
    queryArg.endTime = static_cast<int64_t>(endTimeRef);
    ani_long maxEventsRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Long(queryArgAni, "maxEvents", &maxEventsRef)) {
        HILOG_ERROR(LOG_CORE, "get property maxEvents failed");
    }
    queryArg.maxEvents = static_cast<int64_t>(maxEventsRef);
    ani_ref fromSeqRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(queryArgAni, "fromSeq", &fromSeqRef)) {
        HILOG_ERROR(LOG_CORE, "get property fromSeq failed");
    }
    if (!HiSysEventAniUtil::IsRefUndefined(env, fromSeqRef)) {
        queryArg.fromSeq = HiSysEventAniUtil::ParseBigintValue(env, fromSeqRef);
    }
    ani_ref toSeqRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(queryArgAni, "toSeq", &toSeqRef)) {
        HILOG_ERROR(LOG_CORE, "get property toSeq failed");
    }
    if (!HiSysEventAniUtil::IsRefUndefined(env, toSeqRef)) {
        queryArg.toSeq = HiSysEventAniUtil::ParseBigintValue(env, toSeqRef);
    }
}

static void ParseSysEventInfo(ani_env *env, ani_object info, HiSysEventInfo& eventInfo)
{
    ani_ref domainRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "domain", &domainRef)) {
        HILOG_ERROR(LOG_CORE, "get property domain failed");
        return;
    }
    eventInfo.domain = HiSysEventAniUtil::ParseStringValue(env, domainRef);
    ani_ref nameRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "name", &nameRef)) {
        HILOG_ERROR(LOG_CORE, "get property name failed");
        return;
    }
    eventInfo.name = HiSysEventAniUtil::ParseStringValue(env, nameRef);
    ani_ref eventTypeRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "eventType", &eventTypeRef)) {
        HILOG_ERROR(LOG_CORE, "get property eventType failed");
        return;
    }
    ani_enum_item eventTypeItem = static_cast<ani_enum_item>(eventTypeRef);
    int32_t eventTypeValue;
    if (ANI_OK != env->EnumItem_GetValue_Int(eventTypeItem, &eventTypeValue)) {
        HILOG_ERROR(LOG_CORE, "get enum failed");
        return;
    }
    eventInfo.eventType = static_cast<HiSysEvent::EventType>(eventTypeValue);
    ani_ref paramsRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "params", &paramsRef)) {
        HILOG_ERROR(LOG_CORE, "get property params failed");
        return;
    }
    if (!ParseParamsInSysEventInfo(env, paramsRef, eventInfo)) {
        HILOG_ERROR(LOG_CORE, "get property params failed");
        return;
    }
}

static void CheckThenWriteSysEvent(HiSysEventInfo iptEventInfo, JsCallerInfo iptJsCallerInfo, uint64_t &iptTimeStamp)
{
    auto eventInfo = iptEventInfo;
    auto jsCallerInfo = iptJsCallerInfo;
    ControlParam param {
        .period = HISYSEVENT_DEFAULT_PERIOD,
        .threshold = HISYSEVENT_DEFAULT_THRESHOLD,
    };
    CallerInfo info = {
        .func = jsCallerInfo.first.c_str(),
        .line = jsCallerInfo.second,
        .timeStamp = iptTimeStamp,
    };
    iptTimeStamp = WriteController::CheckLimitWritingEvent(param, eventInfo.domain.c_str(),
        eventInfo.name.c_str(), info);
}

static void Split(const std::string& origin, char delimiter, std::vector<std::string>& ret)
{
    std::string::size_type start = 0;
    std::string::size_type end = origin.find(delimiter);
    while (end != std::string::npos) {
        if (end == start) {
            start++;
            end = origin.find(delimiter, start);
            continue;
        }
        ret.emplace_back(origin.substr(start, end - start));
        start = end + 1;
        end = origin.find(delimiter, start);
    }
    if (start != origin.length()) {
        ret.emplace_back(origin.substr(start));
    }
}

static void ParseCallerInfoFromStackTrace(const std::string& stackTrace, JsCallerInfo& callerInfo)
{
    if (stackTrace.empty()) {
        HILOG_ERROR(LOG_CORE, "js stack trace is invalid.");
        return;
    }
    std::vector<std::string> callInfos;
    Split(stackTrace, CALL_FUNC_INFO_DELIMITER, callInfos);
    if (callInfos.size() <= FUNC_NAME_INDEX) {
        HILOG_ERROR(LOG_CORE, "js function name parsed failed.");
        return;
    }
    callerInfo.first = callInfos[FUNC_NAME_INDEX];
    if (callInfos.size() <= LINE_INFO_INDEX) {
        HILOG_ERROR(LOG_CORE, "js function line info parsed failed.");
        return;
    }
    std::string callInfo = callInfos[LINE_INFO_INDEX];
    std::vector<std::string> lineInfos;
    Split(callInfo, CALL_LINE_INFO_DELIMITER, lineInfos);
    if (lineInfos.size() <= LINE_INDEX) {
        HILOG_ERROR(LOG_CORE, "js function line number parsed failed.");
        return;
    }
    if (callerInfo.first == "anonymous") {
        auto fileName = lineInfos[LINE_INDEX - 1];
        auto pos = fileName.find_last_of(PATH_DELIMITER);
        callerInfo.first = (pos == std::string::npos) ? fileName : fileName.substr(++pos);
    }
    auto lineInfo = lineInfos[LINE_INDEX];
    if (std::any_of(lineInfo.begin(), lineInfo.end(), [] (auto& c) {
        return !isdigit(c);
    })) {
        callerInfo.second = DEFAULT_LINE_NUM;
        return;
    }
    callerInfo.second = static_cast<int64_t>(std::stoll(lineInfos[LINE_INDEX]));
}

static void GetStack(ani_env *env, std::string &stackTrace)
{
    ani_class stackTraceCls;
    ani_status status = env->FindClass(CLASS_NAME_STACKTRACE, &stackTraceCls);
    if (ANI_OK != status) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_STACKTRACE);
    }
    ani_ref stackTraceElementArray;
    status = env->Class_CallStaticMethodByName_Ref(stackTraceCls, "provisionStackTrace", nullptr,
        &stackTraceElementArray);
    if (ANI_OK != status) {
        HILOG_ERROR(LOG_CORE, "call method provisionStackTrace failed");
    }
    ani_size length = 0;
    status = env->Array_GetLength(static_cast<ani_array>(stackTraceElementArray), &length);
    if (ANI_OK != status) {
        HILOG_ERROR(LOG_CORE, "get length failed");
    }
    for (ani_size i = 0; i < length; i++) {
        ani_ref stackTraceElementRef = nullptr;
        status = env->Array_Get(static_cast<ani_array>(stackTraceElementArray), i, &stackTraceElementRef);
        if (ANI_OK != status) {
            HILOG_ERROR(LOG_CORE, "get %{public}zu item from array failed", i);
        }
        ani_ref stackTraceStr;
        status = env->Object_CallMethodByName_Ref(static_cast<ani_object>(stackTraceElementRef), "toString", nullptr,
            &stackTraceStr);
        if (ANI_OK != status) {
            HILOG_ERROR(LOG_CORE, "call method toString failed");
        }
        stackTrace = HiSysEventAniUtil::AniStringToStdString(env, static_cast<ani_string>(stackTraceStr));
    }
}

static void ParseCallerInfo(ani_env *env, JsCallerInfo& callerInfo)
{
    std::string stackTrace;
    GetStack(env, stackTrace);
    ParseCallerInfoFromStackTrace(stackTrace, callerInfo);
}

int32_t HiSysEventAni::WriteInner(ani_env *env, const HiSysEventInfo &eventInfo)
{
    JsCallerInfo jsCallerInfo;
    ParseCallerInfo(env, jsCallerInfo);
    uint64_t timeStamp = WriteController::GetCurrentTimeMills();
    CheckThenWriteSysEvent(eventInfo, jsCallerInfo, timeStamp);
    if (!StringFilter::GetInstance().IsValidName(eventInfo.domain, MAX_DOMAIN_LENGTH)) {
        return HiSysEvent::ExplainThenReturnRetCode(ERR_DOMAIN_NAME_INVALID);
    }
    if (!StringFilter::GetInstance().IsValidName(eventInfo.name, MAX_EVENT_NAME_LENGTH)) {
        return HiSysEvent::ExplainThenReturnRetCode(ERR_EVENT_NAME_INVALID);
    }
    HiSysEvent::EventBase eventBase(eventInfo.domain, eventInfo.name, eventInfo.eventType, timeStamp);
    HiSysEvent::WritebaseInfo(eventBase);
    if (HiSysEvent::IsError(eventBase)) {
        return HiSysEvent::ExplainThenReturnRetCode(eventBase.GetRetCode());
    }
    AppendParams(eventBase, eventInfo.params);
    HiSysEvent::InnerWrite(eventBase);
    if (HiSysEvent::IsError(eventBase)) {
        return HiSysEvent::ExplainThenReturnRetCode(eventBase.GetRetCode());
    }
    HiSysEvent::SendSysEvent(eventBase);
    return eventBase.GetRetCode();
}

ani_object HiSysEventAni::WriteSync(ani_env *env, ani_object info)
{
    if (!HiSysEventAniUtil::IsSystemAppCall()) {
        return HiSysEventAniUtil::WriteResult(env, {AniError::ERR_NON_SYS_APP_PERMISSION,
            "Permission denied. "
            "System api can be invoked only by system applications."});
    }
    HiSysEventInfo eventInfo;
    ParseSysEventInfo(env, info, eventInfo);
    int32_t eventWroteResult = HiSysEventAni::WriteInner(env, eventInfo);
    return HiSysEventAniUtil::WriteResult(env, HiSysEventAniUtil::GetErrorDetailByRet(eventWroteResult));
}

void HiSysEventAni::AddWatcher(ani_env *env, ani_object watcher)
{
    if (!HiSysEventAniUtil::IsSystemAppCall()) {
        HiSysEventAniUtil::ThrowAniError(env, AniError::ERR_NON_SYS_APP_PERMISSION, "Permission denied. "
            "System api can be invoked only by system applications.");
        return;
    }
    ani_vm *vm = HiSysEventAniUtil::GetAniVm(env);
    std::vector<ListenerRule> rules;
    ParseWatcher(env, watcher, rules);
    CallbackContextAni *callbackContextAni = new CallbackContextAni();
    callbackContextAni->vm = vm;
    callbackContextAni->threadId = getproctid();
    ani_ref listenerRef;
    env->GlobalReference_Create(static_cast<ani_ref>(watcher), &listenerRef);
    callbackContextAni->ref = listenerRef;
    std::shared_ptr<AniHiSysEventListener> listener = std::make_shared<AniHiSysEventListener>(callbackContextAni);
    auto ret = HiSysEventBaseManager::AddListener(listener, rules);
    if (ret != ANI_SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to add event listener, result code is %{public}d.", ret);
        delete callbackContextAni;
        HiSysEventAniUtil::ThrowErrorByRet(env, ret);
        return;
    }
    std::lock_guard<std::mutex> lock(g_listenerMapMutex);
    listeners[callbackContextAni->ref] = std::make_pair(callbackContextAni->threadId, listener);
}

void HiSysEventAni::RemoveWatcher(ani_env *env, ani_object watcher)
{
    if (!HiSysEventAniUtil::IsSystemAppCall()) {
        HiSysEventAniUtil::ThrowAniError(env, AniError::ERR_NON_SYS_APP_PERMISSION, "Permission denied. "
            "System api can be invoked only by system applications.");
        return;
    }
    std::unordered_map<ani_ref, std::pair<pid_t, std::shared_ptr<AniHiSysEventListener>>>::iterator iter;
    std::shared_ptr<AniHiSysEventListener> listenerPtr = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_listenerMapMutex);
        iter = HiSysEventAniUtil::CompareAndReturnCacheItem<AniHiSysEventListener>(env, watcher, listeners);
        if (iter == listeners.end()) {
            HILOG_ERROR(LOG_CORE, "listener not exist.");
            HiSysEventAniUtil::ThrowErrorByRet(env, ERR_ANI_LISTENER_NOT_FOUND);
            return;
        }
        listenerPtr = iter->second.second;
    }
    if (auto ret = HiSysEventBaseManager::RemoveListener(listenerPtr);
        ret != ANI_SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to remove event listener, result code is %{public}d.", ret);
        HiSysEventAniUtil::ThrowErrorByRet(env, ret);
        return;
    }
    std::lock_guard<std::mutex> lock(g_listenerMapMutex);
    listeners.erase(iter);
}

void HiSysEventAni::Query(ani_env *env, ani_object queryArgAni, ani_array rulesAni, ani_object querierAni)
{
    if (!HiSysEventAniUtil::IsSystemAppCall()) {
        HiSysEventAniUtil::ThrowAniError(env, AniError::ERR_NON_SYS_APP_PERMISSION, "Permission denied. "
            "System api can be invoked only by system applications.");
        return;
    }
    ani_vm *vm = HiSysEventAniUtil::GetAniVm(env);
    QueryArg queryArg = { DEFAULT_TIME_STAMP, DEFAULT_TIME_STAMP, DEFAULT_EVENT_COUNT };
    ParseQueryArg(env, queryArgAni, queryArg);
    std::vector<QueryRule> rules;
    auto result = ParseQueryRules(env, rulesAni, rules);
    if (result != SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to parse query rules, result code is %{public}d.", result);
        return;
    }
    CallbackContextAni *callbackContextAni = new CallbackContextAni();
    callbackContextAni->vm = vm;
    callbackContextAni->threadId = getproctid();
    ani_ref querierRef;
    env->GlobalReference_Create(static_cast<ani_ref>(querierAni), &querierRef);
    callbackContextAni->ref = querierRef;
    std::shared_ptr<AniHiSysEventQuerier> querier = std::make_shared<AniHiSysEventQuerier>(callbackContextAni,
        [] (ani_vm *vm, ani_ref ref) {
            ani_object querierObj = static_cast<ani_object>(ref);
            std::lock_guard<std::mutex> lock(g_querierMapMutex);
            ani_env *env = HiSysEventAniUtil::AttachAniEnv(vm);
            auto iter = HiSysEventAniUtil::CompareAndReturnCacheItem<AniHiSysEventQuerier>(env, querierObj, queriers);
            if (iter != queriers.end()) {
                queriers.erase(iter);
            }
            HiSysEventAniUtil::DetachAniEnv(vm);
        });
    auto ret = HiSysEventBaseManager::Query(queryArg, rules, querier);
    if (ret != ANI_SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to query hisysevent, result code is %{public}d.", ret);
        HiSysEventAniUtil::ThrowErrorByRet(env, ret);
        return;
    }
    std::lock_guard<std::mutex> lock(g_querierMapMutex);
    queriers[callbackContextAni->ref] = std::make_pair(callbackContextAni->threadId, querier);
}

ani_double HiSysEventAni::ExportSysEvents(ani_env *env, ani_object queryArgAni, ani_array rulesAni)
{
    if (!HiSysEventAniUtil::IsSystemAppCall()) {
        HiSysEventAniUtil::ThrowAniError(env, AniError::ERR_NON_SYS_APP_PERMISSION, "Permission denied. "
            "System api can be invoked only by system applications.");
        return static_cast<ani_double>(0);
    }
    QueryArg queryArg = { DEFAULT_TIME_STAMP, DEFAULT_TIME_STAMP, DEFAULT_EVENT_COUNT };
    ParseQueryArg(env, queryArgAni, queryArg);
    std::vector<QueryRule> rules;
    auto result = ParseQueryRules(env, rulesAni, rules);
    if (result != SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to parse query rules, result code is %{public}d.", result);
        return static_cast<ani_double>(result);
    }
    auto ret = HiSysEventBaseManager::Export(queryArg, rules);
    if (std::to_string(ret).length() < TIME_STAMP_LENGTH) {
        HILOG_ERROR(LOG_CORE, "failed to export event");
        int32_t retCode = static_cast<int32_t>(ret);
        HiSysEventAniUtil::ThrowErrorByRet(env, retCode);
        return static_cast<ani_double>(0);
    }
    return static_cast<ani_double>(ret);
}

ani_double HiSysEventAni::Subscribe(ani_env *env, ani_array rulesAni)
{
    if (!HiSysEventAniUtil::IsSystemAppCall()) {
        HiSysEventAniUtil::ThrowAniError(env, AniError::ERR_NON_SYS_APP_PERMISSION, "Permission denied. "
            "System api can be invoked only by system applications.");
        return static_cast<ani_double>(0);
    }
    std::vector<QueryRule> rules;
    auto result = ParseQueryRules(env, rulesAni, rules);
    if (result != SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to parse query rules, result code is %{public}d.", result);
        return static_cast<ani_double>(result);
    }
    auto ret = HiSysEventBaseManager::Subscribe(rules);
    if (std::to_string(ret).length() < TIME_STAMP_LENGTH) {
        HILOG_ERROR(LOG_CORE, "failed to subscribe event.");
        int32_t retCode = static_cast<int32_t>(ret);
        HiSysEventAniUtil::ThrowErrorByRet(env, retCode);
        return static_cast<ani_double>(0);
    }
    return static_cast<ani_double>(ret);
}

void HiSysEventAni::Unsubscribe(ani_env *env)
{
    if (!HiSysEventAniUtil::IsSystemAppCall()) {
        HiSysEventAniUtil::ThrowAniError(env, AniError::ERR_NON_SYS_APP_PERMISSION, "Permission denied. "
            "System api can be invoked only by system applications.");
        return;
    }
    auto ret = HiSysEventBaseManager::Unsubscribe();
    if (ret != ANI_SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to unsubscribe, result code is %{public}d.", ret);
        int32_t retCode = static_cast<int32_t>(ret);
        HiSysEventAniUtil::ThrowErrorByRet(env, retCode);
    }
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    ani_namespace ns {};
    if (ANI_OK != env->FindNamespace(CLASS_NAME_HISYSEVENTANI, &ns)) {
        return ANI_INVALID_ARGS;
    }
    std::array methods = {
        ani_native_function {"writeSync", nullptr, reinterpret_cast<void *>(HiSysEventAni::WriteSync) },
        ani_native_function {"addWatcher", nullptr, reinterpret_cast<void *>(HiSysEventAni::AddWatcher) },
        ani_native_function {"removeWatcher", nullptr, reinterpret_cast<void *>(HiSysEventAni::RemoveWatcher) },
        ani_native_function {"query", nullptr, reinterpret_cast<void *>(HiSysEventAni::Query) },
        ani_native_function {"exportSysEvents", nullptr, reinterpret_cast<void *>(HiSysEventAni::ExportSysEvents) },
        ani_native_function {"subscribe", nullptr, reinterpret_cast<void *>(HiSysEventAni::Subscribe) },
        ani_native_function {"unsubscribe", nullptr, reinterpret_cast<void *>(HiSysEventAni::Unsubscribe) },
    };
    if (ANI_OK != env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size())) {
        return ANI_INVALID_TYPE;
    };
    *result = ANI_VERSION_1;
    return ANI_OK;
}
