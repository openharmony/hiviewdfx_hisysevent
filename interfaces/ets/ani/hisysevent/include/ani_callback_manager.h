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

#ifndef ANI_CALLBACK_MANAGER_H
#define ANI_CALLBACK_MANAGER_H

#include <ani.h>
#include <atomic>
#include <mutex>
#include <queue>
#include <tuple>

#include "ani_callback_context.h"

namespace OHOS {
namespace HiviewDFX {
using TaskQueue = std::queue<std::tuple<CallbackContextAni*, CALLBACK_FUNC, RELEASE_FUNC>>;
class AniCallbackManager final {
public:
    explicit AniCallbackManager() {}
public:
    void Add(CallbackContextAni *context, CALLBACK_FUNC callback, RELEASE_FUNC release = nullptr);
    void Release();

private:
    void ImmediateRun(ani_vm *vm, bool needPop = false);
    void Clear(TaskQueue& tasks);

private:
    std::atomic<bool> inCalling = false;
    std::atomic<bool> IsReleased = false;
    std::mutex managerMutex;
    TaskQueue AniCallbacks;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // ANI_CALLBACK_MANAGER_H
