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

#include "napi_hisysevent_querier.h"

#include "hilog/log.h"
#include "napi_hisysevent_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "NAPI_HISYSEVENT_QUERIER" };
constexpr char ON_QUERY_ATTR[] = "onQuery";
constexpr char ON_COMPLETE_ATTR[] = "onComplete";
constexpr size_t ON_QUERY_COMPLTE_PARAM_COUNT = 2;
}

void NapiHiSysEventQuerier::OnQuery(const std::vector<std::string>& sysEvent,
    const std::vector<int64_t>& seq)
{
    NapiHiSysEventUtil::CallJSCallback(callbackContext, [this, sysEvent, seq](CallbackContext* context) {
        napi_value sysEventJsParam = nullptr;
        napi_create_array_with_length(context->env, sysEvent.size(), &sysEventJsParam);
        NapiHiSysEventUtil::CreateJsSysEventInfoArray(context->env, sysEvent, sysEventJsParam);
        napi_value seqJsParam = nullptr;
        napi_create_array_with_length(context->env, seq.size(), &seqJsParam);
        NapiHiSysEventUtil::CreateJsInt64Array(context->env, seq, sysEventJsParam);
        napi_value argv[ON_QUERY_COMPLTE_PARAM_COUNT] = {sysEventJsParam, seqJsParam};
        napi_value querier = nullptr;
        napi_get_reference_value(context->env, context->ref, &querier);
        napi_value onQuery = NapiHiSysEventUtil::GetPropertyByName(context->env, querier, ON_QUERY_ATTR);
        napi_value ret = nullptr;
        napi_status status = napi_call_function(context->env, querier, onQuery, ON_QUERY_COMPLTE_PARAM_COUNT,
            argv, &ret);
        if (status != napi_ok) {
            HiLog::Debug(LABEL, "Failed to call OnQuery JS function.");
        }
    });
}

void NapiHiSysEventQuerier::OnComplete(int32_t reason, int32_t total)
{
    NapiHiSysEventUtil::CallJSCallback(callbackContext, [this, reason, total](CallbackContext* context) {
        napi_value reasonJsParam = nullptr;
        NapiHiSysEventUtil::CreateInt32Value(context->env, reason, reasonJsParam);
        napi_value totalJsParam = nullptr;
        NapiHiSysEventUtil::CreateInt32Value(context->env, total, totalJsParam);
        napi_value argv[ON_QUERY_COMPLTE_PARAM_COUNT] = {reasonJsParam, totalJsParam};
        napi_value querier = nullptr;
        napi_get_reference_value(context->env, context->ref, &querier);
        napi_value OnComplete = NapiHiSysEventUtil::GetPropertyByName(context->env, querier, ON_COMPLETE_ATTR);
        napi_value ret = nullptr;
        napi_status status = napi_call_function(context->env, querier, OnComplete, ON_QUERY_COMPLTE_PARAM_COUNT,
            argv, &ret);
        if (status != napi_ok) {
            HiLog::Debug(LABEL, "Failed to call OnComplete JS function.");
        }
        if (this->onCompleteHandler != nullptr) {
            this->onCompleteHandler();
        }
    });
}
} // HiviewDFX
} // OHOS