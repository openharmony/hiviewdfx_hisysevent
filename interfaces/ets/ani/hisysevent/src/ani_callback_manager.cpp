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

#include "ani_callback_manager.h"
#include "hisysevent_ani_util.h"
#include "hilog/log.h"
namespace OHOS {
namespace HiviewDFX {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "ANI_CALLBACK_MANAGER"
namespace {
constexpr int CONTEXT_INDEX = 0;
constexpr int CALLBACK_FUNC_INDEX = 1;
constexpr int RELEASE_FUNC_INDEX = 2;
}

void RunCallback(ani_vm *vm, CallbackContextAni *context,
    std::tuple<CallbackContextAni*, CALLBACK_FUNC, RELEASE_FUNC>& current)
{
    context->callback = std::get<CALLBACK_FUNC_INDEX>(current);
    context->release = std::get<RELEASE_FUNC_INDEX>(current);
    if (context->callback != nullptr) {
        context->callback(vm, context->ref, context->threadId);
    }
    if (context->release != nullptr) {
        context->release(vm, context->threadId);
    }
}

void AniCallbackManager::Add(CallbackContextAni *context, CALLBACK_FUNC callback,
    RELEASE_FUNC release)
{
    {
        if (IsReleased.load(std::memory_order_acquire)) {
            return;
        }
        std::lock_guard<std::mutex> lock(managerMutex);
        AniCallbacks.emplace(std::make_tuple(context, callback, [this, release, context] (ani_vm*, pid_t threadId) {
            if (release == nullptr) {
                this->ImmediateRun(context->vm, true);
            } else {
                // Destructor of JsCallbackManager will be called in release callback,
                // so no need to call next callback in queue.
                release(context->vm, threadId);
            }
        }));
        if (inCalling.load(std::memory_order_acquire)) {
            return;
        }
    }
    ImmediateRun(context->vm);
}

void AniCallbackManager::Release()
{
    IsReleased = true;
    Clear(AniCallbacks);
}

void AniCallbackManager::ImmediateRun(ani_vm *vm, bool needPop)
{
    inCalling = true;
    std::tuple<CallbackContextAni*, CALLBACK_FUNC, RELEASE_FUNC> current;
    CallbackContextAni *context;
    {
        if (IsReleased.load(std::memory_order_acquire)) {
            return;
        }
        std::lock_guard<std::mutex> lock(managerMutex);
        if (needPop && !AniCallbacks.empty()) {
            AniCallbacks.pop();
        }

        if (AniCallbacks.empty() && !IsReleased.load(std::memory_order_acquire)) {
            inCalling = false;
            return;
        }
        current = AniCallbacks.front();
        context = std::get<CONTEXT_INDEX>(current);
        if (context == nullptr || IsReleased.load(std::memory_order_acquire)) {
            inCalling = false;
            return;
        }
    }
    if (IsReleased.load(std::memory_order_acquire)) {
        return;
    }
    RunCallback(vm, context, current);
}

void AniCallbackManager::Clear(TaskQueue& tasks)
{
    TaskQueue empty;
    swap(empty, tasks);
}
} // HiviewDFX
} // OHOS
