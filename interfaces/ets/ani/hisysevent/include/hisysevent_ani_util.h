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

#ifndef HISYSEVENT_ANI_UTILS_H
#define HISYSEVENT_ANI_UTILS_H

#include <ani.h>
#include <map>
#include <string>
#include <memory>
#include <sys/syscall.h>
#include <vector>
#include <unistd.h>
#include <unordered_map>
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "ANI_HISYSEVENT_UTIL"

enum EventTypeAni : int32_t {
    FAULT = 1,
    STATISTIC = 2,
    SECURITY = 3,
    BEHAVIOR = 4
};

constexpr char CLASS_NAME_INT[] = "std.core.Int";
constexpr char CLASS_NAME_BOOLEAN[] = "std.core.Boolean";
constexpr char CLASS_NAME_DOUBLE[] = "std.core.Double";
constexpr char CLASS_NAME_STACKTRACE[] = "std.core.StackTrace";
constexpr char CLASS_NAME_STRING[] = "std.core.String";
constexpr char CLASS_NAME_BIGINT[] = "escompat.BigInt";
constexpr char CLASS_NAME_ARRAY[] = "escompat.Array";
constexpr char CLASS_NAME_SYSEVENTINFOANI[] = "@ohos.hiSysEvent.SysEventInfoAni";
constexpr char ENUM_NAME_EVENT_TYPE[] = "@ohos.hiSysEvent.hiSysEvent.EventType";
constexpr char CLASS_NAME_HISYSEVENTANI[] = "@ohos.hiSysEvent.hiSysEvent";
constexpr char CLASS_NAME_RESULTINNER[] = "@ohos.hiSysEvent.ResultInner";
constexpr char CLASS_NAME_WATCHERANI[] = "@ohos.hiSysEvent.WatcherAni";
constexpr char CLASS_NAME_QUERIERANI[] = "@ohos.hiSysEvent.QuerierAni";
constexpr char CLASS_NAME_BUSINESSERROR[] = "@ohos.base.BusinessError";
constexpr char CLASS_NAME_ITERATOR[] = "std.core.Iterator";
constexpr char CLASS_NAME_RECORD[] = "escompat.Record";
constexpr char EVENT_TYPE_ATTR[] = "eventType";
constexpr char FUNC_NAME_GETLONG[] = "getLong";
constexpr char FUNC_NAME_TOBOOLEAN[] = "toBoolean";
constexpr char FUNC_NAME_TOINT[] = "toInt";
constexpr char FUNC_NAME_TODOUBLE[] = "toDouble";
constexpr char FUNC_NAME_NEXT[] = "next";
constexpr char DOMAIN_ATTR[] = "domain";
constexpr char NAME_ATTR[] = "name";

class HiSysEventAniUtil {
public:
    static std::string AniStringToStdString(ani_env *env, ani_string aniStr);
    static bool CheckKeyTypeString(const std::string &str);
    static bool IsArray(ani_env *env, ani_object object);
    static bool IsRefUndefined(ani_env *env, ani_ref ref);
    static bool IsSystemAppCall();
    static int64_t ParseBigintValue(ani_env *env, ani_ref elementRef);
    static bool ParseBoolValue(ani_env *env, ani_ref elementRef);
    static double ParseNumberValue(ani_env *env, ani_ref elementRef);
    static std::map<std::string, ani_ref> ParseRecord(ani_env *env, ani_ref recordRef);
    static std::string ParseStringValue(ani_env *env, ani_ref aniStrRef);
    static int ParseIntValue(ani_env *env, ani_ref elementRef);
    static bool GetBooleans(ani_env *env, ani_ref arrayRef, std::vector<bool>& bools);
    static bool GetDoubles(ani_env *env, ani_ref arrayRef, std::vector<double>& doubles);
    static bool GetIntsToDoubles(ani_env *env, ani_ref arrayRef, std::vector<double>& doubles);
    static bool GetStrings(ani_env *env, ani_ref arrayRef, std::vector<std::string>& strs);
    static bool GetBigints(ani_env *env, ani_ref arrayRef, std::vector<int64_t>& bigints);
    static ani_object WriteResult(ani_env *env, const std::pair<int32_t, std::string>& result);
    static ani_object CreateDoubleUint64(ani_env *env, uint64_t number);
    static ani_object CreateDoubleInt64(ani_env *env, int64_t number);
    static ani_object CreateDoubleUint32(ani_env *env, uint32_t number);
    static ani_object CreateDoubleInt32(ani_env *env, int number);
    static ani_enum_item ToAniEnum(ani_env *env, EventTypeAni value);
    static ani_object CreateStringValue(ani_env *env, const std::string& value);
    static ani_object CreateDouble(ani_env *env, double number);
    static ani_object CreateBool(ani_env *env, bool boolValue);
    static void CreateHiSysEventInfoJsObject(ani_env *env, const std::string& jsonStr, ani_object& sysEventInfo);
    static void CreateJsSysEventInfoArray(ani_env *env, const std::vector<std::string>& originValues,
        ani_array& sysEventInfoJsArray);
    static void AppendInt32PropertyToJsObject(ani_env *env, const std::string& key, const int32_t& value,
        ani_object& jsObj);
    static void AppendStringPropertyToJsObject(ani_env *env, const std::string& key, const std::string& value,
        ani_object& jsObj);

public:
    static std::pair<int32_t, std::string> GetErrorDetailByRet(const int32_t retCode);
    static void ThrowAniError(ani_env *env, int32_t code, const std::string &message);
    static void ThrowErrorByRet(ani_env *env, const int32_t retCode);

public:
    static ani_vm* GetAniVm(ani_env *env);
    static ani_env* GetAniEnv(ani_vm *vm);
    static ani_env* AttachAniEnv(ani_vm *vm);
    static void DetachAniEnv(ani_vm *vm);

public:
    template<typename T>
    static typename std::unordered_map<ani_ref, std::pair<pid_t, std::shared_ptr<T>>>::iterator
    CompareAndReturnCacheItem(ani_env *env, ani_object& standard,
        std::unordered_map<ani_ref, std::pair<pid_t, std::shared_ptr<T>>>& resources)
    {
        auto iter = resources.begin();
        for (; iter != resources.end(); iter++) {
            if (iter->second.first != getproctid()) {
                continue;
            }
            ani_ref val = iter->first;
            ani_boolean isEquals;
            ani_status ret = env->Reference_StrictEquals(standard, static_cast<ani_object>(val), &isEquals);
            if (ret != ANI_OK) {
                continue;
            }
            if (isEquals) {
                break;
            }
        }
        return iter;
    }
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HISYSEVENT_ANI_UTILS_H
