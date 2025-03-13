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
#include "write_controller.h"
#include "stringfilter.h"
#include "hisysevent.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"
#include "write_controller.h"

using namespace OHOS::HiviewDFX;

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "ANI_HISYSEVENT"

// common api error
constexpr int32_t ERR_NON_SYS_APP_PERMISSION = 202;
constexpr int32_t TESTLINE = 10;
const std::string CLASS_NAME_INT = "Lstd/core/Int;";
const std::string CLASS_NAME_BOOLEAN = "Lstd/core/Boolean;";
const std::string CLASS_NAME_DOUBLE = "Lstd/core/Double;";
const std::string CLASS_NAME_STRING = "Lstd/core/String;";
const std::string CLASS_NAME_BIGINT = "Lescompat/BigInt;";
const std::string CLASS_NAME_ARRAY = "Lescompat/Array;";
const std::string CLASS_NAME_ITERATOR = "Lescompat/Iterator;";
const std::string CLASS_NAME_HISYSEVENTANI = "L@ohos/hiSysEvent/HiSysEventAni;";
const std::string ENUM_NAME_EVENTTYPE = "L@ohos/hiSysEvent/hiSysEvent/#EventType;";
const std::string FUNC_NAME_UNBOXED = "unboxed";
const std::string FUNC_NAME_GETLONG = "getLong";
const std::string FUNC_NAME_ISARRAY = "isArray";
const std::string FUNC_NAME_NEXT = "next";
const std::string PROP_NAME_LENGTH = "length";

const std::pair<std::string, AniArgsType> OBJECT_TYPE[] = {
    {CLASS_NAME_INT, AniArgsType::ANI_INT},
    {CLASS_NAME_BOOLEAN, AniArgsType::ANI_BOOLEAN},
    {CLASS_NAME_DOUBLE, AniArgsType::ANI_NUMBER},
    {CLASS_NAME_STRING, AniArgsType::ANI_STRING},
    {CLASS_NAME_BIGINT, AniArgsType::ANI_BIGINT},
};

static bool RefIsUndefined(ani_env *env, ani_ref ref)
{
    ani_boolean isUndefined = false;
    env->Reference_IsUndefined(ref, &isUndefined);
    return isUndefined;
}

static std::pair<std::string, AniArgsType> GetArgType(ani_env *env, ani_object elementObj)
{
    if (RefIsUndefined(env, static_cast<ani_ref>(elementObj))) {
        return {"Undefined", AniArgsType::ANI_UNDEFINED};
    }
    for (const auto& objType : OBJECT_TYPE) {
        ani_class cls;
        if (ANI_OK != env->FindClass(objType.first.c_str(), &cls)) {
            continue;
        }
        ani_boolean isInstance;
        if (ANI_OK != env->Object_InstanceOf(elementObj, cls, &isInstance)) {
            continue;
        }
        if (static_cast<bool>(isInstance)) {
            return objType;
        }
    }
    return {"UNEXPECTED", AniArgsType::ANI_UNEXPECTED};
}

static bool ParseBoolValue(ani_env *env, ani_class booleanClass, ani_object elementObj, bool defaultvalue = false)
{
    ani_method unboxedMethod;
    if (ANI_OK != env->Class_FindMethod(booleanClass, FUNC_NAME_UNBOXED.c_str(), ":Z", &unboxedMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_UNBOXED.c_str());
    }
    ani_boolean booleanVal = static_cast<ani_boolean>(defaultvalue);
    if (ANI_OK != env->Object_CallMethod_Boolean(elementObj, unboxedMethod, &booleanVal)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Boolean %{public}s Failed", FUNC_NAME_UNBOXED.c_str());
    }
    return static_cast<bool>(booleanVal);
}

static double ParseNumberValue(ani_env *env, ani_class numberClass, ani_object elementObj)
{
    ani_method unboxedMethod;
    if (ANI_OK != env->Class_FindMethod(numberClass, FUNC_NAME_UNBOXED.c_str(), ":D", &unboxedMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_UNBOXED.c_str());
    }
    ani_double doubleVal = static_cast<ani_double>(0.0);
    if (ANI_OK != env->Object_CallMethod_Double(elementObj, unboxedMethod, &doubleVal)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Double %{public}s Failed", FUNC_NAME_UNBOXED.c_str());
    }
    return static_cast<double>(doubleVal);
}

