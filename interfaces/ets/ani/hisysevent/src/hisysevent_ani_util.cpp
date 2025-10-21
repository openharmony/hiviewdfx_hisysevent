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

#include "hisysevent_ani_util.h"
#include "def.h"
#include "hilog/log_cpp.h"
#include "ipc_skeleton.h"
#include "ret_code.h"
#include "ret_def.h"
#include "json/json.h"
#include "tokenid_kit.h"

using namespace OHOS::HiviewDFX;
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "ANI_HISYSEVENT_UTIL"
namespace {
constexpr char DOMAIN__KEY[] = "domain_";
constexpr char NAME__KEY[] = "name_";
constexpr char TYPE__KEY[] = "type_";
const std::string INVALID_KEY_TYPE_ARR[] = {
    "[object Object]",
    "null",
    "()",
    ","
};
constexpr char CONSTRUCTOR_NAME[] = "<ctor>";
}

static const std::map<EventTypeAni, int32_t> ANI_EVENTTYPE_INDEX_MAP = {
    {EventTypeAni::FAULT, 0},
    {EventTypeAni::STATISTIC, 1},
    {EventTypeAni::SECURITY, 2},
    {EventTypeAni::BEHAVIOR, 3},
};

bool HiSysEventAniUtil::CheckKeyTypeString(const std::string& str)
{
    bool ret = true;
    for (auto invalidType : INVALID_KEY_TYPE_ARR) {
        if (str.find(invalidType) != std::string::npos) {
            ret = false;
            break;
        }
    }
    return ret;
}

bool HiSysEventAniUtil::IsArray(ani_env *env, ani_object object)
{
    ani_boolean IsArray = ANI_FALSE;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return IsArray;
    }

    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_ARRAY, &cls)) {
        return false;
    }
    ani_static_method static_method {};
    if (ANI_OK != env->Class_FindStaticMethod(cls, "isArray", nullptr, &static_method)) {
        return false;
    }
    if (ANI_OK != env->Class_CallStaticMethod_Boolean(cls, static_method, &IsArray, object)) {
        return false;
    }
    return static_cast<bool>(IsArray);
}

bool HiSysEventAniUtil::IsRefUndefined(ani_env *env, ani_ref ref)
{
    ani_boolean isUndefined = ANI_FALSE;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return isUndefined;
    }

    env->Reference_IsUndefined(ref, &isUndefined);
    return isUndefined;
}

bool HiSysEventAniUtil::IsSystemAppCall()
{
    uint64_t tokenId = OHOS::IPCSkeleton::GetCallingFullTokenID();
    return OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

int64_t HiSysEventAniUtil::ParseBigintValue(ani_env *env, ani_ref elementRef)
{
    ani_long longNum = 0;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return longNum;
    }

    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_BIGINT, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_BIGINT);
        return static_cast<int64_t>(longNum);
    }
    ani_method getLongMethod {};
    if (ANI_OK != env->Class_FindMethod(cls, FUNC_NAME_GETLONG, ":l", &getLongMethod)) {
        HILOG_ERROR(LOG_CORE, "find method %{public}s failed", FUNC_NAME_GETLONG);
        return static_cast<int64_t>(longNum);
    }
    if (ANI_OK != env->Object_CallMethod_Long(static_cast<ani_object>(elementRef), getLongMethod, &longNum)) {
        HILOG_ERROR(LOG_CORE, "call method %{public}s failed", FUNC_NAME_GETLONG);
        return static_cast<int64_t>(longNum);
    }
    return static_cast<int64_t>(longNum);
}

bool HiSysEventAniUtil::ParseBoolValue(ani_env *env, ani_ref elementRef)
{
    ani_boolean booleanVal = ANI_FALSE;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return booleanVal;
    }

    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_BOOLEAN, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_BOOLEAN);
        return false;
    }
    ani_method unboxedMethod {};
    if (ANI_OK != env->Class_FindMethod(cls, FUNC_NAME_UNBOXED, ":z", &unboxedMethod)) {
        HILOG_ERROR(LOG_CORE, "find method %{public}s failed", FUNC_NAME_UNBOXED);
        return false;
    }
    if (ANI_OK != env->Object_CallMethod_Boolean(static_cast<ani_object>(elementRef), unboxedMethod, &booleanVal)) {
        HILOG_ERROR(LOG_CORE, "call method %{public}s failed", FUNC_NAME_UNBOXED);
        return false;
    }
    return static_cast<bool>(booleanVal);
}

