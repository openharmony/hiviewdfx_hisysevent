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

#ifndef NAPI_HISYSEVENT_LISTENER_H
#define NAPI_HISYSEVENT_LISTENER_H

#include <string>
#include <sys/syscall.h>
#include <unistd.h>

#include "hisysevent_subscribe_callback.h"
#include "js_callback_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_callback_context.h"

namespace OHOS {
namespace HiviewDFX {
class NapiHiSysEventListener : public OHOS::HiviewDFX::HiSysEventSubscribeCallBack {
public:
    NapiHiSysEventListener(CallbackContext* context)
    {
        callbackContext = context;
        jsCallbackManager = std::make_shared<JsCallbackManager>();
    }
    virtual ~NapiHiSysEventListener()
    {
        if (jsCallbackManager != nullptr) {
            jsCallbackManager->Release();
        }
        if (callbackContext->threadId == syscall(SYS_gettid)) {
            napi_delete_reference(callbackContext->env, callbackContext->ref);
        }
        delete callbackContext;
    }

public:
    void OnHandle(const std::string& domain, const std::string& eventName, const int eventType,
        const std::string& eventDetail);
    void OnServiceDied();

private:
    CallbackContext* callbackContext;
    std::shared_ptr<JsCallbackManager> jsCallbackManager;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // NAPI_HISYSEVENT_LISTENER_H