static std::string ParseStringValue(ani_env *env, ani_string aniStr)
{
    ani_size strSize;
    if (ANI_OK != env->String_GetUTF8Size(aniStr, &strSize)) {
        HILOG_ERROR(LOG_CORE, "String_GetUTF8Size Failed");
    }
    std::vector<char> buffer(strSize + 1);
    char* utf8Buffer = buffer.data();
    ani_size bytesWritten = 0;
    if (ANI_OK != env->String_GetUTF8(aniStr, utf8Buffer, strSize + 1, &bytesWritten)) {
        HILOG_ERROR(LOG_CORE, "String_GetUTF8 Failed");
    }
    utf8Buffer[bytesWritten] = '\0';
    std::string content = std::string(utf8Buffer);
    return content;
}

static int64_t ParseBigIntValue(ani_env *env, ani_class bigIntClass, ani_object elementObj)
{
    ani_method getLongMethod;
    if (ANI_OK != env->Class_FindMethod(bigIntClass, FUNC_NAME_GETLONG.c_str(), ":J", &getLongMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_GETLONG.c_str());
    }
    ani_long longVal;
    if (ANI_OK != env->Object_CallMethod_Long(elementObj, getLongMethod, &longVal)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Long %{public}s Failed", FUNC_NAME_GETLONG.c_str());
    }
    return static_cast<int64_t>(longVal);
}

static void AppendArray(ani_env *env, ani_class cls, ani_object valueObj, AniArgsType type, ParamArray& paramArray)
{
    switch (type) {
        case AniArgsType::ANI_BOOLEAN:
            paramArray.boolArray.emplace_back(ParseBoolValue(env, cls, valueObj));
            break;
        case AniArgsType::ANI_NUMBER:
            paramArray.numberArray.emplace_back(ParseNumberValue(env, cls, valueObj));
            break;
        case AniArgsType::ANI_STRING:
            paramArray.stringArray.emplace_back(ParseStringValue(env, static_cast<ani_string>(valueObj)));
            break;
        case AniArgsType::ANI_BIGINT:
            paramArray.bigintArray.emplace_back(ParseBigIntValue(env, cls, valueObj));
            break;
        default:
            HILOG_ERROR(LOG_CORE, "Unexpected type");
            break;
    }
}

static void AddArrayParam(AniArgsType type, std::string key, ParamArray& paramArray, HiSysEventInfo& info)
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
            break;
    }
}

static void AddArrayParamToEventInfo(ani_env *env, ani_ref arrayRef, std::string key, HiSysEventInfo& info)
{
    ani_size size;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array_ref>(arrayRef), &size)) {
        HILOG_ERROR(LOG_CORE, "Array_GetLength Failed");
        return;
    }
    ParamArray paramArray;
    AniArgsType elementType = AniArgsType::ANI_UNEXPECTED;
    for (ani_size i = 0; i < static_cast<ani_size>(size); i++) {
        ani_ref valueRef;
        if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(arrayRef), static_cast<ani_size>(i), &valueRef)) {
            HILOG_ERROR(LOG_CORE, "Array_Get_Ref %{public}d Failed", static_cast<int32_t>(i));
            return;
        }
        ani_object valueObj = static_cast<ani_object>(valueRef);
        std::pair<std::string, AniArgsType> objType = GetArgType(env, static_cast<ani_object>(arrayRef));
        if (elementType == AniArgsType::ANI_UNEXPECTED && objType.second != AniArgsType::ANI_UNEXPECTED) {
            elementType = objType.second;
        }
        if (objType.second == AniArgsType::ANI_UNEXPECTED || objType.second == AniArgsType::ANI_UNDEFINED) {
            return;
        }
        ani_class cls;
        env->FindClass(objType.first.c_str(), &cls);
        AppendArray(env, cls, valueObj, objType.second, paramArray);
    }
    AddArrayParam(elementType, key, paramArray, info);
    return;
}

static void AddParamToEventInfo(ani_env *env, ani_ref element, std::string key, HiSysEventInfo& info)
{
    ani_object elementObj = static_cast<ani_object>(element);
    std::pair<std::string, AniArgsType> objType = GetArgType(env, elementObj);
    if (objType.second == AniArgsType::ANI_UNEXPECTED || objType.second == AniArgsType::ANI_UNDEFINED) {
        return;
    }
    ani_class cls;
    env->FindClass(objType.first.c_str(), &cls);
    switch (objType.second) {
        case AniArgsType::ANI_BOOLEAN:
            info.params[key] = ParseBoolValue(env, cls, elementObj);
            break;
        case AniArgsType::ANI_NUMBER:
            info.params[key] = ParseNumberValue(env, cls, elementObj);
            break;
        case AniArgsType::ANI_STRING:
            info.params[key] = ParseStringValue(env, static_cast<ani_string>(elementObj));
            break;
        case AniArgsType::ANI_BIGINT:
            info.params[key] = ParseBigIntValue(env, cls, elementObj);
            break;
        default:
            HILOG_ERROR(LOG_CORE, "Unexpected type");
            break;
    }
    return;
}

