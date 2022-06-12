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

#include "js_callback_manager.h"

#include "hilog/log.h"
#include "uv.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "JS_CALLBACK_MANAGER" };
constexpr int CONTEXT_INDEX = 0;
constexpr int CALLBACK_FUNC_INDEX = 1;
constexpr int CALLBACK_END_FUNC_INDEX = 2;
}

void JsCallbackManager::Add(CallbackContext* context, CALLBACK_FUNC callback,
    CALLBACK_END_FUNC callbackEnd)
{
    jsCallbacks.emplace(std::make_tuple(context, callback, [this, callbackEnd] () {
        this->inCalling.exchange(true, std::memory_order_acq_rel);
        this->ImmediateRun(true);
        if (callbackEnd != nullptr) {
            callbackEnd();
        }
    }));
    if (inCalling.load(std::memory_order_acquire)) {
        return;
    }
    ImmediateRun();
}

void JsCallbackManager::ImmediateRun(bool needPop)
{
    inCalling.exchange(true, std::memory_order_acq_rel);
    if (needPop) {
        jsCallbacks.pop();
    }
    if (jsCallbacks.empty()) {
        inCalling.exchange(false, std::memory_order_acq_rel);
        return;
    }
    std::tuple<CallbackContext*, CALLBACK_FUNC, CALLBACK_END_FUNC> current = jsCallbacks.front();
    CallbackContext* context = std::get<CONTEXT_INDEX>(current);
    uv_loop_t* loop = nullptr;
    napi_get_uv_event_loop(context->env, &loop);
    context->callback = std::get<CALLBACK_FUNC_INDEX>(current);
    context->endCallback = std::get<CALLBACK_END_FUNC_INDEX>(current);
    uv_work_t* work = new uv_work_t();
    work->data = (void*)context;
    uv_queue_work(
        loop,
        work,
        [] (uv_work_t* work) {},
        [] (uv_work_t* work, int status) {
            CallbackContext* context = (CallbackContext*)work->data;
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(context->env, &scope);
            if (scope == nullptr) {
                HiLog::Debug(LABEL, "napi scope is null.");
                return;
            }
            if (context->callback != nullptr) {
                context->callback(context);
            }
            napi_close_handle_scope(context->env, scope);
            delete work;
            work = nullptr;
            if (context->endCallback != nullptr) {
                context->endCallback();
            }
        });
}
} // HiviewDFX
} // OHOS