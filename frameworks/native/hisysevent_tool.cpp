/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "hisysevent_tool.h"

#include <getopt.h>
#include <iostream>
#include <unistd.h>

#include "hisysevent_tool_listener.h"
#include "hisysevent_tool_query.h"

using namespace std;

#define RULETYPE_TO_STR(ruleType) #ruleType
#define COMPARE_RULETYPE_WITH_ARG(ruleType, arg) \
    if (RULETYPE_TO_STR(ruleType) ==             \
        string("RuleType::").append(arg)) {      \
        return ruleType;                         \
    }

namespace OHOS {
namespace HiviewDFX {
HiSysEventTool::HiSysEventTool() : clientCmdArg {
    false, "", "", "", RuleType::WHOLE_WORD,
    false, false, -1, -1, 10000 } {}

RuleType GetRuleTypeFromArg(const string& arg)
{
    COMPARE_RULETYPE_WITH_ARG(RuleType::WHOLE_WORD, arg)
    COMPARE_RULETYPE_WITH_ARG(RuleType::PREFIX, arg)
    COMPARE_RULETYPE_WITH_ARG(RuleType::REGULAR, arg)
    return RuleType::WHOLE_WORD;
}

bool HiSysEventTool::ParseCmdLine(int argc, char** argv)
{
    int opt;
    char string[] = "ar:o:n:t:ls:e:m:dh";
    if (argc > 1) {
        while ((opt = getopt(argc, argv, string)) != -1) {
            switch (opt) {
                case 'a':
                    clientCmdArg.real = true;
                    break;
                case 'r':
                    clientCmdArg.ruleType = GetRuleTypeFromArg(optarg);
                    break;
                case 'o':
                    clientCmdArg.domain = optarg;
                    break;
                case 'n':
                    clientCmdArg.eventName = optarg;
                    break;
                case 't':
                    clientCmdArg.tag = optarg;
                    break;
                case 'l':
                    clientCmdArg.history = true;
                    break;
                case 's':
                    clientCmdArg.beginTime = strtoll(optarg, nullptr, 0);
                    break;
                case 'e':
                    clientCmdArg.endTime = strtoll(optarg, nullptr, 0);
                    break;
                case 'm':
                    clientCmdArg.maxEvents = strtol(optarg, nullptr, 0);
                    break;
                case 'd':
                    clientCmdArg.isDebug = true;
                    break;
                case 'h':
                    DoCmdHelp();
                    _exit(0);
                    break;
                default:
                    break;
            }
        }
    }
    return CheckCmdLine();
}

bool HiSysEventTool::CheckCmdLine()
{
    if (!clientCmdArg.real && !clientCmdArg.history) {
        return false;
    }

    if (clientCmdArg.real && clientCmdArg.history) {
        cout << "canot read both read && history hisysevent" << endl;
        return false;
    }

    if (clientCmdArg.isDebug && !clientCmdArg.real) {
        cout << "debug must follow with real log" << endl;
        return false;
    }

    if (clientCmdArg.history) {
        if (clientCmdArg.endTime > 0 && clientCmdArg.beginTime > clientCmdArg.endTime) {
            cout << "invalid time startTime must less than endTime(";
            cout << clientCmdArg.beginTime << " > " << clientCmdArg.endTime << ")." << endl;
            return false;
        }
    }
    return true;
}

void HiSysEventTool::DoCmdHelp()
{
    cout << "hisysevent [-a [-d | -r [1|2|3] -t <tag> | -r [1|2|3] -o <domain> -n <eventName> ] |"
        "       -l [-s <time> -e <time> -m <count>]]" << endl;
    cout << "-a    subscribe a listener" << endl;
    cout << "-a -r [1|2|3] -t <tag>, subscribe by tag" << endl;
    cout << "-a -r [1|2|3] -o <domain> -n <eventName>, subscribe by domain and event name" << endl;
    cout << "-a -d set debug mode, both options must appear at the same time." << endl;
    cout << "-l -s <begin time> -e <end time> -m <max hisysevent count>" << endl;
    cout << "      get history hisysevent log, begin time should not be earlier than end time." << endl;
}

bool HiSysEventTool::DoAction()
{
    if (clientCmdArg.real) {
        auto toolListener = std::make_shared<HiSysEventToolListener>();
        std::vector<struct ListenerRule> sysRules;
        struct ListenerRule listenerRule(clientCmdArg.domain, clientCmdArg.eventName,
            clientCmdArg.tag, clientCmdArg.ruleType);
        sysRules.emplace_back(listenerRule);
        if (HiSysEventManager::AddEventListener(toolListener, sysRules)) {
            if (clientCmdArg.isDebug) {
                if (HiSysEventManager::SetDebugMode(toolListener, true)) {
                    return true;
                }
            } else {
                return true;
            }
        }
    }

    if (clientCmdArg.history) {
        auto queryCallBack = std::make_shared<HiSysEventToolQuery>();
        struct QueryArg args(clientCmdArg.beginTime, clientCmdArg.endTime, clientCmdArg.maxEvents);
        std::vector<struct QueryRule> mRules;
        if (HiSysEventManager::QueryHiSysEvent(args, mRules, queryCallBack)) {
            return true;
        }
    }
    return false;
}

void HiSysEventTool::WaitClient()
{
    if (clientCmdArg.real) {
        unique_lock<mutex> lock(mutexClient);
        condvClient.wait(lock);
    }
}

void HiSysEventTool::NotifyClient()
{
    condvClient.notify_one();
}
} // namespace HiviewDFX
} // namespace OHOS