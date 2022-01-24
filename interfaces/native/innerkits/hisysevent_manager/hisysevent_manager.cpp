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

#include "hisysevent_manager.h"

#include "hisysevent_delegate.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
int HiSysEventManager::AddEventListener(std::shared_ptr<HiSysEventSubscribeCallBackBase> listener,
    std::vector<ListenerRule>& rules)
{
    return HiSysEventDelegate::GetInstance().AddEventListener(listener, rules, nullptr);
}

void HiSysEventManager::RemoveListener(std::shared_ptr<HiSysEventSubscribeCallBackBase> listener)
{
    HiSysEventDelegate::GetInstance().RemoveListener(listener, nullptr);
}

bool HiSysEventManager::QueryHiSysEvent(struct QueryArg& queryArg,
    std::vector<QueryRule>& queryRules,
    std::shared_ptr<HiSysEventQueryCallBackBase> queryCallBack)
{
    return HiSysEventDelegate::GetInstance().QueryHiSysEvent(queryArg, queryRules, queryCallBack);
}

bool HiSysEventManager::SetDebugMode(std::shared_ptr<HiSysEventSubscribeCallBackBase> listener, bool mode)
{
    return HiSysEventDelegate::GetInstance().SetDebugMode(listener, mode, nullptr);
}
} // namespace HiviewDFX
} // namespace OHOS