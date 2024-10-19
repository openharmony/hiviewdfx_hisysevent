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

#include "napi_hisysevent_init.h"

#include <memory>
#include <mutex>
#include <unordered_map>

#include "def.h"
#include "hilog/log.h"
#include "hisysevent_base_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_callback_context.h"
#include "napi_hisysevent_listener.h"
#include "napi_hisysevent_querier.h"
#include "napi_hisysevent_util.h"
#include "ret_def.h"

using namespace OHOS::HiviewDFX;

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "NAPI_HISYSEVENT_JS"

namespace {
constexpr char RULES_ATTR[] = "rules";
constexpr size_t WRITE_FUNC_MAX_PARAM_NUM = 2;
constexpr size_t ADD_LISTENER_FUNC_MAX_PARAM_NUM = 1;
constexpr size_t REMOVE_LISTENER_FUNC_MAX_PARAM_NUM = 1;
constexpr size_t QUERY_FUNC_MAX_PARAM_NUM = 3;
constexpr size_t ADD_LISTENER_LISTENER_PARAM_INDEX = 0;
constexpr size_t REMOVE_LISTENER_LISTENER_PARAM_INDEX = 0;
constexpr size_t QUERY_QUERY_ARG_PARAM_INDEX = 0;
constexpr size_t QUERY_RULE_ARRAY_PARAM_INDEX = 1;
constexpr size_t QUERY_QUERIER_PARAM_INDEX = 2;
constexpr size_t EXPORT_FUNC_MAX_PARAM_NUM = 2;
constexpr size_t EXPORT_QUERY_ARG_PARAM_INDEX = 0;
constexpr size_t EXPORT_RULE_ARRAY_PARAM_INDEX = 1;
constexpr size_t SUBSCRIBE_FUNC_MAX_PARAM_NUM = 1;
constexpr size_t SUBSCRIBE_RULE_ARRAY_PARAM_INDEX = 0;
constexpr long long DEFAULT_TIME_STAMP = -1;
constexpr int DEFAULT_EVENT_COUNT = 1000;
constexpr int TIME_STAMP_LENGTH = 13;
using NAPI_LISTENER_PAIR = std::pair<pid_t, std::shared_ptr<NapiHiSysEventListener>>;
using NAPI_QUERIER_PAIR = std::pair<pid_t, std::shared_ptr<NapiHiSysEventQuerier>>;
std::mutex g_listenerMapMutex;
std::unordered_map<napi_ref, NAPI_LISTENER_PAIR> listeners;
std::mutex g_querierMapMutex;
std::unordered_map<napi_ref, NAPI_QUERIER_PAIR> queriers;
}

