/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "inner_writer.h"

#include <cctype>
#include <chrono>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>

#include "def.h"
#include "hilog/log.h"
#include "hitrace/trace.h"
#include "transport.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HiSysEvent-InnerWriter" };
inline uint64_t GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

std::string GetTimeZone()
{
    struct timeval tv;
    if (gettimeofday(&tv, nullptr) != 0) {
        HiLog::Error(LABEL, "can not get tz");
        return "";
    }
    time_t sysSec = tv.tv_sec;
    struct tm tmLocal;
    if (localtime_r(&sysSec, &tmLocal) == nullptr) {
        HiLog::Error(LABEL, "failed to get local time.");
        return "";
    }
    int timeZoneBufSize = 20;
    char timeZone[timeZoneBufSize];
    auto ret = strftime(timeZone, timeZoneBufSize, "%z", &tmLocal);
    if (ret > 0) {
        return std::string(timeZone);
    }
    return std::string("+0000");
}
}

InnerWriter::EventBase::EventBase(const std::string& domain, const std::string& eventName, int type)
{
    retCode_ = 0;
    builder_ = RawDataBuilder(domain, eventName, type);
}

RawDataBuilder& InnerWriter::EventBase::GetEventBuilder()
{
    return builder_;
}

int InnerWriter::EventBase::GetRetCode()
{
    return retCode_;
}

void InnerWriter::EventBase::SetRetCode(int retCode)
{
    retCode_ = retCode;
}

size_t InnerWriter::EventBase::GetParamCnt()
{
    return builder_.GetParamCnt();
}

std::shared_ptr<RawData> InnerWriter::EventBase::GetEventRawData()
{
    return builder_.Build();
}

int InnerWriter::CheckKey(const std::string& key)
{
    if (!StringFilter::GetInstance().IsValidName(key, MAX_PARAM_NAME_LENGTH)) {
        return ERR_KEY_NAME_INVALID;
    }
    return SUCCESS;
}

int InnerWriter::CheckValue(const std::string& value)
{
    if (value.length() > MAX_STRING_LENGTH) {
        return ERR_VALUE_LENGTH_TOO_LONG;
    }
    return SUCCESS;
}

int InnerWriter::CheckArraySize(const size_t size)
{
    if (size > MAX_ARRAY_SIZE) {
        return ERR_ARRAY_TOO_MUCH;
    }
    return SUCCESS;
}

void InnerWriter::ExplainRetCode(EventBase& eventBase)
{
    auto retCode = eventBase.GetRetCode();
    if (retCode > SUCCESS) {
        HiLog::Warn(LABEL, "some value of param discard as invalid data, error=%{public}d, message=%{public}s",
            retCode, ERR_MSG_LEVEL1[retCode - 1]);
    } else if (retCode < SUCCESS) {
        HiLog::Error(LABEL, "discard data, error=%{public}d, message=%{public}s",
            retCode, ERR_MSG_LEVEL0[-retCode - 1]);
    }
}

bool InnerWriter::IsError(EventBase& eventBase)
{
    return (eventBase.GetRetCode() < SUCCESS);
}

bool InnerWriter::IsErrorAndUpdate(int retCode, EventBase& eventBase)
{
    if (retCode < SUCCESS) {
        eventBase.SetRetCode(retCode);
        return true;
    }
    return false;
}

bool InnerWriter::IsWarnAndUpdate(int retCode, EventBase& eventBase)
{
    if (retCode != SUCCESS) {
        eventBase.SetRetCode(retCode);
        return true;
    }
    return false;
}

bool InnerWriter::UpdateAndCheckKeyNumIsOver(EventBase& eventBase)
{
    if (eventBase.GetParamCnt() >= MAX_PARAM_NUMBER) {
        eventBase.SetRetCode(ERR_KEY_NUMBER_TOO_MUCH);
        return true;
    }
    return false;
}

void InnerWriter::SendSysEvent(EventBase& eventBase)
{
    auto rawData = *(eventBase.GetEventRawData());
    int r = Transport::GetInstance().SendData(rawData);
    if (r != SUCCESS) {
        eventBase.SetRetCode(r);
        ExplainRetCode(eventBase);
    }
}

void InnerWriter::AppendHexData(EventBase& eventBase, const std::string& key, uint64_t value)
{
    eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint64_t>>(key, value));
}

