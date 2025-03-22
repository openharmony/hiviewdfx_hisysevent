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
#include "hilog/log.h"
#include "hilog/log_cpp.h"
#include "ipc_skeleton.h"
#include "ret_code.h"
#include "ret_def.h"
#include "tokenid_kit.h"

using namespace OHOS::HiviewDFX;
const std::string INVALID_KEY_TYPE_ARR[] = {
    "[object Object]",
    "null",
    "()",
    ","
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
    ani_long longnum = static_cast<ani_long>(0);
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_BIGINT, &cls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_BIGINT);
        return static_cast<int64_t>(longnum);
    }
    ani_method getLongMethod {};
    if (ANI_OK != env->Class_FindMethod(cls, FUNC_NAME_GETLONG, ":J", &getLongMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_GETLONG);
        return static_cast<int64_t>(longnum);
    }
    if (ANI_OK != env->Object_CallMethod_Long(static_cast<ani_object>(elementRef), getLongMethod, &longnum)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Long %{public}s Failed", FUNC_NAME_GETLONG);
        return static_cast<int64_t>(longnum);
    }
    return static_cast<int64_t>(longnum);
}

bool HiSysEventAniUtil::ParseBoolValue(ani_env *env, ani_ref elementRef)
{
    ani_boolean booleanVal = ANI_FALSE;
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_BOOLEAN, &cls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_BOOLEAN);
        return false;
    }
    ani_method unboxedMethod {};
    if (ANI_OK != env->Class_FindMethod(cls, FUNC_NAME_UNBOXED, ":Z", &unboxedMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_UNBOXED);
        return false;
    }
    if (ANI_OK != env->Object_CallMethod_Boolean(static_cast<ani_object>(elementRef), unboxedMethod, &booleanVal)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Boolean %{public}s Failed", FUNC_NAME_UNBOXED);
        return false;
    }
    return static_cast<bool>(booleanVal);
}

double HiSysEventAniUtil::ParseNumberValue(ani_env *env, ani_ref elementRef)
{
    ani_double doubleVal = 0;
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_DOUBLE, &cls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_DOUBLE);
        return static_cast<double>(doubleVal);
    }
    ani_method unboxedMethod {};
    if (ANI_OK != env->Class_FindMethod(cls, FUNC_NAME_UNBOXED, ":D", &unboxedMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_UNBOXED);
        return static_cast<double>(doubleVal);
    }
    if (ANI_OK != env->Object_CallMethod_Double(static_cast<ani_object>(elementRef), unboxedMethod, &doubleVal)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Double %{public}s Failed", FUNC_NAME_UNBOXED);
        return static_cast<double>(doubleVal);
    }
    return static_cast<double>(doubleVal);
}

static void GetKeyValueFormIterArray(ani_env *env, ani_ref param, std::string &key, ani_ref &value)
{
    ani_size keyIndex = 0;
    ani_size valueIndex = keyIndex + 1;
    ani_ref keyRef {};
    if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(param), keyIndex, &keyRef)) {
        HILOG_ERROR(LOG_CORE, "Array_Get_Ref %{public}zu Failed", keyIndex);
        return;
    }
    key = HiSysEventAniUtil::ParseStringValue(env, keyRef);
    if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(param), valueIndex, &value)) {
        HILOG_ERROR(LOG_CORE, "Array_Get_Ref %{public}zu Failed", valueIndex);
        return;
    }
}

static void ParseIterValue(ani_env *env, ani_ref iterValue, std::map<std::string, ani_ref>& recordResult)
{
    if (HiSysEventAniUtil::IsArray(env, static_cast<ani_object>(iterValue))) {
        std::string key = "";
        ani_ref value {};
        GetKeyValueFormIterArray(env, iterValue, key, value);
        recordResult[key] = value;
    }
}

