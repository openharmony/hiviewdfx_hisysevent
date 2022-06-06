/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef NAPI_HISYSEVENT_RET_DEF_H
#define NAPI_HISYSEVENT_RET_DEF_H

namespace OHOS {
namespace HiviewDFX {
static constexpr int32_t NAPI_SUCCESS = 0;

static constexpr int32_t ERROR_INVALID_PARAM_COUNT = -100;
static constexpr int32_t ERROR_NAPI_PARSED_FAILED = -101;
static constexpr int32_t ERROR_LISTENER_RULES_TYPE_NOT_ARRAY = -102;
static constexpr int32_t ERROR_LISTENER_RULE_TYPE_INVALID = -103;
static constexpr int32_t ERROR_QUERY_ARG_TYPE_INVALID = -104;
static constexpr int32_t ERROR_QUERY_RULES_TYPE_NOT_ARRAY = -105;
static constexpr int32_t ERROR_QUERY_RULE_TYPE_INVALID = -106;
static constexpr int32_t ERROR_NAPI_LISTENER_NOT_FOUND = -107;
} // namespace HiviewDFX
} // namespace OHOS

#endif // NAPI_HISYSEVENT_RET_DEF_H

