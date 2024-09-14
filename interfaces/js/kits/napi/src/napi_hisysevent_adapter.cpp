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

#include "napi_hisysevent_adapter.h"

#include <cctype>
#include <memory>

#include "def.h"
#include "hilog/log.h"
#include "napi_hisysevent_util.h"
#include "napi/native_node_api.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "NAPI_HISYSEVENT_ADAPTER"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t ERR_INDEX = 0;
constexpr size_t VAL_INDEX = 1;
constexpr size_t RET_SIZE = 2;
constexpr int64_t DEFAULT_LINE_NUM = -1;
constexpr char FUNC_SOURCE_NAME[] = "JSHiSysEventWrite";
constexpr int FUNC_NAME_INDEX = 1;
constexpr int LINE_INFO_INDEX = 2;
constexpr int LINE_INDEX = 1;
constexpr char CALL_FUNC_INFO_DELIMITER = ' ';
constexpr char CALL_LINE_INFO_DELIMITER = ':';
constexpr char PATH_DELIMITER = '/';

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
        HILOG_ERROR(LOG_CORE, "js stack trace is invalid.");
        return;
    }
    std::vector<std::string> callInfos;
    Split(stackTrace, CALL_FUNC_INFO_DELIMITER, callInfos);
    if (callInfos.size() <= FUNC_NAME_INDEX) {
        HILOG_ERROR(LOG_CORE, "js function name parsed failed.");
        return;
    }
    callerInfo.first = callInfos[FUNC_NAME_INDEX];
    if (callInfos.size() <= LINE_INFO_INDEX) {
        HILOG_ERROR(LOG_CORE, "js function line info parsed failed.");
        return;
    }
    std::string callInfo = callInfos[LINE_INFO_INDEX];
    std::vector<std::string> lineInfos;
    Split(callInfo, CALL_LINE_INFO_DELIMITER, lineInfos);
    if (lineInfos.size() <= LINE_INDEX) {
        HILOG_ERROR(LOG_CORE, "js function line number parsed failed.");
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
    std::string stackTrace;
    if (napi_get_stack_trace(env, stackTrace) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "js stack trace build failed.");
        return;
    }
    ParseCallerInfoFromStackTrace(stackTrace, callerInfo);
}

void NapiHiSysEventAdapter::CheckThenWriteSysEvent(HiSysEventAsyncContext* eventAsyncContext)
{
    if (eventAsyncContext == nullptr) {
        return;
    }
    if (eventAsyncContext->eventWroteResult != SUCCESS) {
        return;
    }
    auto eventInfo = eventAsyncContext->eventInfo;
    auto jsCallerInfo = eventAsyncContext->jsCallerInfo;
    ControlParam param {
        .period = HISYSEVENT_DEFAULT_PERIOD,
        .threshold = HISYSEVENT_DEFAULT_THRESHOLD,
    };
    CallerInfo info = {
        .func = jsCallerInfo.first.c_str(),
        .line = jsCallerInfo.second,
        .timeStamp = eventAsyncContext->timeStamp,
    };
    uint64_t timeStamp = WriteController::CheckLimitWritingEvent(param, eventInfo.domain.c_str(),
        eventInfo.name.c_str(), info);
    if (timeStamp == INVALID_TIME_STAMP) {
        eventAsyncContext->eventWroteResult = ERR_WRITE_IN_HIGH_FREQ;
        return;
    }
    eventAsyncContext->eventWroteResult = Write(eventInfo, timeStamp);
}

void NapiHiSysEventAdapter::Write(const napi_env env, HiSysEventAsyncContext* eventAsyncContext)
{
    napi_value resource = nullptr;
    NapiHiSysEventUtil::CreateStringValue(env, FUNC_SOURCE_NAME, resource);
    eventAsyncContext->timeStamp = WriteController::GetCurrentTimeMills();
    napi_create_async_work(
        env, nullptr, resource,
        [] (napi_env env, void* data) {
            HiSysEventAsyncContext* eventAsyncContext = reinterpret_cast<HiSysEventAsyncContext*>(data);
            CheckThenWriteSysEvent(eventAsyncContext);
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
    napi_queue_async_work_with_qos(env, eventAsyncContext->asyncWork, napi_qos_default);
}

void NapiHiSysEventAdapter::InnerWrite(HiSysEvent::EventBase& eventBase,
    const HiSysEventInfo& eventInfo)
{
    AppendParams(eventBase, eventInfo.params);
}

int NapiHiSysEventAdapter::Write(const HiSysEventInfo& eventInfo, uint64_t timeStamp)
{
    if (!StringFilter::GetInstance().IsValidName(eventInfo.domain, MAX_DOMAIN_LENGTH)) {
        return HiSysEvent::ExplainThenReturnRetCode(ERR_DOMAIN_NAME_INVALID);
    }
    if (!StringFilter::GetInstance().IsValidName(eventInfo.name, MAX_EVENT_NAME_LENGTH)) {
        return HiSysEvent::ExplainThenReturnRetCode(ERR_EVENT_NAME_INVALID);
    }
    HiSysEvent::EventBase eventBase(eventInfo.domain, eventInfo.name, eventInfo.eventType, timeStamp);
    HiSysEvent::WritebaseInfo(eventBase);
    if (HiSysEvent::IsError(eventBase)) {
        return HiSysEvent::ExplainThenReturnRetCode(eventBase.GetRetCode());
    }

    InnerWrite(eventBase, eventInfo);
    HiSysEvent::InnerWrite(eventBase);
    if (HiSysEvent::IsError(eventBase)) {
        return HiSysEvent::ExplainThenReturnRetCode(eventBase.GetRetCode());
    }

    HiSysEvent::SendSysEvent(eventBase);
    return eventBase.GetRetCode();
}
} // namespace HiviewDFX
} // namespace OHOS