void InnerWriter::WritebaseInfo(EventBase& eventBase)
{
    if (!StringFilter::GetInstance().IsValidName(eventBase.GetEventBuilder().GetDomain(), MAX_DOMAIN_LENGTH)) {
        eventBase.SetRetCode(ERR_DOMAIN_NAME_INVALID);
        return;
    }
    if (!StringFilter::GetInstance().IsValidName(eventBase.GetEventBuilder().GetName(), MAX_EVENT_NAME_LENGTH)) {
        eventBase.SetRetCode(ERR_EVENT_NAME_INVALID);
        return;
    }
    eventBase.GetEventBuilder().AppendTimeStamp(GetMilliseconds()).AppendTimeZone(GetTimeZone())
        .AppendPid(getpid()).AppendTid(gettid()).AppendUid(getuid());

    HiTraceId hitraceId = HiTraceChain::GetId();
    if (!hitraceId.IsValid()) {
        return;
    }
    eventBase.GetEventBuilder().AppendTraceInfo(hitraceId.GetChainId(), hitraceId.GetSpanId(),
        hitraceId.GetParentSpanId(), hitraceId.GetFlags());
}

void InnerWriter::AppendInvalidParam(EventBase& eventBase, const HiSysEventParam& param)
{
    eventBase.SetRetCode(ERR_VALUE_INVALID);
}

void InnerWriter::AppendBoolParam(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<bool>>(param.name,
            param.v.b));
    }
}

void InnerWriter::AppendInt8Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<int8_t>>(param.name,
            param.v.i8));
    }
}

void InnerWriter::AppendUint8Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint8_t>>(param.name,
            param.v.ui8));
    }
}

void InnerWriter::AppendInt16Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<int16_t>>(param.name,
            param.v.i16));
    }
}

void InnerWriter::AppendUint16Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint16_t>>(param.name,
            param.v.ui16));
    }
}

void InnerWriter::AppendInt32Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<int32_t>>(param.name,
            param.v.i32));
    }
}

void InnerWriter::AppendUint32Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint32_t>>(param.name,
            param.v.ui32));
    }
}

void InnerWriter::AppendInt64Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<int64_t>>(param.name,
            param.v.i64));
    }
}

void InnerWriter::AppendUint64Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint64_t>>(param.name,
            param.v.ui64));
    }
}

void InnerWriter::AppendFloatParam(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<FloatingNumberEncodedParam<float>>(param.name,
            param.v.f));
    }
}

void InnerWriter::AppendDoubleParam(EventBase& eventBase, const HiSysEventParam& param)
{
    if (CheckParamValidity(eventBase, param.name)) {
        eventBase.GetEventBuilder().AppendValue(std::make_shared<FloatingNumberEncodedParam<double>>(param.name,
            param.v.d));
    }
}

void InnerWriter::AppendStringParam(EventBase& eventBase, const HiSysEventParam& param)
{
    if (param.v.s == nullptr) {
        eventBase.SetRetCode(ERR_VALUE_INVALID);
        return;
    }
    if (CheckParamValidity(eventBase, param.name)) {
        IsWarnAndUpdate(CheckValue(std::string(param.v.s)), eventBase);
        auto rawStr = StringFilter::GetInstance().EscapeToRaw(std::string(param.v.s));
        eventBase.GetEventBuilder().AppendValue(std::make_shared<StringEncodedParam>(param.name, rawStr));
    }
}

void InnerWriter::AppendBoolArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    bool* array = reinterpret_cast<bool*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<bool> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<bool>>(
                param.name, value));
        }
    }
}

void InnerWriter::AppendInt8ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    int8_t* array = reinterpret_cast<int8_t*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<int8_t> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int8_t>>(
                param.name, value));
        }
    }
}

void InnerWriter::AppendUint8ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    uint8_t* array = reinterpret_cast<uint8_t*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<uint8_t> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedArrayParam<uint8_t>>(
                param.name, value));
        }
    }
}

void InnerWriter::AppendInt16ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    int16_t* array = reinterpret_cast<int16_t*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<int16_t> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int16_t>>(
                param.name, value));
        }
    }
}

void InnerWriter::AppendUint16ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    uint16_t* array = reinterpret_cast<uint16_t*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<uint16_t> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedArrayParam<uint16_t>>(
                param.name, value));
        }
    }
}