int HiSysEventAniUtil::ParseIntValue(ani_env *env, ani_ref elementRef)
{
    ani_int intVal = 0;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return intVal;
    }

    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_INT, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_INT);
        return static_cast<int>(intVal);
    }
    ani_method unboxedMethod {};
    if (ANI_OK != env->Class_FindMethod(cls, FUNC_NAME_UNBOXED, ":i", &unboxedMethod)) {
        HILOG_ERROR(LOG_CORE, "find method %{public}s failed", FUNC_NAME_UNBOXED);
        return static_cast<int>(intVal);
    }
    if (ANI_OK != env->Object_CallMethod_Int(static_cast<ani_object>(elementRef), unboxedMethod, &intVal)) {
        HILOG_ERROR(LOG_CORE, "call method %{public}s failed", FUNC_NAME_UNBOXED);
        return static_cast<int>(intVal);
    }
    return static_cast<int>(intVal);
}

double HiSysEventAniUtil::ParseNumberValue(ani_env *env, ani_ref elementRef)
{
    ani_double doubleVal = 0;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return doubleVal;
    }

    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_DOUBLE, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_DOUBLE);
        return static_cast<double>(doubleVal);
    }
    ani_method unboxedMethod {};
    if (ANI_OK != env->Class_FindMethod(cls, FUNC_NAME_UNBOXED, ":d", &unboxedMethod)) {
        HILOG_ERROR(LOG_CORE, "find method %{public}s failed", FUNC_NAME_UNBOXED);
        return static_cast<double>(doubleVal);
    }
    if (ANI_OK != env->Object_CallMethod_Double(static_cast<ani_object>(elementRef), unboxedMethod, &doubleVal)) {
        HILOG_ERROR(LOG_CORE, "call method %{public}s failed", FUNC_NAME_UNBOXED);
        return static_cast<double>(doubleVal);
    }
    return static_cast<double>(doubleVal);
}

std::map<std::string, ani_ref> HiSysEventAniUtil::ParseRecord(ani_env *env, ani_ref recordRef)
{
    std::map<std::string, ani_ref> recordResult = {};
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return recordResult;
    }

    ani_ref keys {};
    if (ANI_OK != env->Object_CallMethodByName_Ref(static_cast<ani_object>(recordRef), "keys",
        ":C{std.core.IterableIterator}", &keys)) {
        HILOG_ERROR(LOG_CORE, "call method keys failed.");
        return recordResult;
    }
    ani_boolean done = ANI_FALSE;
    while (done != ANI_TRUE) {
        ani_ref next {};
        if (ANI_OK != env->Object_CallMethodByName_Ref(static_cast<ani_object>(keys), "next", nullptr, &next)) {
            HILOG_ERROR(LOG_CORE, "call method next failed.");
            break;
        }
        if (ANI_OK != env->Object_GetFieldByName_Boolean(static_cast<ani_object>(next), "done", &done)) {
            HILOG_ERROR(LOG_CORE, "get field done failed.");
            break;
        }
        if (done) {
            break;
        }
        ani_ref keyRef {};
        if (ANI_OK != env->Object_GetFieldByName_Ref(static_cast<ani_object>(next), "value", &keyRef)) {
            HILOG_ERROR(LOG_CORE, "get field value failed.");
            break;
        }
        ani_ref valueRef {};
        if (ANI_OK != env->Object_CallMethodByName_Ref(static_cast<ani_object>(recordRef),
            "$_get", nullptr, &valueRef, keyRef)) {
            HILOG_ERROR(LOG_CORE, "call method $_get failed.");
            break;
        }
        std::string keyStr = ParseStringValue(env, keyRef);
        if (!CheckKeyTypeString(keyStr)) {
            HILOG_WARN(LOG_CORE, "this param would be discarded because of invalid format of the key.");
            continue;
        }
        recordResult[keyStr] = valueRef;
    }
    return recordResult;
}

std::string HiSysEventAniUtil::ParseStringValue(ani_env *env, ani_ref aniStrRef)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return "";
    }

    ani_size strSize = 0;
    if (ANI_OK != env->String_GetUTF8Size(static_cast<ani_string>(aniStrRef), &strSize)) {
        HILOG_ERROR(LOG_CORE, "get string length failed");
        return "";
    }
    std::vector<char> buffer(strSize + 1);
    char* utf8Buffer = buffer.data();
    ani_size bytesWritten = 0;
    if (ANI_OK != env->String_GetUTF8(static_cast<ani_string>(aniStrRef), utf8Buffer, strSize + 1, &bytesWritten)) {
        HILOG_ERROR(LOG_CORE, "get string failed");
        return "";
    }
    utf8Buffer[bytesWritten] = '\0';
    std::string content = std::string(utf8Buffer);
    return content;
}

