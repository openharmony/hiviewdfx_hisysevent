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

namespace OHOS {
namespace HiviewDFX {
constexpr char CLASS_NAME_RESULTINNER[] = "L@ohos/hiSysEvent/ResultInner;";
constexpr char CLASS_NAME_INT[] = "Lstd/core/Int;";
constexpr char CLASS_NAME_BOOLEAN[] = "Lstd/core/Boolean;";
constexpr char CLASS_NAME_DOUBLE[] = "Lstd/core/Double;";
constexpr char CLASS_NAME_STRING[] = "Lstd/core/String;";
constexpr char CLASS_NAME_BIGINT[] = "Lescompat/BigInt;";
constexpr char CLASS_NAME_ARRAY[] = "Lescompat/Array;";
constexpr char CLASS_NAME_ITERATOR[] = "Lescompat/Iterator;";
constexpr char CLASS_NAME_RECORD[] = "Lescompat/Record;";
constexpr char FUNC_NAME_GETLONG[] = "getLong";
constexpr char FUNC_NAME_UNBOXED[] = "unboxed";
constexpr char FUNC_NAME_NEXT[] = "next";

class HiSysEventAniUtil {
public:
    static bool CheckKeyTypeString(const std::string &str);
    static bool IsArray(ani_env *env, ani_object object);
    static bool IsRefUndefined(ani_env *env, ani_ref ref);
    static bool IsSystemAppCall();
    static int64_t ParseBigintValue(ani_env *env, ani_ref elementRef);
    static bool ParseBoolValue(ani_env *env, ani_ref elementRef);
    static double ParseNumberValue(ani_env *env, ani_ref elementRef);
    static std::map<std::string, ani_ref> ParseRecord(ani_env *env, ani_ref recordRef);
    static std::string ParseStringValue(ani_env *env, ani_ref aniStrRef);
    static std::pair<int32_t, std::string> GetErrorDetailByRet(const int32_t retCode);
    static ani_object WriteResult(ani_env *env, std::pair<int32_t, std::string> result);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_ANI_UTILS_H
