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
#include "ret_code.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
int32_t HiSysEventManager::AddEventListener(std::shared_ptr<HiSysEventSubscribeCallBack> listener,
    std::vector<ListenerRule>& rules)
{
    if (listener == nullptr) {
        return ERROR_LISTENER_NOT_EXIST;
    }
    if (listener->listenerProxy == nullptr) {
        listener->listenerProxy = new HiSysEventDelegate();
    }
    return listener->listenerProxy->AddEventListener(listener, rules);
}

int32_t HiSysEventManager::RemoveListener(std::shared_ptr<HiSysEventSubscribeCallBack> listener)
{
    if (listener == nullptr || listener->listenerProxy == nullptr) {
        return ERROR_LISTENER_NOT_EXIST;
    }
    auto listenerRemoveResult = listener->listenerProxy->RemoveListener(listener);
    delete listener->listenerProxy;
    listener->listenerProxy = nullptr;
    return listenerRemoveResult;
}

int32_t HiSysEventManager::QueryHiSysEvent(struct QueryArg& queryArg,
    std::vector<QueryRule>& queryRules,
    std::shared_ptr<HiSysEventQueryCallBack> queryCallBack)
{
    auto proxy = std::make_unique<HiSysEventDelegate>();
    if (proxy != nullptr) {
        return proxy->QueryHiSysEvent(queryArg, queryRules, queryCallBack);
    }
    return ERROR_LISTENER_NOT_EXIST;
}

int32_t HiSysEventManager::SetDebugMode(std::shared_ptr<HiSysEventSubscribeCallBack> listener, bool mode)
{
    if (listener == nullptr || listener->listenerProxy == nullptr) {
        return ERROR_LISTENER_NOT_EXIST;
    }
    return listener->listenerProxy->SetDebugMode(listener, mode);
}
} // namespace HiviewDFX
} // namespace OHOS