static ani_status EnumEventTypeGetInt32(ani_env *env, ani_enum_item enumItem, int32_t &value)
{
    ani_int aniInt {};
    if (ANI_OK != env->EnumItem_GetValue_Int(enumItem, &aniInt)) {
        HILOG_ERROR(LOG_CORE, "EnumItem_GetValue_Int Failed");
        return ANI_ERROR;
    }
    value = static_cast<int32_t>(aniInt);
    return ANI_OK;
}

static bool IsArray(ani_env *env, ani_object object)
{
    ani_boolean isArray = ANI_FALSE;
    ani_class cls;
    if (ANI_OK != env->FindClass(CLASS_NAME_ARRAY.c_str(), &cls)) {
        HILOG_ERROR(LOG_CORE, "FindEnum %{public}s Failed", CLASS_NAME_ARRAY.c_str());
        return static_cast<bool>(isArray);
    }
    ani_static_method static_method;
    if (ANI_OK != env->Class_FindStaticMethod(cls, FUNC_NAME_ISARRAY.c_str(), nullptr, &static_method)) {
        HILOG_ERROR(LOG_CORE, "Class_FindStaticMethod %{public}s Failed", FUNC_NAME_ISARRAY.c_str());
        return static_cast<bool>(isArray);
    }
    if (ANI_OK != env->Class_CallStaticMethod_Boolean(cls, static_method, &isArray, object)) {
        HILOG_ERROR(LOG_CORE, "Class_CallStaticMethod_Boolean %{public}s Failed", FUNC_NAME_ISARRAY.c_str());
        return false;
    }
    return static_cast<bool>(isArray);
}

static bool GetKeyValueFormIterArray(ani_env *env, ani_ref param, std::string& key, ani_ref& value)
{
    ani_size keyIndex = 0;
    ani_size valueIndex = keyIndex + 1;
    ani_ref keyRef;
    ani_array_ref paramArrayRef = static_cast<ani_array_ref>(param);
    if (ANI_OK != env->Array_Get_Ref(paramArrayRef, keyIndex, &keyRef)) {
        HILOG_ERROR(LOG_CORE, "Array_Get_Ref %{public}d Failed", static_cast<int32_t>(keyIndex));
        return false;
    }
    key = ParseStringValue(env, static_cast<ani_string>(keyRef));
    if (ANI_OK != env->Array_Get_Ref(paramArrayRef, valueIndex, &value)) {
        HILOG_ERROR(LOG_CORE, "Array_Get_Ref %{public}d Failed", static_cast<int32_t>(valueIndex));
        return false;
    }
    return true;
}

static bool ParseParamsIterValue(ani_env *env, ani_ref param, HiSysEventInfo& eventInfo)
{
    ani_object paramObj = static_cast<ani_object>(param);
    if (IsArray(env, paramObj)) {
        std::string key;
        ani_ref value;
        GetKeyValueFormIterArray(env, param, key, value);
        ani_object valueObj = static_cast<ani_object>(value);
        if (IsArray(env, valueObj)) {
            AddArrayParamToEventInfo(env, value, key, eventInfo);
        } else {
            AddParamToEventInfo(env, value, key, eventInfo);
        }
    }
    return true;
}

