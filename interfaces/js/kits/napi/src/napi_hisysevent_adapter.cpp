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

#include "napi_hisysevent_adapter.h"

#include "def.h"
#include "napi_hisysevent_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int CALLBACK_FUNC_PARAM_NUM = 2;
constexpr char FUNC_SOURCE_NAME[] = "JSHiSysEventWrite";
constexpr char ERROR_CODE_KEY[] = "code";
}

void NapiHiSysEventAdapter::Write(const napi_env env, HiSysEventAsyncContext* eventAsyncContext)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, FUNC_SOURCE_NAME, NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [] (napi_env env, void* data) {
            HiSysEventAsyncContext* eventAsyncContext = (HiSysEventAsyncContext*)data;
            if (eventAsyncContext->eventWroteResult == SUCCESS) {
                eventAsyncContext->eventWroteResult = Write(eventAsyncContext->eventInfo);
            }
        },
        [] (napi_env env, napi_status status, void* data) {
            HiSysEventAsyncContext* eventAsyncContext = (HiSysEventAsyncContext*)data;
            napi_value result[CALLBACK_FUNC_PARAM_NUM] = {0};
            if (eventAsyncContext->eventWroteResult == SUCCESS) {
                napi_get_undefined(env, &result[0]);
                napi_create_int32(env, eventAsyncContext->eventWroteResult, &result[1]);
            } else {
                napi_create_object(env, &result[0]);
                napi_value errCode = nullptr;
                napi_create_int32(env, eventAsyncContext->eventWroteResult, &errCode);
                napi_set_named_property(env, result[0], ERROR_CODE_KEY, errCode);
                napi_get_undefined(env, &result[1]);
            }
            if (eventAsyncContext->deferred) {
                if (eventAsyncContext->eventWroteResult == SUCCESS) {
                    napi_resolve_deferred(env, eventAsyncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, eventAsyncContext->deferred, result[0]);
                }
            } else {
                napi_value callback = nullptr;
                napi_get_reference_value(env, eventAsyncContext->callback, &callback);
                napi_value retValue = nullptr;
                napi_call_function(env, nullptr, callback, CALLBACK_FUNC_PARAM_NUM, result, &retValue);
                napi_delete_reference(env, eventAsyncContext->callback);
            }

            napi_delete_async_work(env, eventAsyncContext->asyncWork);
            delete eventAsyncContext;
        }, (void*)eventAsyncContext, &eventAsyncContext->asyncWork);
    napi_queue_async_work(env, eventAsyncContext->asyncWork);
}

void NapiHiSysEventAdapter::InnerWrite(HiSysEvent::EventBase& eventBase,
    const HiSysEventInfo& eventInfo)
{
    AppendData<bool>(eventBase, eventInfo.boolParams);
    AppendArrayData<bool>(eventBase, eventInfo.boolArrayParams);
    AppendData<double>(eventBase, eventInfo.doubleParams);
    AppendArrayData<double>(eventBase, eventInfo.doubleArrayParams);
    AppendData<std::string>(eventBase, eventInfo.stringParams);
    AppendArrayData<std::string>(eventBase, eventInfo.stringArrayParams);
}

int NapiHiSysEventAdapter::Write(const HiSysEventInfo& eventInfo)
{
    HiSysEvent::EventBase eventBase(eventInfo.domain, eventInfo.name, eventInfo.eventType);
    eventBase.jsonStr_ << "{";
    HiSysEvent::WritebaseInfo(eventBase);
    if (HiSysEvent::IsError(eventBase)) {
        HiSysEvent::ExplainRetCode(eventBase);
        return eventBase.retCode_;
    }

    InnerWrite(eventBase, eventInfo);
    HiSysEvent::InnerWrite(eventBase);
    if (HiSysEvent::IsError(eventBase)) {
        HiSysEvent::ExplainRetCode(eventBase);
        return eventBase.retCode_;
    }
    eventBase.jsonStr_ << "}";

    HiSysEvent::SendSysEvent(eventBase);
    return eventBase.retCode_;
}
} // namespace HiviewDFX
} // namespace OHOS