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
constexpr int32_t CALLERINFOINE = 10;
constexpr char CLASS_NAME_HISYSEVENTANI[] = "L@ohos/hiSysEvent/HiSysEventAni;";

const std::pair<const char*, AniArgsType> OBJECT_TYPE[] = {
    {CLASS_NAME_BOOLEAN, AniArgsType::ANI_BOOLEAN},
    {CLASS_NAME_DOUBLE, AniArgsType::ANI_NUMBER},
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

static void AppendArray(ani_env *env, ani_ref value, AniArgsType type, ParamArray& paramArray)
{
    switch (type) {
        case AniArgsType::ANI_BOOLEAN:
            paramArray.boolArray.emplace_back(HiSysEventAniUtil::ParseBoolValue(env, value));
            break;
        case AniArgsType::ANI_NUMBER:
            paramArray.numberArray.emplace_back(HiSysEventAniUtil::ParseNumberValue(env, value));
            break;
        case AniArgsType::ANI_STRING:
            paramArray.stringArray.emplace_back(HiSysEventAniUtil::ParseStringValue(env, value));
            break;
        case AniArgsType::ANI_BIGINT:
            paramArray.bigintArray.emplace_back(HiSysEventAniUtil::ParseBigintValue(env, value));
            break;
        default:
            HILOG_ERROR(LOG_CORE, "Unexpected type");
            break;
    }
}

static bool AddArrayParam(AniArgsType type, const std::string& key, ParamArray& paramArray, HiSysEventInfo& info)
{
    switch (type) {
        case AniArgsType::ANI_BOOLEAN:
            info.params[key] = paramArray.boolArray;
            break;
        case AniArgsType::ANI_NUMBER:
            info.params[key] = paramArray.numberArray;
            break;
        case AniArgsType::ANI_STRING:
            info.params[key] = paramArray.stringArray;
            break;
        case AniArgsType::ANI_BIGINT:
            info.params[key] = paramArray.bigintArray;
            break;
        default:
            HILOG_ERROR(LOG_CORE, "Unexpected type");
            return false;
    }
    return true;
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

static bool AddArrayParamToEventInfo(ani_env *env, const std::string& key, ani_ref arrayRef, HiSysEventInfo& info)
{
    ani_size size = 0;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array_ref>(arrayRef), &size)) {
        HILOG_ERROR(LOG_CORE, "get array length failed");
        return false;
    }
    ParamArray paramArray;
    AniArgsType arrayType = GetArrayType(env, static_cast<ani_array_ref>(arrayRef));
    if (!IsValidParamType(arrayType)) {
        return false;
    }
    for (ani_size i = 0; i < size; i++) {
        ani_ref valueRef {};
        if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(arrayRef), i, &valueRef)) {
            HILOG_ERROR(LOG_CORE, "get array %{public}zu failed", i);
            return false;
        }
        if (GetArgType(env, static_cast<ani_object>(valueRef)) != arrayType) {
            HILOG_ERROR(LOG_CORE, "element type in arrry not same");
            return false;
        }
        AppendArray(env, valueRef, arrayType, paramArray);
    }
    return AddArrayParam(arrayType, key, paramArray, info);
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
        case AniArgsType::ANI_NUMBER:
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

int32_t HiSysEventAni::WriteInner(const HiSysEventInfo &eventInfo)
{
    JsCallerInfo jsCallerInfo("NotFoundCallerInfo", CALLERINFOINE);
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

    int32_t eventWroteResult =  HiSysEventAni::WriteInner(eventInfo);
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
