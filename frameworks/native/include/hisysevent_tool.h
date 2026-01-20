/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef HISYSEVENT_TOOL_H
#define HISYSEVENT_TOOL_H

#include <functional>
#include <map>
#include <string>
#include <thread>

#include "hisysevent_manager.h"

namespace OHOS {
namespace HiviewDFX {
struct ArgStuct {
    bool real = false;
    bool checkValidEvent = false;
    bool history = false;
    RuleType ruleType = RuleType::WHOLE_WORD;
    int maxEvents = 10000; // 10000 is the default query count
    uint32_t eventType = 0;
    long long beginTime = -1; // -1 is the default begin timestamp
    long long endTime = -1; // -1 is the default end timestamp
    std::string domain;
    std::string eventName;
    std::string tag;
};

using OptHandler = std::function<void(struct ArgStuct&, const char*)>;

class HiSysEventTool {
public:
    HiSysEventTool(bool autoExit = true);
    ~HiSysEventTool() {}

public:
    void DoCmdHelp();
    bool DoAction();
    void InitOptHandlers();
    void NotifyClient();
    bool ParseCmdLine(int argc, char** argv);
    void WaitClient();

private:
    bool CheckCmdLine();
    void HandleInput(int argc, char** argv, const char* selection);

private:
    struct ArgStuct clientCmdArg_;
    bool autoExit_ = true;
    bool isCondNeedWait_ = true;
    std::condition_variable condvClient_;
    std::mutex mutexClient_;
    std::map<int, OptHandler> optHandlers_;
    bool isSupportEventCheck_ = false;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_TOOL_H
