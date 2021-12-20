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
#include "hisysevent_delegate.h"

using namespace std;
using namespace OHOS::HiviewDFX;

int main(int argc, char* argv[])
{
    auto client = make_unique<HiSysEventTool>();
    if (!client->ParseCmdLine(argc, argv)) {
        client->DoCmdHelp();
        return -1;
    }
    if (!client->DoAction()) {
        client->DoCmdHelp();
        return -1;
    }
    thread binderThread(HiSysEventDelegate::BinderFunc);
    binderThread.detach();

    client->WaitClient();
    return 0;
}