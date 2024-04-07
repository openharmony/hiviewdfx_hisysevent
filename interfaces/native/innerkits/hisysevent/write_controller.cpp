/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "write_controller.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <mutex>
#include <ostream>
#include <sys/time.h>
#include <sstream>
#include <string>

#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "WRITE_CONTROLLER"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int SEC_TO_MILLS = 1000;
constexpr uint64_t PRIME = 0x100000001B3ULL;
constexpr uint64_t BASIS = 0xCBF29CE484222325ULL;
constexpr char CONNECTOR[] = "_";

uint64_t GenerateHash(const std::string& info)
{
    uint64_t ret {BASIS};
    const char* p = info.c_str();
    size_t infoLen = info.size();
    size_t infoLenLimit = 256;
    size_t hashLen = (infoLen < infoLenLimit) ? infoLen : infoLenLimit;
    size_t i = 0;
    while (i < hashLen) {
        ret ^= *(p + i);
        ret *= PRIME;
        i++;
    }
    return ret;
}
}

uint64_t WriteController::GetCurrentTimeMills()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

uint64_t WriteController::CheckLimitWritingEvent(const ControlParam& param, const char* domain, const char* eventName,
    const CallerInfo& callerInfo)
{
    std::lock_guard<std::mutex> lock(lmtMutex);
    uint64_t key = ConcatenateInfoAsKey(eventName, callerInfo.func, callerInfo.line);
    EventLimitStat stat = lruCache.Get(key);
    uint64_t cur = callerInfo.timeStamp;
    if (!stat.IsValid() || ((stat.begin / SEC_TO_MILLS) + param.period < (cur / SEC_TO_MILLS)) ||
        ((stat.begin / SEC_TO_MILLS) > (cur / SEC_TO_MILLS))) {
        stat.count = 1; // record the first event writing during one cycle
        stat.begin = cur;
        lruCache.Put(key, stat);
        return cur;
    }
    stat.count++;
    if (stat.count <= param.threshold) {
        lruCache.Put(key, stat);
        return cur;
    }
    lruCache.Put(key, stat);
    HILOG_DEBUG(LOG_CORE, "{.period = %{public}zu, .threshold = %{public}zu} "
        "[%{public}lld, %{public}lld] discard %{public}zu event(s) "
        "with domain %{public}s and name %{public}s which wrote in function %{public}s.",
        param.period, param.threshold, static_cast<long long>(stat.begin / SEC_TO_MILLS),
        static_cast<long long>(cur / SEC_TO_MILLS), stat.count - param.threshold,
        domain, eventName, callerInfo.func);
    return INVALID_TIME_STAMP;
}

uint64_t WriteController::CheckLimitWritingEvent(const ControlParam& param, const char* domain,
    const char* eventName, const char* func, int64_t line)
{
    CallerInfo info = {
        .func = func,
        .line = line,
        .timeStamp = GetCurrentTimeMills(),
    };
    return CheckLimitWritingEvent(param, domain, eventName, info);
}

uint64_t WriteController::ConcatenateInfoAsKey(const char* eventName, const char* func, int64_t line) const
{
    std::string key;
    key.append(eventName).append(CONNECTOR).append(func).append(CONNECTOR).append(std::to_string(line));
    return GenerateHash(key);
}
} // HiviewDFX
} // OHOS