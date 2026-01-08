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

#include "hisysevent_tool.h"

#include <charconv>
#include <fstream>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <regex>
#include <regex.h>
#include <sstream>
#include <unistd.h>

#include "hisysevent.h"
#include "hisysevent_tool_listener.h"
#include "hisysevent_tool_query.h"

#include "ret_code.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr uint32_t INVALID_EVENT_TYPE = 0;
constexpr int INVALID_ARG_OPT = -1;
constexpr long long DEFAULT_TIME_STAMP = -1;
constexpr long long SECONDS_2_MILLS = 1000;

template<typename T>
void ParseNumFromStr(const std::string& numStr, T& num)
{
    auto ret = std::from_chars(numStr.c_str(), numStr.c_str() + numStr.size(), num);
    if (ret.ec != std::errc()) {
        num = 0;
    }
}

inline bool IsSupportEventCheck()
{
    std::ifstream file;
    file.open("/data/system/hiview/unzip_configs/sys_event_def/hisysevent.def");
    return file.is_open();
}

RuleType GetRuleTypeFromArg(const string& fromArgs)
{
    static std::map<const string, RuleType> ruleTypeMap {
        { "WHOLE_WORD", RuleType::WHOLE_WORD },
        { "PREFIX", RuleType::PREFIX },
        { "REGULAR", RuleType::REGULAR }
    };
    if (ruleTypeMap.find(fromArgs) != ruleTypeMap.end()) {
        return ruleTypeMap[fromArgs];
    }
    return RuleType::WHOLE_WORD;
}

uint32_t GetEventTypeFromArg(const string& fromArgs)
{
    static std::map<const string, HiSysEvent::EventType> eventTypeMap {
        { "FAULT", HiSysEvent::EventType::FAULT },
        { "STATISTIC", HiSysEvent::EventType::STATISTIC },
        { "SECURITY", HiSysEvent::EventType::SECURITY },
        { "BEHAVIOR", HiSysEvent::EventType::BEHAVIOR }
    };
    if (eventTypeMap.find(fromArgs) != eventTypeMap.end()) {
        return static_cast<uint32_t>(eventTypeMap[fromArgs]);
    }
    return INVALID_EVENT_TYPE;
}

long long ParseTimeStampFromArgs(const string& fromArgs)
{
    regex formatRegex("[0-9]{4}-"
        "((0[13578]|1[02])-(0[1-9]|[12][0-9]|3[01])|(0[2469]|11)-(0[1-9]|[12][0-9]|30))"
        " ([01][0-9]|2[0-3])(:[0-5][0-9]){2}");
    smatch matchRet;
    if (!std::regex_match(fromArgs, matchRet, formatRegex)) {
        return DEFAULT_TIME_STAMP;
    }
    std::istringstream is(fromArgs);
    struct tm time = {};
    is >> std::get_time(&time, "%Y-%m-%d %H:%M:%S");
    return static_cast<long long>(mktime(&time)) * SECONDS_2_MILLS;
}

std::string GetErrorDescription(int32_t errCode)
{
    std::map<int32_t, std::string> errMap = {
        { ERR_SYS_EVENT_SERVICE_NOT_FOUND, "service not found." },
        { ERR_PARCEL_DATA_IS_NULL, "parcel data is null." },
        { ERR_REMOTE_SERVICE_IS_NULL, "remote service is null." },
        { ERR_CAN_NOT_WRITE_DESCRIPTOR, "descriptor wrote failed." },
        { ERR_CAN_NOT_WRITE_PARCEL, "parcel wrote failed." },
        { ERR_CAN_NOT_WRITE_REMOTE_OBJECT, "remote object wrote failed." },
        { ERR_CAN_NOT_SEND_REQ, "request sent failed." },
        { ERR_CAN_NOT_READ_PARCEL, "parcel read failed." },
        { ERR_ADD_DEATH_RECIPIENT, "add death recipient failed." },
        { ERR_QUERY_RULE_INVALID, "invalid query rule." },
        { ERR_TOO_MANY_WATCHERS, "too many wathers subscribed." },
        { ERR_QUERY_TOO_FREQUENTLY, "query too frequently." },
    };
    return errMap.find(errCode) == errMap.end() ?
        "unknown error." : errMap.at(errCode);
}