std::pair<int32_t, std::string> HiSysEventAniUtil::GetErrorDetailByRet(const int32_t retCode)
{
    HILOG_DEBUG(LOG_CORE, "original result code is %{public}d.", retCode);
    const std::unordered_map<int32_t, std::pair<int32_t, std::string>> errMap = {
        // success
        {IPC_CALL_SUCCEED, {ANI_SUCCESS, "success."}},
        // common
        {ERR_NO_PERMISSION, {AniError::ERR_PERMISSION_CHECK,
            "Permission denied. An attempt was made to read sysevent forbidden"
            " by permission: ohos.permission.READ_DFX_SYSEVENT."}},
        // write refer
        {ERR_DOMAIN_NAME_INVALID, {AniError::ERR_INVALID_DOMAIN, "Invalid event domain"}},
        {ERR_EVENT_NAME_INVALID, {AniError::ERR_INVALID_EVENT_NAME, "Invalid event name"}},
        {ERR_DOES_NOT_INIT, {AniError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_OVER_SIZE, {AniError::ERR_CONTENT_OVER_LIMIT, "The event length exceeds the limit"}},
        {ERR_KEY_NAME_INVALID, {AniError::ERR_INVALID_PARAM_NAME, "Invalid event parameter"}},
        {ERR_VALUE_LENGTH_TOO_LONG, {AniError::ERR_STR_LEN_OVER_LIMIT,
            "The size of the event parameter of the string type exceeds the limit"}},
        {ERR_KEY_NUMBER_TOO_MUCH, {AniError::ERR_PARAM_COUNT_OVER_LIMIT,
            "The number of event parameters exceeds the limit"}},
        {ERR_ARRAY_TOO_MUCH, {AniError::ERR_ARRAY_SIZE_OVER_LIMIT,
            "The number of event parameters of the array type exceeds the limit"}},
        // ipc common
        {ERR_SYS_EVENT_SERVICE_NOT_FOUND, {AniError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_CAN_NOT_WRITE_DESCRIPTOR, {AniError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_CAN_NOT_WRITE_PARCEL, {AniError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_CAN_NOT_WRITE_REMOTE_OBJECT, {AniError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_CAN_NOT_SEND_REQ, {AniError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_CAN_NOT_READ_PARCEL, {AniError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        {ERR_SEND_FAIL, {AniError::ERR_ENV_ABNORMAL, "Abnormal environment"}},
        // add watcher
        {ERR_TOO_MANY_WATCHERS, {AniError::ERR_WATCHER_COUNT_OVER_LIMIT,
            "The number of watchers exceeds the limit"}},
        {ERR_TOO_MANY_WATCH_RULES, {AniError::ERR_WATCH_RULE_COUNT_OVER_LIMIT,
            "The number of watch rules exceeds the limit"}},
        // remove watcher
        {ERR_LISTENER_NOT_EXIST, {AniError::ERR_WATCHER_NOT_EXIST, "The watcher does not exist"}},
        {ERR_ANI_LISTENER_NOT_FOUND, {AniError::ERR_WATCHER_NOT_EXIST, "The watcher does not exist"}},
        // query refer
        {ERR_TOO_MANY_QUERY_RULES, {AniError::ERR_QUERY_RULE_COUNT_OVER_LIMIT,
            "The number of query rules exceeds the limit"}},
        {ERR_TOO_MANY_CONCURRENT_QUERIES, {AniError::ERR_CONCURRENT_QUERY_COUNT_OVER_LIMIT,
            "The number of concurrent queries exceeds the limit"}},
        {ERR_QUERY_TOO_FREQUENTLY, {AniError::ERR_QUERY_TOO_FREQUENTLY, "The query frequency exceeds the limit"}},
        {AniInnerError::ERR_INVALID_DOMAIN_IN_QUERY_RULE,
            {AniError::ERR_INVALID_QUERY_RULE, "Invalid query rule"}},
        {ERR_QUERY_RULE_INVALID, {AniError::ERR_INVALID_QUERY_RULE, "Invalid query rule"}},
        {AniInnerError::ERR_INVALID_EVENT_NAME_IN_QUERY_RULE,
            {AniError::ERR_INVALID_QUERY_RULE, "Invalid query rule"}},
        // export
        {ERR_EXPORT_FREQUENCY_OVER_LIMIT, {AniError::ERR_QUERY_TOO_FREQUENTLY,
            "The query frequency exceeds the limit"}},
        // add subscriber
        {ERR_TOO_MANY_EVENTS, {AniError::ERR_QUERY_RULE_COUNT_OVER_LIMIT,
            "The number of query rules exceeds the limit"}},
        // remove subscriber
        {ERR_REMOVE_SUBSCRIBE, {AniError::ERR_REMOVE_SUBSCRIBE, "Unsubscription failed"}},
    };
    return errMap.find(retCode) == errMap.end() ?
        std::make_pair(AniError::ERR_ENV_ABNORMAL, "Abnormal environment") : errMap.at(retCode);
}

ani_object HiSysEventAniUtil::WriteResult(ani_env *env, const std::pair<int32_t, std::string>& result)
{
    ani_object resultObj {};
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return resultObj;
    }

    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_RESULTINNER, &cls)) {
        HILOG_ERROR(LOG_CORE, "find Class %{public}s failed", CLASS_NAME_RESULTINNER);
        return resultObj;
    }
    ani_method ctor {};
    if (ANI_OK != env->Class_FindMethod(cls, CONSTRUCTOR_NAME, nullptr, &ctor)) {
        HILOG_ERROR(LOG_CORE, "get method %{public}s <ctor> failed", CLASS_NAME_RESULTINNER);
        return resultObj;
    }
    if (ANI_OK != env->Object_New(cls, ctor, &resultObj)) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_RESULTINNER);
        return resultObj;
    }
    ani_method codeSetter {};
    if (ANI_OK != env->Class_FindMethod(cls, "<set>code", nullptr, &codeSetter)) {
        HILOG_ERROR(LOG_CORE, "find method <set>code failed");
    }
    if (ANI_OK != env->Object_CallMethod_Void(resultObj, codeSetter, static_cast<ani_int>(result.first))) {
        HILOG_ERROR(LOG_CORE, "call method <set>code failed");
        return resultObj;
    }
    ani_method messageSetter {};
    if (ANI_OK != env->Class_FindMethod(cls, "<set>message", nullptr, &messageSetter)) {
        HILOG_ERROR(LOG_CORE, "find method <set>message failed");
    }
    std::string message = result.second;
    ani_string message_string{};
    env->String_NewUTF8(message.c_str(), message.size(), &message_string);
    if (ANI_OK != env->Object_CallMethod_Void(resultObj, messageSetter, message_string)) {
        HILOG_ERROR(LOG_CORE, "call method <set>message failed");
        return resultObj;
    }
    return resultObj;
}

ani_object HiSysEventAniUtil::CreateDoubleInt64(ani_env *env, int64_t number)
{
    ani_object personInfoObj = nullptr;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return personInfoObj;
    }

    ani_class personCls;
    if (ANI_OK != env->FindClass(CLASS_NAME_DOUBLE, &personCls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    ani_method personInfoCtor;
    if (ANI_OK != env->Class_FindMethod(personCls, CONSTRUCTOR_NAME, "d:", &personInfoCtor)) {
        HILOG_ERROR(LOG_CORE, "find method <ctor> failed");
        return personInfoObj;
    }
    if (ANI_OK != env->Object_New(personCls, personInfoCtor, &personInfoObj, static_cast<ani_double>(number))) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    return personInfoObj;
}

ani_object HiSysEventAniUtil::CreateDouble(ani_env *env, double number)
{
    ani_object personInfoObj = nullptr;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return personInfoObj;
    }

    ani_class personCls;
    if (ANI_OK != env->FindClass(CLASS_NAME_DOUBLE, &personCls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    ani_method personInfoCtor;
    if (ANI_OK != env->Class_FindMethod(personCls, CONSTRUCTOR_NAME, "d:", &personInfoCtor)) {
        HILOG_ERROR(LOG_CORE, "find method <ctor> failed");
        return personInfoObj;
    }
    if (ANI_OK != env->Object_New(personCls, personInfoCtor, &personInfoObj, static_cast<ani_double>(number))) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    return personInfoObj;
}

ani_object HiSysEventAniUtil::CreateBool(ani_env *env, bool boolValue)
{
    ani_object personInfoObj = nullptr;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return personInfoObj;
    }

    ani_class personCls;
    if (ANI_OK != env->FindClass(CLASS_NAME_BOOLEAN, &personCls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_BOOLEAN);
        return personInfoObj;
    }
    ani_method personInfoCtor;
    if (ANI_OK != env->Class_FindMethod(personCls, CONSTRUCTOR_NAME, "d:", &personInfoCtor)) {
        HILOG_ERROR(LOG_CORE, "find method <ctor> failed");
        return personInfoObj;
    }
    if (ANI_OK != env->Object_New(personCls, personInfoCtor, &personInfoObj, static_cast<ani_boolean>(boolValue))) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_BOOLEAN);
        return personInfoObj;
    }
    return personInfoObj;
}

ani_object HiSysEventAniUtil::CreateDoubleUint64(ani_env *env, uint64_t number)
{
    ani_object personInfoObj = nullptr;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return personInfoObj;
    }

    ani_class personCls;
    if (ANI_OK != env->FindClass(CLASS_NAME_DOUBLE, &personCls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    ani_method personInfoCtor;
    if (ANI_OK != env->Class_FindMethod(personCls, CONSTRUCTOR_NAME, "d:", &personInfoCtor)) {
        HILOG_ERROR(LOG_CORE, "find method <ctor> failed");
        return personInfoObj;
    }
    if (ANI_OK != env->Object_New(personCls, personInfoCtor, &personInfoObj, static_cast<ani_double>(number))) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    return personInfoObj;
}

ani_object HiSysEventAniUtil::CreateDoubleInt32(ani_env *env, int32_t number)
{
    ani_object personInfoObj = nullptr;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return personInfoObj;
    }

    ani_class personCls;
    if (ANI_OK != env->FindClass(CLASS_NAME_DOUBLE, &personCls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    ani_method personInfoCtor;
    if (ANI_OK != env->Class_FindMethod(personCls, CONSTRUCTOR_NAME, "d:", &personInfoCtor)) {
        HILOG_ERROR(LOG_CORE, "find method <ctor> failed");
        return personInfoObj;
    }
    if (ANI_OK != env->Object_New(personCls, personInfoCtor, &personInfoObj, static_cast<ani_double>(number))) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    return personInfoObj;
}

ani_enum_item HiSysEventAniUtil::ToAniEnum(ani_env *env, EventTypeAni value)
{
    ani_enum_item aniEnumItem {};
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return aniEnumItem;
    }

    auto it = ANI_EVENTTYPE_INDEX_MAP.find(value);
    if (it == ANI_EVENTTYPE_INDEX_MAP.end()) {
        HILOG_ERROR(LOG_CORE, "find enum %{public}d failed", value);
        return aniEnumItem;
    }
    ani_int enumIndex = static_cast<ani_int>(it->second);
    ani_enum aniEnum {};
    if (ANI_OK != env->FindEnum(ENUM_NAME_EVENT_TYPE, &aniEnum)) {
        HILOG_ERROR(LOG_CORE, "find enum %{public}s failed", ENUM_NAME_EVENT_TYPE);
        return aniEnumItem;
    }
    if (env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem)) {
        HILOG_ERROR(LOG_CORE, "get enum eventType value failed");
        return aniEnumItem;
    }
    return aniEnumItem;
}

ani_object HiSysEventAniUtil::CreateDoubleUint32(ani_env *env, uint32_t number)
{
    ani_object personInfoObj = nullptr;
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return personInfoObj;
    }

    ani_class personCls;
    if (ANI_OK != env->FindClass(CLASS_NAME_DOUBLE, &personCls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    ani_method personInfoCtor;
    if (ANI_OK != env->Class_FindMethod(personCls, CONSTRUCTOR_NAME, "d:", &personInfoCtor)) {
        HILOG_ERROR(LOG_CORE, "find method <ctor> failed");
        return personInfoObj;
    }
    if (ANI_OK != env->Object_New(personCls, personInfoCtor, &personInfoObj, static_cast<ani_double>(number))) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    return personInfoObj;
}

ani_object HiSysEventAniUtil::CreateStringValue(ani_env *env, const std::string& value)
{
    ani_string valueStr {};
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return static_cast<ani_object>(valueStr);
    }

    if (ANI_OK != env->String_NewUTF8(value.c_str(), value.size(), &valueStr)) {
        HILOG_ERROR(LOG_CORE, "create new value string failed");
        return static_cast<ani_object>(valueStr);
    }
    return static_cast<ani_object>(valueStr);
}

void HiSysEventAniUtil::ThrowAniError(ani_env *env, int32_t code, const std::string &message)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return;
    }

    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_BUSINESSERROR, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_BUSINESSERROR);
        return;
    }
    ani_method ctor {};
    if (ANI_OK != env->Class_FindMethod(cls, CONSTRUCTOR_NAME, ":", &ctor)) {
        HILOG_ERROR(LOG_CORE, "find method BusinessError constructor failed");
        return;
    }
    ani_object error {};
    if (ANI_OK != env->Object_New(cls, ctor, &error)) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_BUSINESSERROR);
        return;
    }
    if (ANI_OK != env->Object_SetPropertyByName_Int(error, "code_", static_cast<ani_int>(code))) {
        HILOG_ERROR(LOG_CORE, "set property BusinessError.code_ failed");
        return;
    }
    ani_string messageRef {};
    if (ANI_OK != env->String_NewUTF8(message.c_str(), message.size(), &messageRef)) {
        HILOG_ERROR(LOG_CORE, "create new message string failed");
        return;
    }
    if (ANI_OK != env->Object_SetPropertyByName_Ref(error, "message", static_cast<ani_ref>(messageRef))) {
        HILOG_ERROR(LOG_CORE, "set property BusinessError.message failed");
        return;
    }
    if (ANI_OK != env->ThrowError(static_cast<ani_error>(error))) {
        HILOG_ERROR(LOG_CORE, "throwError ani_error object failed");
    }
}

