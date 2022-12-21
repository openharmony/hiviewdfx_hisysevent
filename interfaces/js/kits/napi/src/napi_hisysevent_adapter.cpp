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

#include <cctype>

#include "def.h"
#include "hilog/log.h"
#include "napi_hisysevent_util.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "NAPI_HISYSEVENT_ADAPTER" };
constexpr size_t ERR_INDEX = 0;
constexpr size_t VAL_INDEX = 1;
constexpr size_t RET_SIZE = 2;
constexpr int64_t DEFAULT_LINE_NUM = -1;
constexpr char FUNC_SOURCE_NAME[] = "JSHiSysEventWrite";
constexpr int FUNC_NAME_INDEX = 1;
constexpr int LINE_INFO_INDEX = 2;
constexpr int LINE_INDEX = 1;
constexpr char CALL_FUNC_INFO_DELIMITER = ' ';
constexpr char CALL_LINE_INFO_DELIMITER= ':';
constexpr char PATH_DELIMITER = '/';

bool IsUninitializedJsCallerInfo(const JsCallerInfo& info)
{
    return info.first.empty() || info.second == DEFAULT_LINE_NUM;
}

void Split(const std::string& origin, char delimiter, std::vector<std::string>& ret)
{
    std::string::size_type start = 0;
    std::string::size_type end = origin.find(delimiter);
    while (end != std::string::npos) {
        if (end == start) {
            start++;
            end = origin.find(delimiter, start);
            continue;
        }
        ret.emplace_back(origin.substr(start, end - start));
        start = end + 1;
        end = origin.find(delimiter, start);
    }
    if (start != origin.length()) {
        ret.emplace_back(origin.substr(start));
    }
}

void ParseCallerInfoFromStackTrace(const std::string& stackTrace, JsCallerInfo& callerInfo)
{
    if (stackTrace.empty()) {
        HiLog::Error(LABEL, "js stack trace is invalid.");
        return;
    }
    std::vector<std::string> callInfos;
    Split(stackTrace, CALL_FUNC_INFO_DELIMITER, callInfos);
    if (callInfos.size() <= FUNC_NAME_INDEX) {
        HiLog::Error(LABEL, "js function name parsed failed.");
        return;
    }
    callerInfo.first = callInfos[FUNC_NAME_INDEX];
    if (callInfos.size() <= LINE_INFO_INDEX) {
        HiLog::Error(LABEL, "js function line info parsed failed.");
        return;
    }
    std::string callInfo = callInfos[LINE_INFO_INDEX];
    std::vector<std::string> lineInfos;
    Split(callInfo, CALL_LINE_INFO_DELIMITER, lineInfos);
    if (lineInfos.size() <= LINE_INDEX) {
        HiLog::Error(LABEL, "js function line number parsed failed.");
        return;
    }
    if (callerInfo.first == "anonymous") {
        auto fileName = lineInfos[LINE_INDEX - 1];
        auto pos = fileName.find_last_of(PATH_DELIMITER);
        callerInfo.first = (pos == std::string::npos) ? fileName : fileName.substr(++pos);
    }
    auto lineInfo = lineInfos[LINE_INDEX];
    if (std::any_of(lineInfo.begin(), lineInfo.end(), [] (auto& c) {
        return !isdigit(c);
    })) {
        callerInfo.second = DEFAULT_LINE_NUM;
        return;
    }
    callerInfo.second = static_cast<int64_t>(std::stoll(lineInfos[LINE_INDEX]));
}
}

void NapiHiSysEventAdapter::ParseJsCallerInfo(const napi_env env, JsCallerInfo& callerInfo)
{
    NativeEngine* engine = reinterpret_cast<NativeEngine*>(env);
    std::string stackTrace;
    if (!engine->BuildJsStackTrace(stackTrace)) {
        HiLog::Error(LABEL, "js stack trace build failed.");
        return;
    }
    ParseCallerInfoFromStackTrace(stackTrace, callerInfo);
}

void NapiHiSysEventAdapter::CheckThenWriteSysEvent(WriteController& controller,
    HiSysEventAsyncContext* eventAsyncContext)
{
    if (eventAsyncContext == nullptr) {
        return;
    }
    if (eventAsyncContext->eventWroteResult != SUCCESS) {
        return;
    }
    auto eventInfo = eventAsyncContext->eventInfo;
    auto jsCallerInfo = eventAsyncContext->jsCallerInfo;
    if (IsUninitializedJsCallerInfo(jsCallerInfo)) {
        eventAsyncContext->eventWroteResult = Write(eventInfo);
        return;
    }
    if (controller.CheckLimitWritingEvent(eventInfo.domain.c_str(), eventInfo.name.c_str(),
        jsCallerInfo.first.c_str(), jsCallerInfo.second)) {
        eventAsyncContext->eventWroteResult = ERR_WRITE_IN_HIGH_FREQ;
        return;
    }
    eventAsyncContext->eventWroteResult = Write(eventInfo);
}

void NapiHiSysEventAdapter::Write(const napi_env env, HiSysEventAsyncContext* eventAsyncContext)
{
    napi_value resource = nullptr;
    NapiHiSysEventUtil::CreateStringValue(env, FUNC_SOURCE_NAME, resource);
    napi_create_async_work(
        env, nullptr, resource,
        [] (napi_env env, void* data) {
            HiSysEventAsyncContext* eventAsyncContext = reinterpret_cast<HiSysEventAsyncContext*>(data);
            CheckThenWriteSysEvent(HiSysEvent::controller, eventAsyncContext);
        },
        [] (napi_env env, napi_status status, void* data) {
            HiSysEventAsyncContext* eventAsyncContext = reinterpret_cast<HiSysEventAsyncContext*>(data);
            napi_value results[RET_SIZE] = {0};
            auto isNormalWrote = eventAsyncContext->eventWroteResult == SUCCESS &&
                !NapiHiSysEventUtil::HasStrParamLenOverLimit(eventAsyncContext->eventInfo);
            if (isNormalWrote) {
                NapiHiSysEventUtil::CreateNull(env, results[ERR_INDEX]);
                NapiHiSysEventUtil::CreateInt32Value(env, eventAsyncContext->eventWroteResult, results[VAL_INDEX]);
            } else {
                NapiHiSysEventUtil::CreateNull(env, results[VAL_INDEX]);
                auto errorCode = eventAsyncContext->eventWroteResult == SUCCESS ? ERR_VALUE_LENGTH_TOO_LONG :
                    eventAsyncContext->eventWroteResult;
                results[ERR_INDEX] = NapiHiSysEventUtil::CreateErrorByRet(env, errorCode);
            }
            if (eventAsyncContext->deferred != nullptr) { // promise
                isNormalWrote ? napi_resolve_deferred(env, eventAsyncContext->deferred, results[VAL_INDEX]) :
                    napi_reject_deferred(env, eventAsyncContext->deferred, results[ERR_INDEX]);
            } else {
                napi_value callback = nullptr;
                napi_get_reference_value(env, eventAsyncContext->callback, &callback);
                napi_value retValue = nullptr;
                napi_call_function(env, nullptr, callback, RET_SIZE, results, &retValue);
                napi_delete_reference(env, eventAsyncContext->callback);
            }
            napi_delete_async_work(env, eventAsyncContext->asyncWork);
            delete eventAsyncContext;
        }, reinterpret_cast<void*>(eventAsyncContext), &eventAsyncContext->asyncWork);
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