static napi_value Write(napi_env env, napi_callback_info info)
{
    if (!NapiHiSysEventUtil::IsSystemAppCall()) {
        NapiHiSysEventUtil::ThrowSystemAppPermissionError(env);
        return nullptr;
    }
    size_t paramNum = WRITE_FUNC_MAX_PARAM_NUM;
    napi_value params[WRITE_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    napi_value val = nullptr;
    napi_get_undefined(env, &val);
    if (paramNum < WRITE_FUNC_MAX_PARAM_NUM - 1) {
        HILOG_ERROR(LOG_CORE,
            "count of parameters is not equal to %{public}zu or %{public}zu.",
            WRITE_FUNC_MAX_PARAM_NUM - 1, WRITE_FUNC_MAX_PARAM_NUM);
        NapiHiSysEventUtil::ThrowParamMandatoryError(env, "info");
        return val;
    }
    HiSysEventAsyncContext* asyncContext = new(std::nothrow) HiSysEventAsyncContext {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (asyncContext == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to new HiSysEventAsyncContext.");
        return val;
    }
    NapiHiSysEventUtil::ParseHiSysEventInfo(env, params, paramNum, asyncContext->eventInfo);
    NapiHiSysEventAdapter::ParseJsCallerInfo(env, asyncContext->jsCallerInfo);
    asyncContext->eventWroteResult = SUCCESS;
    // set callback function if it exists
    if (paramNum == WRITE_FUNC_MAX_PARAM_NUM) {
        napi_valuetype lastParamType;
        napi_typeof(env, params[paramNum - 1], &lastParamType);
        if (lastParamType == napi_valuetype::napi_function) {
            napi_create_reference(env, params[paramNum - 1], 1, &asyncContext->callback);
        }
    } else if (paramNum > WRITE_FUNC_MAX_PARAM_NUM) {
        HILOG_WARN(LOG_CORE, "count of params is invalid =%{public}d.", static_cast<int>(paramNum));
    }
    // set promise object if callback function is null
    napi_value promise = nullptr;
    napi_get_undefined(env, &promise);
    if (asyncContext->callback == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    }
    NapiHiSysEventAdapter::Write(env, asyncContext);
    return promise;
}

static napi_value AddWatcher(napi_env env, napi_callback_info info)
{
    if (!NapiHiSysEventUtil::IsSystemAppCall()) {
        NapiHiSysEventUtil::ThrowSystemAppPermissionError(env);
        return nullptr;
    }
    size_t paramNum = ADD_LISTENER_FUNC_MAX_PARAM_NUM;
    napi_value params[ADD_LISTENER_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum < ADD_LISTENER_FUNC_MAX_PARAM_NUM) {
        HILOG_ERROR(LOG_CORE, "count of parameters is less than %{public}zu.", ADD_LISTENER_FUNC_MAX_PARAM_NUM);
        NapiHiSysEventUtil::ThrowParamMandatoryError(env, "watcher");
        return nullptr;
    }
    std::vector<ListenerRule> rules;
    napi_value jsRulesVal = NapiHiSysEventUtil::GetPropertyByName(env, params[ADD_LISTENER_LISTENER_PARAM_INDEX],
        RULES_ATTR);
    if (auto ret = NapiHiSysEventUtil::ParseListenerRules(env, jsRulesVal, rules);
        ret != SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to parse watch rules, result code is %{public}d.", ret);
        return nullptr;
    }
    CallbackContext* callbackContext = new CallbackContext();
    callbackContext->env = env;
    callbackContext->threadId = getproctid();
    napi_create_reference(env, params[ADD_LISTENER_LISTENER_PARAM_INDEX], 1, &callbackContext->ref);
    std::shared_ptr<NapiHiSysEventListener> listener = std::make_shared<NapiHiSysEventListener>(callbackContext);
    auto ret = HiSysEventBaseManager::AddListener(listener, rules);
    if (ret != NAPI_SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to add event listener, result code is %{public}d.", ret);
        NapiHiSysEventUtil::ThrowErrorByRet(env, ret);
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(g_listenerMapMutex);
    listeners[callbackContext->ref] = std::make_pair(callbackContext->threadId, listener);
    return nullptr;
}

static napi_value RemoveWatcher(napi_env env, napi_callback_info info)
{
    if (!NapiHiSysEventUtil::IsSystemAppCall()) {
        NapiHiSysEventUtil::ThrowSystemAppPermissionError(env);
        return nullptr;
    }
    size_t paramNum = REMOVE_LISTENER_FUNC_MAX_PARAM_NUM;
    napi_value params[REMOVE_LISTENER_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum < REMOVE_LISTENER_FUNC_MAX_PARAM_NUM) {
        HILOG_ERROR(LOG_CORE, "count of parameters is less than %{public}zu.", REMOVE_LISTENER_FUNC_MAX_PARAM_NUM);
        NapiHiSysEventUtil::ThrowParamMandatoryError(env, "watcher");
        return nullptr;
    }
    auto iter = NapiHiSysEventUtil::CompareAndReturnCacheItem<NapiHiSysEventListener>(env,
        params[REMOVE_LISTENER_LISTENER_PARAM_INDEX], listeners);
    if (iter == listeners.end()) {
        HILOG_ERROR(LOG_CORE, "listener not exist.");
        NapiHiSysEventUtil::ThrowErrorByRet(env, ERR_NAPI_LISTENER_NOT_FOUND);
        return nullptr;
    }
    if (auto ret = HiSysEventBaseManager::RemoveListener(iter->second.second);
        ret != NAPI_SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to remove event listener, result code is %{public}d.", ret);
        NapiHiSysEventUtil::ThrowErrorByRet(env, ret);
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(g_listenerMapMutex);
    listeners.erase(iter->first);
    return nullptr;
}

static napi_value Query(napi_env env, napi_callback_info info)
{
    if (!NapiHiSysEventUtil::IsSystemAppCall()) {
        NapiHiSysEventUtil::ThrowSystemAppPermissionError(env);
        return nullptr;
    }
    size_t paramNum = QUERY_FUNC_MAX_PARAM_NUM;
    napi_value params[QUERY_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum < QUERY_FUNC_MAX_PARAM_NUM) {
        std::unordered_map<int32_t, std::string> paramError = {
            {QUERY_QUERY_ARG_PARAM_INDEX, "queryArg"},
            {QUERY_RULE_ARRAY_PARAM_INDEX, "rules"},
            {QUERY_QUERIER_PARAM_INDEX, "querier"},
        };
        HILOG_ERROR(LOG_CORE, "count of parameters is less than %{public}zu.", QUERY_FUNC_MAX_PARAM_NUM);
        NapiHiSysEventUtil::ThrowParamMandatoryError(env, paramError.at(paramNum));
        return nullptr;
    }
    QueryArg queryArg = { DEFAULT_TIME_STAMP, DEFAULT_TIME_STAMP, DEFAULT_EVENT_COUNT };
    if (auto ret = NapiHiSysEventUtil::ParseQueryArg(env, params[QUERY_QUERY_ARG_PARAM_INDEX], queryArg);
        ret != SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to parse query arg, result code is %{public}d.", ret);
        return nullptr;
    }
    std::vector<QueryRule> rules;
    if (auto ret = NapiHiSysEventUtil::ParseQueryRules(env, params[QUERY_RULE_ARRAY_PARAM_INDEX], rules);
        ret != SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to parse query rules, result code is %{public}d.", ret);
        return nullptr;
    }
    if (NapiHiSysEventUtil::IsNullOrUndefined(env, params[QUERY_QUERIER_PARAM_INDEX])) {
        NapiHiSysEventUtil::ThrowParamTypeError(env, "querier", "Querier");
        HILOG_ERROR(LOG_CORE, "querier is null or undefined.");
        return nullptr;
    }
    CallbackContext* callbackContext = new CallbackContext();
    callbackContext->env = env;
    callbackContext->threadId = getproctid();
    napi_create_reference(env, params[QUERY_QUERIER_PARAM_INDEX], 1, &callbackContext->ref);
    std::shared_ptr<NapiHiSysEventQuerier> querier = std::make_shared<NapiHiSysEventQuerier>(callbackContext,
        [] (const napi_env env, const napi_ref ref) {
            napi_value querier = nullptr;
            napi_get_reference_value(env, ref, &querier);
            auto iter = NapiHiSysEventUtil::CompareAndReturnCacheItem<NapiHiSysEventQuerier>(env, querier, queriers);
            std::lock_guard<std::mutex> lock(g_querierMapMutex);
            if (iter != queriers.end()) {
                queriers.erase(iter->first);
            }
        });
    auto ret = HiSysEventBaseManager::Query(queryArg, rules, querier);
    if (ret != NAPI_SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to query hisysevent, result code is %{public}d.", ret);
        NapiHiSysEventUtil::ThrowErrorByRet(env, ret);
    }
    std::lock_guard<std::mutex> lock(g_querierMapMutex);
    queriers[callbackContext->ref] = std::make_pair(callbackContext->threadId, querier);
    return nullptr;
}

static napi_value ExportSysEvents(napi_env env, napi_callback_info info)
{
    if (!NapiHiSysEventUtil::IsSystemAppCall()) {
        NapiHiSysEventUtil::ThrowSystemAppPermissionError(env);
        return nullptr;
    }
    size_t paramNum = EXPORT_FUNC_MAX_PARAM_NUM;
    napi_value params[EXPORT_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum < EXPORT_FUNC_MAX_PARAM_NUM) {
        std::unordered_map<int32_t, std::string> paramError = {
            {EXPORT_QUERY_ARG_PARAM_INDEX, "queryArg"},
            {EXPORT_RULE_ARRAY_PARAM_INDEX, "rules"},
        };
        HILOG_ERROR(LOG_CORE, "count of parameters is less than %{public}zu.", EXPORT_FUNC_MAX_PARAM_NUM);
        NapiHiSysEventUtil::ThrowParamMandatoryError(env, paramError.at(paramNum));
        return nullptr;
    }
    QueryArg queryArg = { DEFAULT_TIME_STAMP, DEFAULT_TIME_STAMP, DEFAULT_EVENT_COUNT };
    if (auto ret = NapiHiSysEventUtil::ParseQueryArg(env, params[EXPORT_QUERY_ARG_PARAM_INDEX], queryArg);
        ret != SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to parse query arg, result code is %{public}d.", ret);
        return nullptr;
    }
    std::vector<QueryRule> rules;
    if (auto ret = NapiHiSysEventUtil::ParseQueryRules(env, params[EXPORT_RULE_ARRAY_PARAM_INDEX], rules);
        ret != SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to parse query rules, result code is %{public}d.", ret);
        return nullptr;
    }
    auto ret = HiSysEventBaseManager::Export(queryArg, rules);
    if (std::to_string(ret).length() < TIME_STAMP_LENGTH) {
        HILOG_ERROR(LOG_CORE, "failed to export event");
        int32_t retCode = static_cast<int32_t>(ret);
        NapiHiSysEventUtil::ThrowErrorByRet(env, retCode);
        return nullptr;
    }
    napi_value result = nullptr;
    NapiHiSysEventUtil::CreateInt64Value(env, ret, result);
    return result;
}

static napi_value Subscribe(napi_env env, napi_callback_info info)
{
    if (!NapiHiSysEventUtil::IsSystemAppCall()) {
        NapiHiSysEventUtil::ThrowSystemAppPermissionError(env);
        return nullptr;
    }
    size_t paramNum = SUBSCRIBE_FUNC_MAX_PARAM_NUM;
    napi_value params[SUBSCRIBE_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    if (paramNum < SUBSCRIBE_FUNC_MAX_PARAM_NUM) {
        HILOG_ERROR(LOG_CORE, "count of parameters is less than %{public}zu.", SUBSCRIBE_FUNC_MAX_PARAM_NUM);
        NapiHiSysEventUtil::ThrowParamMandatoryError(env, "rules");
        return nullptr;
    }
    std::vector<QueryRule> rules;
    if (auto ret = NapiHiSysEventUtil::ParseQueryRules(env, params[SUBSCRIBE_RULE_ARRAY_PARAM_INDEX], rules);
        ret != SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to parse query rules, result code is %{public}d.", ret);
        return nullptr;
    }
    auto ret = HiSysEventBaseManager::Subscribe(rules);
    if (std::to_string(ret).length() < TIME_STAMP_LENGTH) {
        HILOG_ERROR(LOG_CORE, "failed to subscribe event.");
        int32_t retCode = static_cast<int32_t>(ret);
        NapiHiSysEventUtil::ThrowErrorByRet(env, retCode);
        return nullptr;
    }
    napi_value result = nullptr;
    NapiHiSysEventUtil::CreateInt64Value(env, ret, result);
    return result;
}

static napi_value Unsubscribe(napi_env env, napi_callback_info info)
{
    if (!NapiHiSysEventUtil::IsSystemAppCall()) {
        NapiHiSysEventUtil::ThrowSystemAppPermissionError(env);
        return nullptr;
    }
    auto ret = HiSysEventBaseManager::Unsubscribe();
    if (ret != NAPI_SUCCESS) {
        HILOG_ERROR(LOG_CORE, "failed to unsubscribe, result code is %{public}d.", ret);
        int32_t retCode = static_cast<int32_t>(ret);
        NapiHiSysEventUtil::ThrowErrorByRet(env, retCode);
    }
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("write", Write),
        DECLARE_NAPI_FUNCTION("addWatcher", AddWatcher),
        DECLARE_NAPI_FUNCTION("removeWatcher", RemoveWatcher),
        DECLARE_NAPI_FUNCTION("query", Query),
        DECLARE_NAPI_FUNCTION("exportSysEvents", ExportSysEvents),
        DECLARE_NAPI_FUNCTION("subscribe", Subscribe),
        DECLARE_NAPI_FUNCTION("unsubscribe", Unsubscribe),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));

    // init EventType class, Event class and Param class
    InitNapiClass(env, exports);

    return exports;
}
EXTERN_C_END

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "hiSysEvent",
    .nm_priv = ((void*)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