void HiSysEventAniUtil::ThrowErrorByRet(ani_env *env, const int32_t retCode)
{
    auto detail = GetErrorDetailByRet(retCode);
    HiSysEventAniUtil::ThrowAniError(env, detail.first, detail.second);
}

static std::string TranslateKeyToAttrName(const std::string& key)
{
    if (key == DOMAIN__KEY) {
        return DOMAIN_ATTR;
    }
    if (key == NAME__KEY) {
        return NAME_ATTR;
    }
    if (key == TYPE__KEY) {
        return EVENT_TYPE_ATTR;
    }
    return "";
}

void HiSysEventAniUtil::AppendInt32PropertyToJsObject(ani_env *env, const std::string& key, const int32_t& value,
    ani_object& sysEventInfo)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return;
    }

    ani_class cls;
    if (ANI_OK != env->FindClass(CLASS_NAME_SYSEVENTINFOANI, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_SYSEVENTINFOANI);
        return;
    }
    ani_enum_item eventType_ctor = HiSysEventAniUtil::ToAniEnum(env, static_cast<EventTypeAni>(value));
    if (ANI_OK != env->Object_SetPropertyByName_Ref(sysEventInfo, "eventType", static_cast<ani_ref>(eventType_ctor))) {
        HILOG_ERROR(LOG_CORE, "set property SysEventInfoAni.eventType failed");
    }
}