static bool ParseParamsRecord(ani_env *env, ani_ref params, int32_t& paramsSize, ani_ref& paramsIter)
{
    ani_class recordCls;
    const char *recordClassName = "Lescompat/Record;";
    if (ANI_OK != env->FindClass(recordClassName, &recordCls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", recordClassName);
        return false;
    }

    ani_double recordSize;
    if (ANI_OK != env->Object_GetPropertyByName_Double(static_cast<ani_object>(params), "size", &recordSize)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Double size Failed");
        return false;
    }
    paramsSize = static_cast<int32_t>(recordSize);

    ani_method entriesMethod;
    if (ANI_OK != env->Class_FindMethod(recordCls, "entries", nullptr, &entriesMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod entries Failed");
        return false;
    }
    if (ANI_OK != env->Object_CallMethod_Ref(static_cast<ani_object>(params), entriesMethod, &paramsIter)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Ref entries Failed");
        return false;
    }
    return true;
}

static bool ParseParamsIter(ani_env *env, int32_t paramsSize, ani_ref paramsIter,
    HiSysEventInfo& eventInfo)
{
    ani_class IteratorCls;
    if (ANI_OK != env->FindClass(CLASS_NAME_ITERATOR.c_str(), &IteratorCls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_ITERATOR.c_str());
        return false;
    }
    ani_method nextMethod;
    if (ANI_OK != env->Class_FindMethod(IteratorCls, FUNC_NAME_NEXT.c_str(), nullptr, &nextMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_NEXT.c_str());
        return false;
    }
    ani_object paramsIterObj = static_cast<ani_object>(paramsIter);
    for (int32_t i = 0; i < static_cast<int32_t>(paramsSize); i++) {
        ani_ref paramsIterResult;
        if (ANI_OK != env->Object_CallMethod_Ref(paramsIterObj, nextMethod, &paramsIterResult)) {
            HILOG_ERROR(LOG_CORE, "Object_CallMethod_Ref next Failed");
            return false;
        }
        ani_object paramsIterResultObj = static_cast<ani_object>(paramsIterResult);
        ani_boolean done;
        if (ANI_OK != env->Object_GetPropertyByName_Boolean(paramsIterResultObj, "done", &done)) {
            HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Boolean done Failed");
            return false;
        }
        ani_ref value;
        if (ANI_OK != env->Object_GetPropertyByName_Ref(paramsIterResultObj, "value", &value)) {
            HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref value Failed");
            return false;
        }
        if (!done) {
            ParseParamsIterValue(env, value, eventInfo);
        }
    }
    return true;
}

static bool ParseParamsInSysEventInfo(ani_env *env, ani_ref params, HiSysEventInfo& eventInfo)
{
    if (RefIsUndefined(env, params)) {
        return true;
    }
    int32_t paramsSize = 0;
    ani_ref paramsIter;
    ParseParamsRecord(env, params, paramsSize, paramsIter);
    ParseParamsIter(env, paramsSize, paramsIter, eventInfo);
    return true;
}

static bool ParseSysEventInfo(ani_env *env, ani_object info, HiSysEventInfo& eventInfo)
{
    ani_ref domainRef;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "domain", &domainRef)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref domain Failed");
        return false;
    }
    eventInfo.domain = ParseStringValue(env, static_cast<ani_string>(domainRef));
    ani_ref nameRef;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "name", &nameRef)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref name Failed");
        return false;
    }
    eventInfo.name = ParseStringValue(env, static_cast<ani_string>(nameRef));
    ani_ref eventTypeRef;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "eventType", &eventTypeRef)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref eventType Failed");
        return false;
    }
    ani_enum_item eventTypeItem = static_cast<ani_enum_item>(eventTypeRef);
    int32_t eventTypeValue;
    if (ANI_OK != EnumEventTypeGetInt32(env, eventTypeItem, eventTypeValue)) {
        HILOG_ERROR(LOG_CORE, "EnumEventTypeGetInt32 Failed");
        return false;
    }
    eventInfo.eventType = static_cast<HiSysEvent::EventType>(eventTypeValue);
    ani_ref paramsRef;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "params", &paramsRef)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref params failed");
        return false;
    }
    if (!ParseParamsInSysEventInfo(env, paramsRef, eventInfo)) {
        HILOG_ERROR(LOG_CORE, "ParseParamsInSysEventInfo params failed");
        return false;
    }
    return true;
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

int HiSysEventAni::WriteInner(HiSysEventInfo eventInfo)
{
    JsCallerInfo jsCallerInfo("TestFunc", TESTLINE);
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

static bool IsSystemAppCall()
{
    uint64_t tokenId = OHOS::IPCSkeleton::GetCallingFullTokenID();
    return OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

ani_int HiSysEventAni::Write(ani_env *env, [[maybe_unused]] ani_object object, ani_object info)
{
    if (!IsSystemAppCall()) {
        HILOG_ERROR(LOG_CORE, "Permission denied. ""System api can be invoked only by system applications.");
        return ERR_NON_SYS_APP_PERMISSION;
    }

    HiSysEventInfo eventInfo;
    if (!RefIsUndefined(env, static_cast<ani_ref>(info))) {
        if (!ParseSysEventInfo(env, info, eventInfo)) {
            HILOG_ERROR(LOG_CORE, "ParseSysEventInfo failed.");
        }
    }
    return HiSysEventAni::WriteInner(eventInfo);
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return (ani_status)ANI_ERROR;
    }
    auto status = HiSysEventAni::HiSysEventInit(env);
    if (status != ANI_OK) {
        return status;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}

ani_status HiSysEventAni::HiSysEventInit(ani_env *env)
{
    ani_class cls;
    if (ANI_OK != env->FindClass(CLASS_NAME_HISYSEVENTANI.c_str(), &cls)) {
        return (ani_status)ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {"write", nullptr, reinterpret_cast<void *>(HiSysEventAni::Write) },
    };

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        return (ani_status)ANI_ERROR;
    };
    return ANI_OK;
}