bool IsValidRegex(const std::string& regStr)
{
    if (regStr.length() > 32) { // 32 is the length limit of regex
        return false;
    }
    int flags = REG_EXTENDED;
    regex_t reg;
    // check whether the pattern is valid
    int status = regcomp(&reg, regStr.c_str(), flags);
    // free regex
    regfree(&reg);
    return (status == REG_OK);
}
}

HiSysEventTool::HiSysEventTool(bool autoExit)
{
    autoExit_ = autoExit;
    isSupportEventCheck_ = IsSupportEventCheck();
    InitOptHandlers();
}

void HiSysEventTool::InitOptHandlers()
{
    optHandlers_ = {
        {'r', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.real = true;
        }}, {'c', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.ruleType = GetRuleTypeFromArg(optarg);
        }}, {'o', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.domain = optarg;
        }}, {'n', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.eventName = optarg;
        }}, {'t', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.tag = optarg;
        }}, {'l', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.history = true;
        }}, {'s', [] (struct ArgStuct& cmdArg, const char* optarg) {
            ParseNumFromStr(optarg, cmdArg.beginTime);
        }}, {'S', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.beginTime = ParseTimeStampFromArgs(std::string(optarg));
        }}, {'e', [] (struct ArgStuct& cmdArg, const char* optarg) {
            ParseNumFromStr(optarg, cmdArg.endTime);
        }}, {'E', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.endTime = ParseTimeStampFromArgs(std::string(optarg));
        }}, {'m', [] (struct ArgStuct& cmdArg, const char* optarg) {
            ParseNumFromStr(optarg, cmdArg.maxEvents);
        }}, {'g', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.eventType = GetEventTypeFromArg(optarg);
        }},
    };
    if (isSupportEventCheck_) {
        optHandlers_.insert({'v', [] (struct ArgStuct& cmdArg, const char* optarg) {
            cmdArg.checkValidEvent = true;
        }});
    }
}

bool HiSysEventTool::ParseCmdLine(int argc, char** argv)
{
    if (argv == nullptr) {
        return false;
    }
    if (argc <= 1) {
        return CheckCmdLine();
    }
    if (isSupportEventCheck_) {
        HandleInput(argc, argv, "vrc:o:n:t:lS:s:E:e:m:hg:");
    } else {
        HandleInput(argc, argv, "rc:o:n:t:lS:s:E:e:m:hg:");
    }
    return CheckCmdLine();
}

bool HiSysEventTool::CheckCmdLine()
{
    if (!clientCmdArg_.real && !clientCmdArg_.history) {
        return false;
    }

    if (clientCmdArg_.real && clientCmdArg_.history) {
        cout << "canot read both read && history hisysevent" << endl;
        return false;
    }

    if (clientCmdArg_.history) {
        auto timestampValidCheck = clientCmdArg_.endTime > 0
            && clientCmdArg_.beginTime > clientCmdArg_.endTime;
        if (timestampValidCheck) {
            cout << "invalid time startTime must less than endTime(";
            cout << clientCmdArg_.beginTime << " > " << clientCmdArg_.endTime << ")." << endl;
            return false;
        }
    }
    return true;
}

void HiSysEventTool::HandleInput(int argc, char** argv, const char* selection)
{
    int opt;
    while ((opt = getopt(argc, argv, selection)) != INVALID_ARG_OPT) {
        if (opt == 'h') {
            DoCmdHelp();
            if (autoExit_) {
                _exit(0);
            }
        }
        if (optHandlers_.find(opt) != optHandlers_.end()) {
            optHandlers_.at(opt)(clientCmdArg_, optarg);
        }
    }
}