void HiSysEventAniUtil::AppendStringPropertyToJsObject(ani_env *env, const std::string& key, const std::string& value,
    ani_object& sysEventInfo)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return;
    }

    ani_class cls;
    if (ANI_OK != env->FindClass(CLASS_NAME_SYSEVENTINFOANI, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_SYSEVENTINFOANI);
        return;
    }

    if (key == DOMAIN_ATTR) {
        ani_object domain_ctor = HiSysEventAniUtil::CreateStringValue(env, value);
        if (ANI_OK != env->Object_SetPropertyByName_Ref(sysEventInfo, "domain", static_cast<ani_ref>(domain_ctor))) {
            HILOG_ERROR(LOG_CORE, "set property SysEventInfoAni.domain failed");
        }
    } else if (key == NAME_ATTR) {
        ani_object name_ctor = HiSysEventAniUtil::CreateStringValue(env, value);
        if (ANI_OK != env->Object_SetPropertyByName_Ref(sysEventInfo, "name", static_cast<ani_ref>(name_ctor))) {
            HILOG_ERROR(LOG_CORE, "set property SysEventInfoAni.name failed");
        }
    }
}

static void AppendBaseInfo(ani_env *env, ani_object& sysEventInfo, const std::string& key, Json::Value& value)
{
    if ((key == DOMAIN__KEY || key == NAME__KEY) && value.isString()) {
        HiSysEventAniUtil::AppendStringPropertyToJsObject(env, TranslateKeyToAttrName(key),
            value.asString(), sysEventInfo);
    }
    if (key == TYPE__KEY && value.isInt()) {
        HiSysEventAniUtil::AppendInt32PropertyToJsObject(env, TranslateKeyToAttrName(key),
            static_cast<int32_t>(value.asInt()), sysEventInfo);
    }
}

