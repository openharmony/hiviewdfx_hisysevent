/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef HIVIEWDFX_NAPI_HISYSEVENT_ADAPTER_H
#define HIVIEWDFX_NAPI_HISYSEVENT_ADAPTER_H

#include <cinttypes>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "hisysevent.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "stringfilter.h"
#include "write_controller.h"

namespace OHOS {
namespace HiviewDFX {
using namespace Encoded;
using JsCallerInfo = std::pair<std::string, int64_t>;
enum ParamType { BL = 0, DOU, I64, U64, STR, BOOL_ARR, DOUBLE_ARR, I64_ARR, U64_ARR, STR_ARR };
using Param = std::variant<bool, double, int64_t, uint64_t, std::string,
    std::vector<bool>, std::vector<double>, std::vector<int64_t>, std::vector<uint64_t>,
    std::vector<std::string>>;

using HiSysEventInfo = struct HiSysEventInfo {
    std::string domain;
    std::string name;
    HiSysEvent::EventType eventType;
    std::unordered_map<std::string, Param> params;
};

using HiSysEventAsyncContext = struct HiSysEventAsyncContext {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    HiSysEventInfo eventInfo;
    int eventWroteResult;
    JsCallerInfo jsCallerInfo;
    uint64_t timeStamp;
};

class NapiHiSysEventAdapter {
public:
    static void Write(const napi_env env, HiSysEventAsyncContext* eventAsyncContext);
    static void ParseJsCallerInfo(const napi_env env, JsCallerInfo& callerInfo);

private:
    static void CheckThenWriteSysEvent(HiSysEventAsyncContext* eventAsyncContext);
    static void InnerWrite(HiSysEvent::EventBase& eventBase, const HiSysEventInfo& eventInfo);
    static int Write(const HiSysEventInfo& eventInfo, uint64_t timeStamp);

private:
    static void AppendParams(HiSysEvent::EventBase& eventBase,
        const std::unordered_map<std::string, Param>& params)
    {
        const std::vector<std::function<void(const std::string&, const Param&)>> allParamsHandlers = {
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<BL>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<DOU>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<I64>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<U64>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<STR>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<BOOL_ARR>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<DOUBLE_ARR>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<I64_ARR>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<U64_ARR>(param));
            },
            [&] (const std::string& key, const Param& param) {
                (void)HiSysEvent::InnerWrite(eventBase, key, std::get<STR_ARR>(param));
            },
        };
        for (auto iter = params.cbegin(); iter != params.cend(); ++iter) {
            Param param = iter->second;
            size_t paramIndex = static_cast<size_t>(param.index());
            if (paramIndex >= allParamsHandlers.size()) {
                continue;
            }
            auto paramHandler = allParamsHandlers.at(paramIndex);
            if (paramHandler != nullptr) {
                paramHandler(iter->first, param);
            }
        }
    }
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEWDFX_NAPI_HISYSEVENT_ADAPTER_H