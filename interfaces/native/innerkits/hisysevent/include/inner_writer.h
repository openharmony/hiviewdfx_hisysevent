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

#ifndef HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_INNER_WRITER_H
#define HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_INNER_WRITER_H

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include "encoded_param.h"
#include "def.h"
#include "hisysevent_c.h"
#include "raw_data_builder.h"
#include "raw_data.h"

namespace OHOS {
namespace HiviewDFX {
using namespace Encode;
class InnerWriter {
    friend class HiSysEvent;
    friend class NapiHiSysEventAdapter;

private:
    class EventBase {
    public:
        EventBase(const std::string& domain, const std::string& eventName, int type);
        ~EventBase() = default;

    public:
        RawDataBuilder& GetEventBuilder();
        int GetRetCode();
        void SetRetCode(int retCode);
        size_t GetParamCnt();
        RawData& GetEventRawData();

    private:
        int retCode_;
        RawDataBuilder builder_;
    };

private:
    template<typename... Types>
    static int InnerWrite(const std::string& domain, const std::string& eventName,
        int type, Types... keyValues)
    {
        EventBase eventBase(domain, eventName, type);
        WritebaseInfo(eventBase);
        if (IsError(eventBase)) {
            ExplainRetCode(eventBase);
            return eventBase.GetRetCode();
        }

        InnerWrite(eventBase, keyValues...);
        if (IsError(eventBase)) {
            ExplainRetCode(eventBase);
            return eventBase.GetRetCode();
        }

        SendSysEvent(eventBase);
        return eventBase.GetRetCode();
    }

    static bool CheckParamValidity(EventBase& eventBase, const std::string &key)
    {
        if (IsWarnAndUpdate(CheckKey(key), eventBase)) {
            return false;
        }
        if (UpdateAndCheckKeyNumIsOver(eventBase)) {
            return false;
        }
        return true;
    }

    template<typename T>
    static bool CheckArrayValidity(EventBase& eventBase, const T* array)
    {
        if (array == nullptr) {
            eventBase.SetRetCode(ERR_VALUE_INVALID);
            return false;
        }
        return true;
    }

    template<typename T>
    static bool CheckArrayParamsValidity(EventBase& eventBase, const std::string& key, const std::vector<T>& value)
    {
        if (!CheckParamValidity(eventBase, key)) {
            return false;
        }
        if (value.empty()) {
            std::vector<bool> boolArrayValue;
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<bool>>(key,
                boolArrayValue));
            return false;
        }
        IsWarnAndUpdate(CheckArraySize(value.size()), eventBase);
        return true;
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, bool value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<bool>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const char value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<int8_t>>(key,
                static_cast<int8_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const unsigned char value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint8_t>>(key,
                static_cast<uint8_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const short value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<int16_t>>(key,
                static_cast<int16_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const unsigned short value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint16_t>>(key,
                static_cast<uint16_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const int value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<int32_t>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const unsigned int value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint32_t>>(key,
                value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const long value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<int64_t>>(key,
                static_cast<int64_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const unsigned long value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint64_t>>(key,
                static_cast<uint64_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const long long value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedParam<int64_t>>(key,
                static_cast<int64_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const unsigned long long value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint64_t>>(key,
                static_cast<uint64_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const float value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<FloatingNumberEncodedParam<float>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const double value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<FloatingNumberEncodedParam<double>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::string& value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            IsWarnAndUpdate(CheckValue(value), eventBase);
            eventBase.GetEventBuilder().AppendValue(std::make_shared<StringEncodedParam>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const char* value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            IsWarnAndUpdate(CheckValue(std::string(value)), eventBase);
            eventBase.GetEventBuilder().AppendValue(std::make_shared<StringEncodedParam>(key, std::string(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<bool>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<bool>>(key,
                value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<char>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<int8_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<int8_t>(item));
            }
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int8_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<unsigned char>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<uint8_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<uint8_t>(item));
            }
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedArrayParam<uint8_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<short>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<int16_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<int16_t>(item));
            }
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int16_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<unsigned short>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<uint16_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<uint16_t>(item));
            }
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedArrayParam<uint16_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<int>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int32_t>>(key,
                value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<unsigned int>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedArrayParam<uint32_t>>(key,
                value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<long>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<int64_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<int64_t>(item));
            }
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int64_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<unsigned long>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<uint64_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<uint64_t>(item));
            }
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedArrayParam<uint64_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<long long>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<int64_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<int64_t>(item));
            }
            eventBase.GetEventBuilder().AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int64_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<unsigned long long>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<uint64_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<uint64_t>(item));
            }
            eventBase.GetEventBuilder().AppendValue(std::make_shared<UnsignedVarintEncodedArrayParam<uint64_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<float>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<FloatingNumberEncodedArrayParam<float>>(key,
                value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<double>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            eventBase.GetEventBuilder().AppendValue(std::make_shared<FloatingNumberEncodedArrayParam<double>>(key,
                value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<std::string>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            for (auto& item : value) {
                IsWarnAndUpdate(CheckValue(item), eventBase);
            }
            eventBase.GetEventBuilder().AppendValue(std::make_shared<StringEncodedArrayParam>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

private:
    static void InnerWrite(EventBase& eventBase);
    static void InnerWrite(EventBase& eventBase, const HiSysEventParam params[], size_t size);
    static void WritebaseInfo(EventBase& eventBase);
    static void AppendHexData(EventBase& eventBase, const std::string& key, uint64_t value);

    static int CheckKey(const std::string& key);
    static int CheckValue(const std::string& value);
    static int CheckArraySize(const size_t size);
    static bool IsErrorAndUpdate(int retCode, EventBase& eventBase);
    static bool IsWarnAndUpdate(int retCode, EventBase& eventBase);
    static bool UpdateAndCheckKeyNumIsOver(EventBase& eventBase);
    static bool IsError(EventBase& eventBase);
    static void ExplainRetCode(EventBase& eventBase);

    static void SendSysEvent(EventBase& eventBase);

    static void AppendInvalidParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendBoolParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt8Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint8Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt16Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint16Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt32Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint32Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt64Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint64Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendFloatParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendDoubleParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendStringParam(EventBase& eventBase, const HiSysEventParam& param);

    static void AppendBoolArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt8ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint8ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt16ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint16ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt32ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint32ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt64ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint64ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendFloatArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendDoubleArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendStringArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendParam(EventBase& eventBase, const HiSysEventParam& param);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_INTERFACES_NATIVE_INNERKITS_HISYSEVENT_INCLUDE_INNER_WRITER_H