static bool CreateParamItemTypeValue(ani_env *env, Json::Value& jsonValue, ani_object& value)
{
    if (jsonValue.isBool()) {
        value = HiSysEventAniUtil::CreateBool(env, jsonValue.asBool());
        return true;
    }
    if (jsonValue.isInt()) {
        value = HiSysEventAniUtil::CreateDoubleInt32(env, static_cast<int32_t>(jsonValue.asInt()));
        return true;
    }
    if (jsonValue.isUInt()) {
        value = HiSysEventAniUtil::CreateDoubleUint32(env, static_cast<uint32_t>(jsonValue.asUInt()));
        return true;
    }
#ifdef JSON_HAS_INT64
    if (jsonValue.isInt64() && jsonValue.type() != Json::ValueType::uintValue) {
        value = HiSysEventAniUtil::CreateDoubleInt64(env, jsonValue.asInt64());
        return true;
    }
    if (jsonValue.isUInt64() && jsonValue.type() != Json::ValueType::intValue) {
        value = HiSysEventAniUtil::CreateDoubleUint64(env, jsonValue.asUInt64());
        return true;
    }
#endif
    if (jsonValue.isDouble()) {
        value = HiSysEventAniUtil::CreateDouble(env, jsonValue.asDouble());
        return true;
    }
    if (jsonValue.isString()) {
        value = HiSysEventAniUtil::CreateStringValue(env, jsonValue.asString());
        return true;
    }
    return false;
}

static void AppendArrayParams(ani_env *env, ani_object& sysEventInfo, std::string& key, Json::Value& value)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return;
    }

    size_t len = value.size();
    ani_array array = nullptr;
    ani_class cls;
    if (ANI_OK != env->FindClass(CLASS_NAME_SYSEVENTINFOANI, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_SYSEVENTINFOANI);
        return;
    }
    ani_ref initialArrayRef = nullptr;
    if (len > 0) {
        initialArrayRef = HiSysEventAniUtil::GetAniUndefined(env);
    }
    if (ANI_OK != env->Array_New(len, initialArrayRef, &array)) {
        HILOG_ERROR(LOG_CORE, "create %{public}s array failed", CLASS_NAME_SYSEVENTINFOANI);
        return;
    }
    for (size_t i = 0; i < len; i++) {
        ani_object item;
        if (!CreateParamItemTypeValue(env, value[static_cast<int>(i)], item)) {
            continue;
        }
        if (ANI_OK != env->Array_Set(array, i, static_cast<ani_ref>(item))) {
            HILOG_ERROR(LOG_CORE, "set %{public}zu item failed", i);
            return;
        }
    }
    if (ANI_OK != env->Object_SetPropertyByName_Ref(sysEventInfo, "params", static_cast<ani_ref>(array))) {
        HILOG_ERROR(LOG_CORE, "set property SysEventInfoAni.params failed");
    }
}

