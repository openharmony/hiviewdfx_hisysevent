/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "event_socket_factory.h"

#include "def.h"
#include "hisysevent.h"
#include "hilog/log.h"
#include "raw_data_base_def.h"

#include <algorithm>
#include <list>
#include <string>

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "EVENT_SOCKET_FACTORY"

namespace OHOS {
namespace HiviewDFX {
namespace {
struct sockaddr_un normalAddr = {
    .sun_family = AF_UNIX,
    .sun_path = "/dev/unix/socket/hisysevent",
};

struct sockaddr_un higherPriorityAddr = {
    .sun_family = AF_UNIX,
    .sun_path = "/dev/unix/socket/hisysevent_fast",
};

inline bool IsHigherPriorityEventName(const std::list<std::string>& events, const std::string& name)
{
    auto iter = std::find(events.begin(), events.end(), name);
    return iter != events.end();
}

bool IsHigherPriorityAafwkEvent(const std::string& name)
{
    static std::list<std::string> events {
        "APP_INPUT_BLOCK", "BUSSINESS_THREAD_BLOCK_3S", "BUSSINESS_THREAD_BLOCK_6S",
        "LIFECYCLE_HALF_TIMEOUT", "LIFECYCLE_TIMEOUT", "THREAD_BLOCK_3S", "THREAD_BLOCK_6S" };
    return IsHigherPriorityEventName(events, name);
}

bool IsHigerPriorityAceEvent(const std::string& name)
{
    static std::list<std::string> events {
        "UI_BLOCK_3S", "UI_BLOCK_6S", "UI_BLOCK_RECOVERED" };
    return IsHigherPriorityEventName(events, name);
}

bool IsHigerPriorityFrameworkEvent(const std::string& name)
{
    static std::list<std::string> events {
        "IPC_FULL", "IPC_FULL_WARNING", "SERVICE_BLOCK", "SERVICE_TIMEOUT", "SERVICE_WARNING" };
    return IsHigherPriorityEventName(events, name);
}

bool IsHigherPriorityEvent(const std::string& domain, const std::string& name, int type)
{
    if (domain == "AAFWK") {
        return IsHigherPriorityAafwkEvent(name);
    } else if (domain == "ACE") {
        return IsHigerPriorityAceEvent(name);
    } else if (domain == "FRAMEWORK") {
        return IsHigerPriorityFrameworkEvent(name);
    } else if (domain == "RELIABILITY") {
        return type == HiSysEvent::EventType::FAULT;
    } else if (domain == "GRAPHIC") {
        return name == "NO_DRAW";
    } else if (domain == "MULTIMODALINPUT") {
        return name == "TARGET_POINTER_EVENT_FAILURE";
    } else if (domain == "POWER") {
        return name == "SCREEN_ON_TIMEOUT";
    } else if (domain == "WINDOWMANAGER") {
        return name == "NO_FOCUS_WINDOW";
    } else if (domain == "SCHEDULE_EXT") {
        return name == "SYSTEM_LOAD_LEVEL_CHANGED";
    } else {
        return false;
    }
}

void ParseEventInfo(RawData& data, std::string& domain, std::string& name, int& type)
{
    if (size_t len = data.GetDataLength(); len < sizeof(int32_t) + sizeof(struct HiSysEventHeader)) {
        HILOG_WARN(LOG_CORE, "length[%{public}zu] of data is invalid", len);
        return;
    }
    auto originalData = data.GetData();
    struct HiSysEventHeader header = *(reinterpret_cast<struct HiSysEventHeader*>(originalData + sizeof(int32_t)));
    domain = std::string(header.domain);
    name = std::string(header.name);
    type = static_cast<int>(header.type) + 1; // transform type to HiSysEvent::EventType
}
}

EventSocket& EventSocketFactory::GetEventSocket(RawData& data)
{
    std::string domain;
    std::string name;
    int type = HiSysEvent::EventType::FAULT;
    ParseEventInfo(data, domain, name, type);
    return IsHigherPriorityEvent(domain, name, type) ? higherPriorityAddr : normalAddr;
}
}
}