static void ParseIterator(ani_env *env, ani_ref iteratorRef, std::map<std::string, ani_ref>& recordResult)
{
    ani_class IteratorCls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_ITERATOR, &IteratorCls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_ITERATOR);
        return;
    }
    ani_method nextMethod {};
    if (ANI_OK != env->Class_FindMethod(IteratorCls, FUNC_NAME_NEXT, nullptr, &nextMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_NEXT);
        return;
    }
    ani_object IteratorObj = static_cast<ani_object>(iteratorRef);
    ani_ref iteratorResult {};
    ani_boolean done = ANI_FALSE;
    while (!done) {
        if (ANI_OK != env->Object_CallMethod_Ref(IteratorObj, nextMethod, &iteratorResult)) {
            HILOG_ERROR(LOG_CORE, "Object_CallMethod_Ref next Failed");
            return;
        }
        ani_object iterResultObj = static_cast<ani_object>(iteratorResult);
        if (ANI_OK != env->Object_GetPropertyByName_Boolean(iterResultObj, "done", &done)) {
            HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Boolean done Failed");
            return;
        }
        if (!done) {
            ani_ref value {};
            if (ANI_OK != env->Object_GetPropertyByName_Ref(iterResultObj, "value", &value)) {
                HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref value Failed");
                return;
            }
            ParseIterValue(env, value, recordResult);
        }
    }
}

std::map<std::string, ani_ref> HiSysEventAniUtil::ParseRecord(ani_env *env, ani_ref recordRef)
{
    std::map<std::string, ani_ref> recordResult = {};
    ani_class recordCls{};
    if (ANI_OK != env->FindClass(CLASS_NAME_RECORD, &recordCls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_RECORD);
        return recordResult;
    }

    ani_method entriesMethod {};
    if (ANI_OK != env->Class_FindMethod(recordCls, "entries", nullptr, &entriesMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod entries Failed");
        return recordResult;
    }
    ani_ref iteratorRef {};
    if (ANI_OK != env->Object_CallMethod_Ref(static_cast<ani_object>(recordRef), entriesMethod, &iteratorRef)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Ref entries Failed");
        return recordResult;
    }
    ParseIterator(env, iteratorRef, recordResult);
    return recordResult;
}

std::string HiSysEventAniUtil::ParseStringValue(ani_env *env, ani_ref aniStrRef)
{
    ani_size strSize = 0;
    if (ANI_OK != env->String_GetUTF8Size(static_cast<ani_string>(aniStrRef), &strSize)) {
        HILOG_ERROR(LOG_CORE, "String_GetUTF8Size Failed");
        return "";
    }
    std::vector<char> buffer(strSize + 1);
    char* utf8Buffer = buffer.data();
    ani_size bytesWritten = 0;
    if (ANI_OK != env->String_GetUTF8(static_cast<ani_string>(aniStrRef), utf8Buffer, strSize + 1, &bytesWritten)) {
        HILOG_ERROR(LOG_CORE, "String_GetUTF8 Failed");
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

ani_object HiSysEventAniUtil::WriteResult(ani_env *env, std::pair<int32_t, std::string> result)
{
    ani_object results_obj {};
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_RESULTINNER, &cls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_RESULTINNER);
        return results_obj;
    }
    ani_method ctor {};
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        HILOG_ERROR(LOG_CORE, "get %{public}s <ctor> Failed", CLASS_NAME_RESULTINNER);
        return results_obj;
    }
    if (ANI_OK != env->Object_New(cls, ctor, &results_obj)) {
        HILOG_ERROR(LOG_CORE, "Create Object %{public}s Failed", CLASS_NAME_RESULTINNER);
        return results_obj;
    }

    ani_method codeSetter {};
    if (ANI_OK != env->Class_FindMethod(cls, "<set>code", nullptr, &codeSetter)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod <set>code Failed");
    }
    if (ANI_OK != env->Object_CallMethod_Void(results_obj, codeSetter, static_cast<ani_double>(result.first))) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Void <set>code Failed");
        return results_obj;
    }

    ani_method messageSetter {};
    if (ANI_OK != env->Class_FindMethod(cls, "<set>message", nullptr, &messageSetter)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod <set>message Failed");
    }
    std::string message = result.second;
    ani_string message_string{};
    env->String_NewUTF8(message.c_str(), message.size(), &message_string);
    if (ANI_OK != env->Object_CallMethod_Void(results_obj, messageSetter, message_string)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Void <set>message Failed");
        return results_obj;
    }

    return results_obj;
}