static void AppendParamsInfo(ani_env *env, ani_object& sysEventInfo, std::string& key, Json::Value& jsonValue)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return;
    }

    if (jsonValue.isArray()) {
        AppendArrayParams(env, sysEventInfo, key, jsonValue);
        return;
    }
    ani_object property = nullptr;
    if (!CreateParamItemTypeValue(env, jsonValue, property)) {
        return;
    }
    if (ANI_OK != env->Object_SetPropertyByName_Ref(sysEventInfo, "params", static_cast<ani_ref>(property))) {
        HILOG_ERROR(LOG_CORE, "set params %{public}s failed", CLASS_NAME_SYSEVENTINFOANI);
    }
}

static bool IsBaseInfoKey(std::string& propertyName)
{
    return propertyName == DOMAIN__KEY || propertyName == NAME__KEY || propertyName == TYPE__KEY;
}

void HiSysEventAniUtil::CreateJsSysEventInfoArray(ani_env *env, const std::vector<std::string>& originValues,
    ani_array& sysEventInfoJsArray)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return;
    }

    auto len = originValues.size();
    ani_class cls;
    if (ANI_OK != env->FindClass(CLASS_NAME_SYSEVENTINFOANI, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_SYSEVENTINFOANI);
        return;
    }
    ani_object sysEventInfo = nullptr;
    ani_method ctor {};
    if (ANI_OK != env->Class_FindMethod(cls, CONSTRUCTOR_NAME, nullptr, &ctor)) {
        HILOG_ERROR(LOG_CORE, "get method %{public}s <ctor> failed", CLASS_NAME_SYSEVENTINFOANI);
        return;
    }
    for (size_t i = 0; i < len; i++) {
        if (ANI_OK != env->Object_New(cls, ctor, &sysEventInfo)) {
            HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_SYSEVENTINFOANI);
            return;
        }
        CreateHiSysEventInfoJsObject(env, originValues[i], sysEventInfo);
        ani_status ret = env->Array_Set(sysEventInfoJsArray, i, static_cast<ani_ref>(sysEventInfo));
        if (ret != ANI_OK) {
            HILOG_DEBUG(LOG_CORE, "set %{public}zu sysEventInfo failed", i);
        }
    }
}

void HiSysEventAniUtil::CreateHiSysEventInfoJsObject(ani_env *env, const std::string& jsonStr, ani_object& sysEventInfo)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return;
    }

    Json::Value eventJson;
#ifdef JSONCPP_VERSION_STRING
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    std::unique_ptr<Json::CharReader> const reader(jsonRBuilder.newCharReader());
    JSONCPP_STRING errs;
    if (!reader->parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), &eventJson, &errs)) {
        HILOG_ERROR(LOG_CORE, "parse event detail info failed, please check the style of json infomation: %{public}s",
            jsonStr.c_str());
        return;
    }
#else
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(jsonStr, eventJson)) {
        HILOG_ERROR(LOG_CORE, "parse event detail info failed, please check the style of json infomation: %{public}s",
            jsonStr.c_str());
        return;
    }
#endif
    if (!eventJson.isObject()) {
        HILOG_ERROR(LOG_CORE, "event json parsed isn't a json object");
        return;
    }
    auto eventNameList = eventJson.getMemberNames();
    for (auto it = eventNameList.cbegin(); it != eventNameList.cend(); it++) {
        auto propertyName = *it;
        if (IsBaseInfoKey(propertyName)) {
            AppendBaseInfo(env, sysEventInfo, propertyName, eventJson[propertyName]);
        } else {
            AppendParamsInfo(env, sysEventInfo, propertyName, eventJson[propertyName]);
        }
    }
}

std::string HiSysEventAniUtil::AniStringToStdString(ani_env *env, ani_string aniStr)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return "";
    }

    ani_size strSize;
    env->String_GetUTF8Size(aniStr, &strSize);
    std::vector<char> buffer(strSize + 1);
    char *utf8Buffer = buffer.data();
    ani_size bytesWritten = 0;
    env->String_GetUTF8(aniStr, utf8Buffer, strSize + 1, &bytesWritten);
    utf8Buffer[bytesWritten] = '\0';
    std::string content = std::string(utf8Buffer);
    return content;
}

