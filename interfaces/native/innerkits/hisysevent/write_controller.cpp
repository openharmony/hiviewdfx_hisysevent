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
uint64_t GenerateHash(const std::string& info)
{
    const char* p = info.c_str();
    size_t infoLen = info.size();
    size_t infoLenLimit = 256;
    size_t hashLen = (infoLen < infoLenLimit) ? infoLen : infoLenLimit;
    size_t i = 0;
    uint64_t ret { 0xCBF29CE484222325ULL }; // hash basis value
    while (i < hashLen) {
        ret ^= *(p + i);
        ret *= 0x100000001B3ULL; // hash prime value
        i++;
    }
    return ret;
}

uint64_t ConcatenateInfoAsKey(const char* eventName, const char* func, int64_t line)
{
    std::string key;
    key.append(eventName).append("_").append(func).append("_").append(std::to_string(line));
    return GenerateHash(key);
}

struct EventWroteRecord {
    size_t count = 0;
    uint64_t timestamp = INVALID_TIME_STAMP;
};

struct EventWroteCacheNode {
    std::list<uint64_t>::const_iterator iter;
    struct EventWroteRecord record;
};
}

class EventWroteLruCache {
public:
    explicit EventWroteLruCache(size_t capacity): capacity_(capacity) {}

    ~EventWroteLruCache()
    {
        key2Index_.clear();
    }

public:
    struct EventWroteRecord Get(uint64_t key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        EventWroteRecord record;
        if (key2Index_.count(key) == 0) {
            return record;
        }
        Modify(key);
        return key2Index_[key].record;
    }

    void Put(uint64_t key, struct EventWroteRecord record)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (key2Index_.count(key) > 0) {
            key2Index_[key].record = record;
            Modify(key);
            return;
        }
        if (keyCache_.size() == capacity_) {
            key2Index_.erase(keyCache_.back());
            keyCache_.pop_back();
        }
        keyCache_.push_front(key);
        key2Index_[key] = {
            .iter = keyCache_.cbegin(),
            .record = record
        };
    }

private:
    void Modify(uint64_t key)
    {
        keyCache_.splice(keyCache_.begin(), keyCache_, key2Index_[key].iter);
        key2Index_[key].iter = keyCache_.cbegin();
    }

private:
    std::mutex mutex_;
    std::unordered_map<uint64_t, EventWroteCacheNode> key2Index_;
    std::list<uint64_t> keyCache_;
    size_t capacity_ = 0;
};

std::shared_ptr<EventWroteLruCache> WriteController::eventWroteLruCache_ =
    std::make_shared<EventWroteLruCache>(30); // 30 is the capacity of the lrucache

uint64_t WriteController::GetCurrentTimeMills()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

uint64_t WriteController::CheckLimitWritingEvent(const ControlParam& param, const char* domain, const char* eventName,
    const CallerInfo& callerInfo)
{
    uint64_t key = ConcatenateInfoAsKey(eventName, callerInfo.func, callerInfo.line);
    auto record = eventWroteLruCache_->Get(key);
    uint64_t cur = callerInfo.timeStamp;
    const uint64_t secToMillis = 1000; // second to millisecond
    if ((record.count == 0) || ((record.timestamp / secToMillis) + param.period < (cur / secToMillis)) ||
        ((record.timestamp / secToMillis) > (cur / secToMillis))) {
        record.count = 1; // record the first event writing during one cycle
        record.timestamp = cur;
        eventWroteLruCache_->Put(key, record);
        return cur;
    }
    record.count++;
    if (record.count <= param.threshold) {
        eventWroteLruCache_->Put(key, record);
        return cur;
    }
    eventWroteLruCache_->Put(key, record);
    HILOG_DEBUG(LOG_CORE, "{.period = %{public}zu, .threshold = %{public}zu} "
        "[%{public}lld, %{public}lld] discard %{public}zu event(s) "
        "with domain %{public}s and name %{public}s which wrote in function %{public}s.",
        param.period, param.threshold, static_cast<long long>(record.timestamp / secToMillis),
        static_cast<long long>(cur / secToMillis), record.count - param.threshold,
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
} // HiviewDFX
} // OHOS