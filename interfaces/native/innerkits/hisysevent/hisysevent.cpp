/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "hisysevent.h"

#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>

#include "def.h"
#include "hilog/log.h"
#ifdef HIVIEWDFX_HITRACE_ENABLED
#include "hitrace/trace.h"
#endif
#include "securec.h"
#include "transport.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT"

namespace OHOS {
namespace HiviewDFX {
HiSysEvent::EventBase::EventBase(const std::string& domain, const std::string& eventName, int type,
    uint64_t timeStamp)
{
    retCode_ = 0;
    // append domain to header
    auto ret = memcpy_s(header_.domain, MAX_DOMAIN_LENGTH, domain.c_str(), domain.length());
    if (ret != EOK) {
        SetRetCode(ERR_RAW_DATA_WROTE_EXCEPTION);
        return;
    }
    header_.domain[domain.length()] = '\0';
    // append name to header
    ret = memcpy_s(header_.name, MAX_EVENT_NAME_LENGTH, eventName.c_str(), eventName.length());
    if (ret != EOK) {
        SetRetCode(ERR_RAW_DATA_WROTE_EXCEPTION);
        return;
    }
    header_.name[eventName.length()] = '\0';
    // append event type to header
    header_.type = static_cast<uint8_t>(type - 1);
    // append timestamp to header
    header_.timestamp = timeStamp;
}

int HiSysEvent::EventBase::GetRetCode()
{
    return retCode_;
}

void HiSysEvent::EventBase::SetRetCode(int retCode)
{
    retCode_ = retCode;
}

void HiSysEvent::EventBase::AppendParam(std::shared_ptr<Encoded::EncodedParam> param)
{
    if (param == nullptr) {
        return;
    }
    param->SetRawData(rawData_);
    if (param->Encode()) {
        paramCnt_++;
    }
}

void HiSysEvent::EventBase::WritebaseInfo()
{
    rawData_ = std::make_shared<RawData>();
    if (rawData_ == nullptr) {
        SetRetCode(ERR_RAW_DATA_WROTE_EXCEPTION);
        return;
    }
    tzset();
    header_.timeZone = static_cast<uint8_t>(ParseTimeZone(timezone));
    header_.pid = static_cast<uint32_t>(getprocpid());
    header_.tid = static_cast<uint32_t>(getproctid());
    header_.uid = static_cast<uint32_t>(getuid());
#ifdef HIVIEWDFX_HITRACE_ENABLED
    HiTraceId hitraceId = HiTraceChain::GetId();
    if (hitraceId.IsValid()) {
        header_.isTraceOpened = 1; // 1: include trace info, 0: exclude trace info.
        traceInfo_.traceId = hitraceId.GetChainId();
        traceInfo_.spanId = hitraceId.GetSpanId();
        traceInfo_.pSpanId = hitraceId.GetParentSpanId();
        traceInfo_.traceFlag = hitraceId.GetFlags();
    }
#else
    header_.isTraceOpened = 0; // 1: include trace info, 0: exclude trace info.
#endif
    int32_t blockSize = 0;
    if (!rawData_->Append(reinterpret_cast<uint8_t*>(&blockSize), sizeof(int32_t))) {
        SetRetCode(ERR_RAW_DATA_WROTE_EXCEPTION);
        return;
    }
    // append header to data.
    if (!rawData_->Append(reinterpret_cast<uint8_t*>(&header_), sizeof(struct HiSysEventHeader))) {
        SetRetCode(ERR_RAW_DATA_WROTE_EXCEPTION);
        return;
    }
    // append trace info to data.
    if (header_.isTraceOpened == 1 &&
        !rawData_->Append(reinterpret_cast<uint8_t*>(&traceInfo_), sizeof(struct TraceInfo))) {
        SetRetCode(ERR_RAW_DATA_WROTE_EXCEPTION);
        return;
    }
    paramCntWroteOffset_ = rawData_->GetDataLength();
    // append param count to data.
    if (!rawData_->Append(reinterpret_cast<uint8_t*>(&paramCnt_), sizeof(int32_t))) {
        SetRetCode(ERR_RAW_DATA_WROTE_EXCEPTION);
        return;
    }
}

size_t HiSysEvent::EventBase::GetParamCnt()
{
    return paramCnt_;
}

std::shared_ptr<Encoded::RawData> HiSysEvent::EventBase::GetEventRawData()
{
    if (rawData_ != nullptr) {
        auto blockSize = static_cast<int32_t>(rawData_->GetDataLength());
        (void)rawData_->Update(reinterpret_cast<uint8_t*>(&blockSize), sizeof(int32_t), 0);
        auto paramCnt = static_cast<int32_t>(paramCnt_);
        (void)rawData_->Update(reinterpret_cast<uint8_t*>(&paramCnt), sizeof(int32_t), paramCntWroteOffset_);
    }
    return rawData_;
}

int HiSysEvent::CheckKey(const std::string& key)
{
    if (!StringFilter::GetInstance().IsValidName(key, MAX_PARAM_NAME_LENGTH)) {
        return ERR_KEY_NAME_INVALID;
    }
    return SUCCESS;
}

int HiSysEvent::CheckValue(const std::string& value)
{
    if (value.length() > MAX_STRING_LENGTH) {
        return ERR_VALUE_LENGTH_TOO_LONG;
    }
    return SUCCESS;
}

int HiSysEvent::CheckArraySize(const size_t size)
{
    if (size > MAX_ARRAY_SIZE) {
        return ERR_ARRAY_TOO_MUCH;
    }
    return SUCCESS;
}

int HiSysEvent::ExplainThenReturnRetCode(const int retCode)
{
    if (retCode > SUCCESS) {
        HILOG_DEBUG(LOG_CORE, "some value of param discard as invalid data, error=%{public}d, message=%{public}s",
            retCode, ERR_MSG_LEVEL1[retCode - 1]);
    } else if (retCode < SUCCESS) {
        HILOG_DEBUG(LOG_CORE, "discard data, error=%{public}d, message=%{public}s",
            retCode, ERR_MSG_LEVEL0[-retCode - 1]);
    }
    return retCode;
}

bool HiSysEvent::IsError(EventBase& eventBase)
{
    return (eventBase.GetRetCode() < SUCCESS);
}

bool HiSysEvent::IsErrorAndUpdate(int retCode, EventBase& eventBase)
{
    if (retCode < SUCCESS) {
        eventBase.SetRetCode(retCode);
        return true;
    }
    return false;
}

bool HiSysEvent::IsWarnAndUpdate(int retCode, EventBase& eventBase)
{
    if (retCode != SUCCESS) {
        eventBase.SetRetCode(retCode);
        return true;
    }
    return false;
}

bool HiSysEvent::UpdateAndCheckKeyNumIsOver(EventBase& eventBase)
{
    if (eventBase.GetParamCnt() >= MAX_PARAM_NUMBER) {
        eventBase.SetRetCode(ERR_KEY_NUMBER_TOO_MUCH);
        return true;
    }
    return false;
}

void HiSysEvent::SendSysEvent(EventBase& eventBase)
{
    auto rawData = eventBase.GetEventRawData();
    if (rawData == nullptr) {
        eventBase.SetRetCode(ERR_RAW_DATA_WROTE_EXCEPTION);
        (void)ExplainThenReturnRetCode(ERR_RAW_DATA_WROTE_EXCEPTION);
        return;
    }
    int r = Transport::GetInstance().SendData(*(eventBase.GetEventRawData()));
    if (r != SUCCESS) {
        eventBase.SetRetCode(r);
        (void)ExplainThenReturnRetCode(r);
    }
}

void HiSysEvent::AppendHexData(EventBase& eventBase, const std::string& key, uint64_t value)
{
    eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedParam<uint64_t>>(key, value));
}

void HiSysEvent::WritebaseInfo(EventBase& eventBase)
{
    eventBase.WritebaseInfo();
}

void HiSysEvent::AppendInvalidParam(EventBase& eventBase, const HiSysEventParam& param)
{
    eventBase.SetRetCode(ERR_VALUE_INVALID);
}

void HiSysEvent::AppendBoolParam(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<bool>>(param.name, param.v.b));
}