ani_vm* HiSysEventAniUtil::GetAniVm(ani_env *env)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return nullptr;
    }

    ani_vm *vm = nullptr;
    if (ANI_OK != env->GetVM(&vm)) {
        HILOG_ERROR(LOG_CORE, "get vm failed");
        return nullptr;
    }
    return vm;
}

ani_env* HiSysEventAniUtil::GetAniEnv(ani_vm *vm)
{
    if (vm == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid vm");
        return nullptr;
    }

    ani_env *env = nullptr;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        HILOG_ERROR(LOG_CORE, "get env failed");
        return nullptr;
    }
    return env;
}

ani_env* HiSysEventAniUtil::AttachAniEnv(ani_vm *vm)
{
    if (vm == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid vm");
        return nullptr;
    }

    ani_env *workerEnv = nullptr;
    ani_options aniArgs {0, nullptr};
    if (ani_status ret = vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &workerEnv); ret != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get attach env failed, attach ret is %{public}d", static_cast<int>(ret));
        return nullptr;
    }
    return workerEnv;
}

void HiSysEventAniUtil::DetachAniEnv(ani_vm *vm)
{
    if (vm == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid vm");
        return;
    }

    if (ANI_OK != vm->DetachCurrentThread()) {
        HILOG_ERROR(LOG_CORE, "detach env failed");
    }
}

bool HiSysEventAniUtil::GetBooleans(ani_env *env, ani_ref arrayRef, std::vector<bool>& bools)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return false;
    }

    ani_size len = 0;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array>(arrayRef), &len)) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return false;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref element = nullptr;
        if (ANI_OK != env->Array_Get(static_cast<ani_array>(arrayRef), i, &element)) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            return false;
        }
        bools.emplace_back(HiSysEventAniUtil::ParseBoolValue(env, element));
    }
    return true;
}

bool HiSysEventAniUtil::GetDoubles(ani_env *env, ani_ref arrayRef, std::vector<double>& doubles)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return false;
    }

    ani_size len = 0;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array>(arrayRef), &len)) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return false;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref element = nullptr;
        if (ANI_OK != env->Array_Get(static_cast<ani_array>(arrayRef), i, &element)) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            return false;
        }
        doubles.push_back(HiSysEventAniUtil::ParseNumberValue(env, element));
    }
    return true;
}

bool HiSysEventAniUtil::GetStrings(ani_env *env, ani_ref arrayRef, std::vector<std::string>& strs)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return false;
    }

    ani_size len = 0;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array>(arrayRef), &len)) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return false;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref element = nullptr;
        if (ANI_OK != env->Array_Get(static_cast<ani_array>(arrayRef), i, &element)) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            return false;
        }
        strs.push_back(HiSysEventAniUtil::ParseStringValue(env, element));
    }
    return true;
}

bool HiSysEventAniUtil::GetIntsToDoubles(ani_env *env, ani_ref arrayRef, std::vector<double>& doubles)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return false;
    }

    ani_size len = 0;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array>(arrayRef), &len)) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return false;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref element = nullptr;
        if (ANI_OK != env->Array_Get(static_cast<ani_array>(arrayRef), i, &element)) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            return false;
        }
        doubles.push_back(static_cast<double>(HiSysEventAniUtil::ParseIntValue(env, element)));
    }
    return true;
}

bool HiSysEventAniUtil::GetBigints(ani_env *env, ani_ref arrayRef, std::vector<int64_t>& bigints)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return false;
    }

    ani_size len = 0;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array>(arrayRef), &len)) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return false;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref element = nullptr;
        if (ANI_OK != env->Array_Get(static_cast<ani_array>(arrayRef), i, &element)) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            return false;
        }
        bigints.push_back(HiSysEventAniUtil::ParseBigintValue(env, element));
    }
    return true;
}

ani_object HiSysEventAniUtil::CreateAniInt(ani_env *env, int intValue)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return nullptr;
    }
    ani_object personInfoObj = nullptr;
    ani_class personCls;
    if (ANI_OK != env->FindClass(CLASS_NAME_INT, &personCls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_INT);
        return personInfoObj;
    }
    ani_method personInfoCtor;
    if (ANI_OK != env->Class_FindMethod(personCls, CONSTRUCTOR_NAME, "i:", &personInfoCtor)) {
        HILOG_ERROR(LOG_CORE, "find method <ctor> failed");
        return personInfoObj;
    }
    if (ANI_OK != env->Object_New(personCls, personInfoCtor, &personInfoObj, static_cast<ani_int>(intValue))) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_DOUBLE);
        return personInfoObj;
    }
    return personInfoObj;
}

ani_ref HiSysEventAniUtil::GetAniUndefined(ani_env *env)
{
    if (env == nullptr) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return nullptr;
    }
    ani_ref undefinedRef {};
    if (ANI_OK != env->GetUndefined(&undefinedRef)) {
        HILOG_ERROR(LOG_CORE, "invalid env");
        return undefinedRef;
    }
    return undefinedRef;
}
