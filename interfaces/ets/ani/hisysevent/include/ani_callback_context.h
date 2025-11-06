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
 
#ifndef ANI_CALLBACK_CONTEXT_H
#define ANI_CALLBACK_CONTEXT_H

#include <ani.h>
#include <functional>
#include <sys/syscall.h>

namespace OHOS {
namespace HiviewDFX {
using CALLBACK_FUNC = std::function<void(ani_vm*, ani_ref, pid_t threadId)>;
using RELEASE_FUNC = std::function<void(ani_vm*, pid_t threadId)>;
struct CallbackContextAni {
    ani_vm *vm = nullptr;
    ani_ref ref = nullptr;
    CALLBACK_FUNC callback;
    RELEASE_FUNC release;
    pid_t threadId;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // ANI_CALLBACK_CONTEXT_H
