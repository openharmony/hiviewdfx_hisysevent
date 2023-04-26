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

#include <string>
#include <unordered_map>
#include <vector>

#include "hisysevent.h"
#include "inner_writer.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "write_controller.h"

namespace OHOS {
namespace HiviewDFX {
using namespace Encode;
using JsCallerInfo = std::pair<std::string, int64_t>;
using HiSysEventInfo = struct HiSysEventInfo {
    std::string domain;
    std::string name;
    HiSysEvent::EventType eventType;
    std::unordered_map<std::string, bool> boolParams;
    std::unordered_map<std::string, std::vector<bool>> boolArrayParams;
    std::unordered_map<std::string, double> doubleParams;
    std::unordered_map<std::string, std::vector<double>> doubleArrayParams;
    std::unordered_map<std::string, std::string> stringParams;
    std::unordered_map<std::string, std::vector<std::string>> stringArrayParams;
};

using HiSysEventAsyncContext = struct HiSysEventAsyncContext {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    HiSysEventInfo eventInfo;
    int eventWroteResult;
    JsCallerInfo jsCallerInfo;
};

class NapiHiSysEventAdapter {
public:
    static void Write(const napi_env env, HiSysEventAsyncContext* eventAsyncContext);
    static void ParseJsCallerInfo(const napi_env env, JsCallerInfo& callerInfo);

private:
    static void CheckThenWriteSysEvent(HiSysEventAsyncContext* eventAsyncContext);
    static void InnerWrite(InnerWriter::EventBase& eventBase, const HiSysEventInfo& eventInfo);
    static int Write(const HiSysEventInfo& eventInfo);

private:
    template<typename T>
    static void AppendParams(InnerWriter::EventBase& eventBase, const std::unordered_map<std::string, T>& params)
    {
        for (auto iter = params.cbegin(); iter != params.cend(); ++iter) {
            if (!InnerWriter::CheckParamValidity(eventBase, iter->first)) {
                continue;
            }
            if constexpr (std::is_same_v<std::decay_t<T>, const char*> ||
                std::is_same_v<std::decay_t<T>, std::string>) {
                InnerWriter::IsWarnAndUpdate(InnerWriter::CheckValue(iter->second), eventBase);
                eventBase.GetEventBuilder().AppendValue(
                    std::make_shared<StringEncodedParam>(iter->first, iter->second));
            }
            if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
                eventBase.GetEventBuilder().AppendValue(
                    std::make_shared<SignedVarintEncodedParam<bool>>(iter->first, iter->second));
            }
            if constexpr (std::is_same_v<std::decay_t<T>, double>) {
                eventBase.GetEventBuilder().AppendValue(
                    std::make_shared<FloatingNumberEncodedParam<double>>(iter->first, iter->second));
            }
        }
        AppendArrayParams(eventBase, params);
    }

    template<typename T>
    static void AppendArrayParams(InnerWriter::EventBase& eventBase, const std::unordered_map<std::string, T>& params)
    {
        for (auto iter = params.cbegin(); iter != params.cend(); ++iter) {
            if constexpr (std::is_same_v<std::decay_t<T>, std::vector<std::string>> ||
                std::is_same_v<std::decay_t<T>, std::vector<const char*>>) {
                if (!InnerWriter::CheckArrayParamsValidity(eventBase, iter->first, iter->second)) {
                    continue;
                }
                for (auto& item : iter->second) {
                    InnerWriter::IsWarnAndUpdate(InnerWriter::CheckValue(item), eventBase);
                }
                eventBase.GetEventBuilder().AppendValue(
                    std::make_shared<StringEncodedArrayParam>(iter->first, iter->second));
            }
            if constexpr (std::is_same_v<std::decay_t<T>, std::vector<bool>>) {
                if (!InnerWriter::CheckArrayParamsValidity(eventBase, iter->first, iter->second)) {
                    continue;
                }
                eventBase.GetEventBuilder().AppendValue(
                    std::make_shared<SignedVarintEncodedArrayParam<bool>>(iter->first, iter->second));
            }
            if constexpr (std::is_same_v<std::decay_t<T>, std::vector<double>>) {
                if (!InnerWriter::CheckArrayParamsValidity(eventBase, iter->first, iter->second)) {
                    continue;
                }
                eventBase.GetEventBuilder().AppendValue(
                    std::make_shared<FloatingNumberEncodedArrayParam<double>>(iter->first, iter->second));
            }
        }
    }
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEWDFX_NAPI_HISYSEVENT_ADAPTER_H