void InnerWriter::AppendInt32ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    int32_t* array = reinterpret_cast<int32_t*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<int32_t> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int32_t>>(
                param.name, value));
        }
    }
}

void InnerWriter::AppendUint32ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    uint32_t* array = reinterpret_cast<uint32_t*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<uint32_t> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedArrayParam<uint32_t>>(
                param.name, value));
        }
    }
}

void InnerWriter::AppendInt64ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    int64_t* array = reinterpret_cast<int64_t*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<int64_t> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int64_t>>(
                param.name, value));
        }
    }
}

void InnerWriter::AppendUint64ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    uint64_t* array = reinterpret_cast<uint64_t*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<uint64_t> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(
                std::make_shared<UnsignedVarintEncodedArrayParam<uint64_t>>(param.name, value));
        }
    }
}

void InnerWriter::AppendFloatArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    float* array = reinterpret_cast<float*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<float> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<FloatingNumberEncodedArrayParam<float>>(
                param.name, value));
        }
    }
}

void InnerWriter::AppendDoubleArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    double* array = reinterpret_cast<double*>(param.v.array);
    if (CheckArrayValidity(eventBase, array)) {
        std::vector<double> value(array, array + param.arraySize);
        if (CheckArrayParamsValidity(eventBase, param.name, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<FloatingNumberEncodedArrayParam<double>>(
                param.name, value));
        }
    }
}

void InnerWriter::AppendStringArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    auto array = reinterpret_cast<char**>(param.v.array);
    if (array == nullptr) {
        eventBase.SetRetCode(ERR_VALUE_INVALID);
        return;
    }
    for (size_t i = 0; i < param.arraySize; ++i) {
        if (auto temp = array + i; *temp == nullptr) {
            eventBase.SetRetCode(ERR_VALUE_INVALID);
            return;
        }
    }
    std::vector<std::string> value(array, array + param.arraySize);
    if (CheckArrayParamsValidity(eventBase, param.name, value)) {
        std::vector<std::string> rawStrs;
        for (auto& item : value) {
            IsWarnAndUpdate(CheckValue(item), eventBase);
            rawStrs.emplace_back(StringFilter::GetInstance().EscapeToRaw(item));
        }
        eventBase.GetEventBuilder().AppendValue(std::make_shared<StringEncodedArrayParam>(param.name, rawStrs));
    }
}

void InnerWriter::InnerWrite(EventBase& eventBase)
{
    // do nothing.
    HiLog::Debug(LABEL, "hisysevent inner writer result: %{public}d.", eventBase.GetRetCode());
}

void InnerWriter::InnerWrite(EventBase& eventBase, const HiSysEventParam params[], size_t size)
{
    if (params == nullptr || size == 0) {
        return;
    }
    for (size_t i = 0; i < size; ++i) {
        AppendParam(eventBase, params[i]);
    }
}

void InnerWriter::AppendParam(EventBase& eventBase, const HiSysEventParam& param)
{
    using AppendParamFunc = void (*)(EventBase&, const HiSysEventParam&);
    constexpr int totalAppendFuncSize = 25;
    const AppendParamFunc appendFuncs[totalAppendFuncSize] = {
        &AppendInvalidParam, &AppendBoolParam, &AppendInt8Param, &AppendUint8Param,
        &AppendInt16Param, &AppendUint16Param, &AppendInt32Param, &AppendUint32Param,
        &AppendInt64Param, &AppendUint64Param, &AppendFloatParam, &AppendDoubleParam,
        &AppendStringParam, &AppendBoolArrayParam, &AppendInt8ArrayParam, &AppendUint8ArrayParam,
        &AppendInt16ArrayParam, &AppendUint16ArrayParam, &AppendInt32ArrayParam, &AppendUint32ArrayParam,
        &AppendInt64ArrayParam, &AppendUint64ArrayParam, &AppendFloatArrayParam, &AppendDoubleArrayParam,
        &AppendStringArrayParam,
    };
    if (size_t paramType = param.t; paramType < totalAppendFuncSize) {
        appendFuncs[paramType](eventBase, param);
    } else {
        eventBase.SetRetCode(ERR_VALUE_INVALID);
    }
}
} // namespace HiviewDFX
} // OHOS