void HiSysEventTool::DoCmdHelp()
{
    cout << "hisysevent [";
    if (isSupportEventCheck_) {
        cout << "[-v] ";
    }
    cout << "-r [-c [WHOLE_WORD|PREFIX|REGULAR] -t <tag> "
        << "| -c [WHOLE_WORD|PREFIX|REGULAR] -o <domain> -n <eventName> "
        << "| -g [FAULT|STATISTIC|SECURITY|BEHAVIOR]] "
        << "| -l [[-s <begin time> -e <end time> | -S <formatted begin time> -E <formatted end time>] "
        << "-m <count> -c [WHOLE_WORD] -o <domain> -n <eventName> -g [FAULT|STATISTIC|SECURITY|BEHAVIOR]]]" << endl;
    cout << "-r,    subscribe on all domains, event names and tags." << endl;
    cout << "-r -c [WHOLE_WORD|PREFIX|REGULAR] -t <tag>"
        << ", subscribe on tag." << endl;
    cout << "-r -c [WHOLE_WORD|PREFIX|REGULAR] -o <domain> -n <eventName>"
        << ", subscribe on domain and event name." << endl;
    cout << "-r -g [FAULT|STATISTIC|SECURITY|BEHAVIOR]"
        << ", subscribe on event type." << endl;
    cout << "-l -s <begin time> -e <end time> -m <max hisysevent count>"
        << ", get history hisysevent log with time stamps, end time should not be "
        << "earlier than begin time." << endl;
    cout << "-l -S <formatted begin time> -E <formatted end time> -m <max hisysevent count>"
        << ", get history hisysevent log with formatted time string, end time should not be "
        << "earlier than begin time." << endl;
    cout << "-l -c [WHOLE_WORD] -o <domain> -n <eventName> -m <max hisysevent count>"
        << ", get history hisysevent log with domain and event name." << endl;
    cout << "-l -g [FAULT|STATISTIC|SECURITY|BEHAVIOR] -m <max hisysevent count>"
        << ", get history hisysevent log with event type." << endl;
    if (isSupportEventCheck_) {
        cout << "-v,    open valid event checking mode." << endl;
    }
    cout << "-h,    help manual." << endl;
}

bool HiSysEventTool::DoAction()
{
    if (clientCmdArg_.ruleType == RuleType::REGULAR && (!IsValidRegex(clientCmdArg_.domain)
        || !IsValidRegex(clientCmdArg_.eventName) || !IsValidRegex(clientCmdArg_.tag))) {
        cout << "invalid regex" << endl;
        return false;
    }
    if (clientCmdArg_.real) {
        auto toolListener = std::make_shared<HiSysEventToolListener>(clientCmdArg_.checkValidEvent);
        if (toolListener == nullptr) {
            return false;
        }
        std::vector<ListenerRule> sysRules;
        ListenerRule listenerRule(clientCmdArg_.domain, clientCmdArg_.eventName,
            clientCmdArg_.tag, clientCmdArg_.ruleType, clientCmdArg_.eventType);
        sysRules.emplace_back(listenerRule);
        auto retCode = HiSysEventManager::AddListener(toolListener, sysRules);
        if (retCode != IPC_CALL_SUCCEED) {
            cout << "failed to subscribe system event: " << GetErrorDescription(retCode) << endl;
        }
        return true;
    }

    if (clientCmdArg_.history) {
        auto queryCallBack = std::make_shared<HiSysEventToolQuery>(clientCmdArg_.checkValidEvent, autoExit_);
        if (queryCallBack == nullptr) {
            return false;
        }
        struct QueryArg args(clientCmdArg_.beginTime, clientCmdArg_.endTime, clientCmdArg_.maxEvents);
        std::vector<QueryRule> queryRules;
        if (clientCmdArg_.ruleType != RuleType::WHOLE_WORD) {
            cout << "only \"-c WHOLE_WORD\" supported with \"hisysevent -l\" cmd." << endl;
            return false;
        }
        if (!clientCmdArg_.domain.empty() || !clientCmdArg_.eventName.empty() ||
            clientCmdArg_.eventType != INVALID_EVENT_TYPE) {
            QueryRule rule(clientCmdArg_.domain, { clientCmdArg_.eventName },
                clientCmdArg_.ruleType, clientCmdArg_.eventType);
            queryRules.push_back(rule);
        }
        auto retCode = HiSysEventManager::Query(args, queryRules, queryCallBack);
        if (retCode != IPC_CALL_SUCCEED) {
            cout << "failed to query system event: " << GetErrorDescription(retCode) << endl;
        }
        return true;
    }
    return false;
}

void HiSysEventTool::WaitClient()
{
    unique_lock<mutex> lock(mutexClient_);
    condvClient_.wait(lock);
}

void HiSysEventTool::NotifyClient()
{
    condvClient_.notify_one();
}
} // namespace HiviewDFX
} // namespace OHOS
