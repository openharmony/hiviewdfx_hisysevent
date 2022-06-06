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

#include "def.h"
#include "hilog/log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_hisysevent_init.h"
#include "napi_hisysevent_util.h"
#include "ret_def.h"

using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENT_NAPI" };
constexpr int WRITE_FUNC_MAX_PARAM_NUM = 2;
constexpr int ADD_LISTENER_FUNC_MAX_PARAM_NUM = 2;
constexpr int REMOVE_LISTENER_FUNC_MAX_PARAM_NUM = 1;
constexpr int QUERY_FUNC_MAX_PARAM_NUM = 3;
}

static napi_value Write(napi_env env, napi_callback_info info)
{
    size_t paramNum = WRITE_FUNC_MAX_PARAM_NUM;
    napi_value params[WRITE_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));

    napi_value val = nullptr;
    napi_get_undefined(env, &val);
    if (paramNum != WRITE_FUNC_MAX_PARAM_NUM - 1 && paramNum != WRITE_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL,
            "failed to write hisysevent, count of parameters is not equal to  %{public}d or %{public}d.",
            (WRITE_FUNC_MAX_PARAM_NUM - 1), WRITE_FUNC_MAX_PARAM_NUM);
        return val;
    }

    HiSysEventAsyncContext* asyncContext = new(std::nothrow) HiSysEventAsyncContext {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (asyncContext == nullptr) {
        HiLog::Error(LABEL, "failed to new HiSysEventAsyncContext.");
        return val;
    }
    NapiHiSysEventUtil::ParseHiSysEventInfo(env, params, paramNum, asyncContext->eventInfo);
    asyncContext->eventWroteResult = SUCCESS;

    // set callback function if it exists
    if (paramNum == WRITE_FUNC_MAX_PARAM_NUM) {
        napi_valuetype lastParamType;
        napi_typeof(env, params[paramNum - 1], &lastParamType);
        if (lastParamType == napi_valuetype::napi_function) {
            napi_create_reference(env, params[paramNum - 1], 1, &asyncContext->callback);
        }
    } else if (paramNum > WRITE_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL, "count of params is invalid =%{public}d.", (int)paramNum);
        asyncContext->eventWroteResult = ERROR_INVALID_PARAM_COUNT;
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

static napi_value AddListener(napi_env env, napi_callback_info info)
{
    size_t paramNum = ADD_LISTENER_FUNC_MAX_PARAM_NUM;
    napi_value params[ADD_LISTENER_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));
    
    int32_t ret = ERROR_FUNC_CALL_FAILED;
    napi_value val = nullptr;
    napi_get_boolean(env, ret, &val);
    if (paramNum != ADD_LISTENER_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL,
            "failed to add a hisysevent listener, count of parameters is not equal to %{public}d.",
            ADD_LISTENER_FUNC_MAX_PARAM_NUM);
        return val;
    }

    return val;
}

static napi_value RemoveListener(napi_env env, napi_callback_info info)
{
    size_t paramNum = REMOVE_LISTENER_FUNC_MAX_PARAM_NUM;
    napi_value params[REMOVE_LISTENER_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));

    int32_t ret = ERROR_FUNC_CALL_FAILED;
    napi_value val = nullptr;
    napi_get_boolean(env, ret, &val);
    if (paramNum != REMOVE_LISTENER_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL,
            "failed to remove a hisysevent listener,, count of parameters is not equal to %{public}d.",
            REMOVE_LISTENER_FUNC_MAX_PARAM_NUM);
        return val;
    }

    return val;
}

static napi_value Query(napi_env env, napi_callback_info info)
{
    size_t paramNum = QUERY_FUNC_MAX_PARAM_NUM;
    napi_value params[QUERY_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));

    int32_t ret = ERROR_FUNC_CALL_FAILED;
    napi_value val = nullptr;
    napi_get_boolean(env, ret, &val);
    if (paramNum != QUERY_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL,
            "failed to query hisysevent,, count of parameters is not equal to %{public}d.",
            QUERY_FUNC_MAX_PARAM_NUM);
        return val;
    }

    return val;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("write", Write),
        DECLARE_NAPI_FUNCTION("addListener", AddListener),
        DECLARE_NAPI_FUNCTION("removeListener", RemoveListener),
        DECLARE_NAPI_FUNCTION("query", Query),
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
    .nm_modname = "hisysevent",
    .nm_priv = ((void*)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
