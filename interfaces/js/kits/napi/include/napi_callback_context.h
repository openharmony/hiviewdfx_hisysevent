/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
 
#ifndef NAPI_CALLBACK_CONTEXT_H
#define NAPI_CALLBACK_CONTEXT_H

#include <functional>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
using CALLBACK_FUNC = std::function<void(const napi_env, const napi_ref)>;
using RELEASE_FUNC = std::function<void()>;
using CallbackContext = struct CallbackContext {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    CALLBACK_FUNC callback;
    RELEASE_FUNC release;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // NAPI_CALLBACK_CONTEXT_H