void HiSysEvent::AppendInt8Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<int8_t>>(param.name, param.v.i8));
}

void HiSysEvent::AppendUint8Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedParam<uint8_t>>(param.name, param.v.ui8));
}

void HiSysEvent::AppendInt16Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<int16_t>>(param.name, param.v.i16));
}

void HiSysEvent::AppendUint16Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedParam<uint16_t>>(param.name, param.v.ui16));
}

void HiSysEvent::AppendInt32Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<int32_t>>(param.name, param.v.i32));
}

void HiSysEvent::AppendUint32Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedParam<uint32_t>>(param.name, param.v.ui32));
}

void HiSysEvent::AppendInt64Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<int64_t>>(param.name, param.v.i64));
}

void HiSysEvent::AppendUint64Param(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedParam<uint64_t>>(param.name, param.v.ui64));
}

void HiSysEvent::AppendFloatParam(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::FloatingNumberEncodedParam<float>>(param.name, param.v.f));
}

void HiSysEvent::AppendDoubleParam(EventBase& eventBase, const HiSysEventParam& param)
{
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::FloatingNumberEncodedParam<double>>(param.name, param.v.d));
}

void HiSysEvent::AppendStringParam(EventBase& eventBase, const HiSysEventParam& param)
{
    if (param.v.s == nullptr) {
        eventBase.SetRetCode(ERR_VALUE_INVALID);
        return;
    }
    if (!CheckParamValidity(eventBase, param.name)) {
        return;
    }
    IsWarnAndUpdate(CheckValue(std::string(param.v.s)), eventBase);
    auto rawStr = StringFilter::GetInstance().EscapeToRaw(std::string(param.v.s));
    eventBase.AppendParam(std::make_shared<Encoded::StringEncodedParam>(param.name, rawStr));
}

