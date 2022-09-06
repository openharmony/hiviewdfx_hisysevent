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
#include <map>
#include <unistd.h>

#include "hisysevent_tool_listener.h"
#include "hisysevent_tool_query.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char ARG_SELECTION[] = "vrc:o:n:t:ls:e:m:dh";
}
HiSysEventTool::HiSysEventTool() : clientCmdArg {
    false, false, "", "", "", RuleType::WHOLE_WORD,
    false, false, -1, -1, 10000 } {}

RuleType GetRuleTypeFromArg(const string& fromArgs)
{
    static std::map<const string, RuleType> ruleTypeMap {
        {"WHOLE_WORD", RuleType::WHOLE_WORD},
        {"PREFIX", RuleType::PREFIX},
        {"REGULAR", RuleType::REGULAR}
    };
    if (ruleTypeMap.find(fromArgs) != ruleTypeMap.end()) {
        return ruleTypeMap[fromArgs];
    }
    return RuleType::WHOLE_WORD;
}

bool HiSysEventTool::ParseCmdLine(int argc, char** argv)
{
    if (argv == nullptr) {
        return false;
    }
    if (argc > 1) {
        HandleInput(argc, argv, ARG_SELECTION);
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
        auto timestampValidCheck = clientCmdArg.endTime > 0
            && clientCmdArg.beginTime > clientCmdArg.endTime;
        if (timestampValidCheck) {
            cout << "invalid time startTime must less than endTime(";
            cout << clientCmdArg.beginTime << " > " << clientCmdArg.endTime << ")." << endl;
            return false;
        }
    }
    return true;
}

void HiSysEventTool::HandleInput(int argc, char** argv, const char* selection)
{
    int opt;
    while ((opt = getopt(argc, argv, selection)) != -1) {
        switch (opt) {
            case 'v':
                clientCmdArg.checkValidEvent = true;
                break;
            case 'r':
                clientCmdArg.real = true;
                break;
            case 'c':
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

void HiSysEventTool::DoCmdHelp()
{
    cout << "hisysevent [[-v] -r [-d | -c [WHOLE_WORD|PREFIX|REGULAR] -t <tag> "
        << "| -c [WHOLE_WORD|PREFIX|REGULAR] -o <domain> -n <eventName> ] "
        << "| -l [-s <time> -e <time> -m <count>]]" << endl;
    cout << "-r    subscribe on empty domain, eventname and tag" << endl;
    cout << "-r -c [WHOLE_WORD|PREFIX|REGULAR] -t <tag>, subscribe on tag" << endl;
    cout << "-r -c [WHOLE_WORD|PREFIX|REGULAR] -o <domain> -n <eventName>, "
        << "subscribe on domain and event name" << endl;
    cout << "-r -d set debug mode, both options must appear at the same time." << endl;
    cout << "-l -s <begin time> -e <end time> -m <max hisysevent count>" << endl;
    cout << "      get history hisysevent log, end time should not be "
        << "earlier than begin time." << endl;
    cout << "-v open valid event checking mode." << endl;
}

bool HiSysEventTool::DoAction()
{
    if (clientCmdArg.real) {
        auto toolListener = std::make_shared<HiSysEventToolListener>(clientCmdArg.checkValidEvent);
        if (toolListener == nullptr) {
            return false;
        }
        std::vector<ListenerRule> sysRules;
        ListenerRule listenerRule(clientCmdArg.domain, clientCmdArg.eventName,
            clientCmdArg.tag, clientCmdArg.ruleType);
        sysRules.emplace_back(listenerRule);
        auto listenerAddResult = HiSysEventManager::AddListener(toolListener, sysRules);
        if (listenerAddResult != 0 ||
            (clientCmdArg.isDebug && HiSysEventManager::SetDebugMode(toolListener, true) != 0)) {
            cout << "failed to subscribe sys event." << endl;
        }
        return true;
    }

    if (clientCmdArg.history) {
        auto queryCallBack = std::make_shared<HiSysEventToolQuery>(clientCmdArg.checkValidEvent);
        if (queryCallBack == nullptr) {
            return false;
        }
        struct QueryArg args(clientCmdArg.beginTime, clientCmdArg.endTime, clientCmdArg.maxEvents);
        std::vector<QueryRule> queryRules;
        if (!clientCmdArg.domain.empty() && !clientCmdArg.eventName.empty()) {
            QueryRule rule(clientCmdArg.domain, { clientCmdArg.eventName });
            queryRules.push_back(rule);
        }
        auto queryRet = HiSysEventManager::Query(args, queryRules, queryCallBack);
        if (queryRet != 0) {
            cout << "some errors happened when querying sys event, error code: \033[31m" << queryRet
                << "\033[0m." << endl;
        }
        return true;
    }
    return false;
}

void HiSysEventTool::WaitClient()
{
    unique_lock<mutex> lock(mutexClient);
    condvClient.wait(lock);
}

void HiSysEventTool::NotifyClient()
{
    condvClient.notify_one();
}
} // namespace HiviewDFX
} // namespace OHOS
