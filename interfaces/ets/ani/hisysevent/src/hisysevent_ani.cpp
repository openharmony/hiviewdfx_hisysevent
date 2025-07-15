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
#include <charconv>
#include <iostream>

#include "hisysevent_ani.h"

#include "hilog/log.h"
#include "hilog/log_cpp.h"
#include "hisysevent_ani_util.h"
#include "write_controller.h"
#include "stringfilter.h"
#include "hisysevent.h"
#include "ret_code.h"
#include "ret_def.h"
#include "write_controller.h"

using namespace OHOS::HiviewDFX;

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "ANI_HISYSEVENT"

namespace {
constexpr char CLASS_NAME_HISYSEVENTANI[] = "L@ohos/hiSysEvent/HiSysEventAni;";

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
            HILOG_ERROR(LOG_CORE, "Object_InstanceOf %{public}s failed", objType.first);
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

static AniArgsType GetArrayType(ani_env *env, ani_array_ref arrayRef)
{
    ani_size index = 0;
    ani_ref valueRef {};
    if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(arrayRef), index, &valueRef)) {
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
    AniArgsType arrayType = GetArrayType(env, static_cast<ani_array_ref>(arrayRef));
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

static void ParseSysEventInfo(ani_env *env, ani_object info, HiSysEventInfo& eventInfo)
{
    ani_ref domainRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "domain", &domainRef)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref domain failed");
        return;
    }
    eventInfo.domain = HiSysEventAniUtil::ParseStringValue(env, domainRef);

    ani_ref nameRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "name", &nameRef)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref name failed");
        return;
    }
    eventInfo.name = HiSysEventAniUtil::ParseStringValue(env, nameRef);

    ani_ref eventTypeRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "eventType", &eventTypeRef)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref eventType failed");
        return;
    }
    ani_enum_item eventTypeItem = static_cast<ani_enum_item>(eventTypeRef);
    int32_t eventTypeValue;
    if (ANI_OK != env->EnumItem_GetValue_Int(eventTypeItem, &eventTypeValue)) {
        HILOG_ERROR(LOG_CORE, "EnumItem_GetValue_Int failed");
        return;
    }
    eventInfo.eventType = static_cast<HiSysEvent::EventType>(eventTypeValue);

    ani_ref paramsRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "params", &paramsRef)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref params failed");
        return;
    }
    if (!ParseParamsInSysEventInfo(env, paramsRef, eventInfo)) {
        HILOG_ERROR(LOG_CORE, "ParseParamsInSysEventInfo params failed");
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
    auto lineNumInfo = lineInfos[LINE_INDEX];
    int64_t lineNumParsed = DEFAULT_LINE_NUM;
    auto lineNumParsedRet = std::from_chars(lineNumInfo.c_str(), lineNumInfo.c_str() + lineNumInfo.size(),
        lineNumParsed);
    if (lineNumParsedRet.ec != std::errc()) {
        HILOG_WARN(LOG_CORE, "js function line number is invalid.");
        callerInfo.second = DEFAULT_LINE_NUM;
        return;
    }
    callerInfo.second = lineNumParsed;
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
        status = env->Array_Get_Ref(static_cast<ani_array_ref>(stackTraceElementArray), i, &stackTraceElementRef);
        if (ANI_OK != status) {
            HILOG_ERROR(LOG_CORE, "get %{public}zu item from array failed", i);
        }
        ani_ref stackTraceStr;
        status = env->Object_CallMethodByName_Ref(static_cast<ani_object>(stackTraceElementRef), "toString", nullptr,
            &stackTraceStr);
        if (ANI_OK != status) {
            HILOG_ERROR(LOG_CORE, "call method toString failed");
        }
        stackTrace = HiSysEventAniUtil::ParseStringValue(env, stackTraceStr);
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

ani_object HiSysEventAni::Write(ani_env *env, [[maybe_unused]] ani_object object, ani_object info)
{
    if (!HiSysEventAniUtil::IsSystemAppCall()) {
        return HiSysEventAniUtil::WriteResult(env, {AniError::ERR_NON_SYS_APP_PERMISSION,
            "Permission denied. "
            "System api can be invoked only by system applications."});
    }

    HiSysEventInfo eventInfo;
    ParseSysEventInfo(env, info, eventInfo);

    int32_t eventWroteResult =  HiSysEventAni::WriteInner(env, eventInfo);
    return HiSysEventAniUtil::WriteResult(env, HiSysEventAniUtil::GetErrorDetailByRet(eventWroteResult));
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_HISYSEVENTANI, &cls)) {
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {"write", nullptr, reinterpret_cast<void *>(HiSysEventAni::Write) },
    };
    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        return ANI_ERROR;
    };
    *result = ANI_VERSION_1;
    return ANI_OK;
}