void HiSysEvent::AppendBoolArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    bool* array = reinterpret_cast<bool*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<bool> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<bool>>(param.name, value));
}

void HiSysEvent::AppendInt8ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    int8_t* array = reinterpret_cast<int8_t*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<int8_t> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<int8_t>>(param.name, value));
}

void HiSysEvent::AppendUint8ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    uint8_t* array = reinterpret_cast<uint8_t*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<uint8_t> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedArrayParam<uint8_t>>(param.name, value));
}

void HiSysEvent::AppendInt16ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    int16_t* array = reinterpret_cast<int16_t*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<int16_t> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<int16_t>>(param.name, value));
}

void HiSysEvent::AppendUint16ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    uint16_t* array = reinterpret_cast<uint16_t*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<uint16_t> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedArrayParam<uint16_t>>(param.name, value));
}

void HiSysEvent::AppendInt32ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    int32_t* array = reinterpret_cast<int32_t*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<int32_t> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<int32_t>>(param.name, value));
}

void HiSysEvent::AppendUint32ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    uint32_t* array = reinterpret_cast<uint32_t*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<uint32_t> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedArrayParam<uint32_t>>(param.name, value));
}

void HiSysEvent::AppendInt64ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    int64_t* array = reinterpret_cast<int64_t*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<int64_t> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<int64_t>>(param.name, value));
}

void HiSysEvent::AppendUint64ArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    uint64_t* array = reinterpret_cast<uint64_t*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<uint64_t> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedArrayParam<uint64_t>>(param.name, value));
}

void HiSysEvent::AppendFloatArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    float* array = reinterpret_cast<float*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<float> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::FloatingNumberEncodedArrayParam<float>>(param.name, value));
}

void HiSysEvent::AppendDoubleArrayParam(EventBase& eventBase, const HiSysEventParam& param)
{
    double* array = reinterpret_cast<double*>(param.v.array);
    if (!CheckArrayValidity(eventBase, array)) {
        return;
    }
    std::vector<double> value(array, array + param.arraySize);
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    eventBase.AppendParam(std::make_shared<Encoded::FloatingNumberEncodedArrayParam<double>>(param.name, value));
}

void HiSysEvent::AppendStringArrayParam(EventBase& eventBase, const HiSysEventParam& param)
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
    if (!CheckArrayParamsValidity(eventBase, param.name, value)) {
        return;
    }
    std::vector<std::string> rawStrs;
    for (auto& item : value) {
        IsWarnAndUpdate(CheckValue(item), eventBase);
        rawStrs.emplace_back(StringFilter::GetInstance().EscapeToRaw(item));
    }
    eventBase.AppendParam(std::make_shared<Encoded::StringEncodedArrayParam>(param.name, rawStrs));
}

void HiSysEvent::InnerWrite(EventBase& eventBase)
{
    // do nothing.
    HILOG_DEBUG(LOG_CORE, "hisysevent inner writer result: %{public}d.", eventBase.GetRetCode());
}

void HiSysEvent::InnerWrite(EventBase& eventBase, const HiSysEventParam params[], size_t size)
{
    if (params == nullptr || size == 0) {
        return;
    }
    for (size_t i = 0; i < size; ++i) {
        AppendParam(eventBase, params[i]);
    }
}

void HiSysEvent::AppendParam(EventBase& eventBase, const HiSysEventParam& param)
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