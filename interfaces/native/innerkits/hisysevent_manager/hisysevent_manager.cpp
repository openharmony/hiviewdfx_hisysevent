/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "hisysevent_base_manager.h"
#include "hisysevent_delegate.h"
#include "ret_code.h"

namespace OHOS {
namespace HiviewDFX {
int32_t HiSysEventManager::AddListener(std::shared_ptr<HiSysEventListener> listener,
    std::vector<ListenerRule>& rules)
{
    auto baseListener = std::make_shared<HiSysEventBaseListener>(listener);
    return HiSysEventBaseManager::AddListener(baseListener, rules);
}

int32_t HiSysEventManager::RemoveListener(std::shared_ptr<HiSysEventListener> listener)
{
    auto baseListener = std::make_shared<HiSysEventBaseListener>(listener);
    return HiSysEventBaseManager::RemoveListener(baseListener);
}

int32_t HiSysEventManager::Query(struct QueryArg& arg, std::vector<QueryRule>& rules,
    std::shared_ptr<HiSysEventQueryCallback> callback)
{
    auto baseQueryCallback = std::make_shared<HiSysEventBaseQueryCallback>(callback);
    return HiSysEventBaseManager::Query(arg, rules, baseQueryCallback);
}

int32_t HiSysEventManager::SetDebugMode(std::shared_ptr<HiSysEventListener> listener, bool mode)
{
    auto baseListener = std::make_shared<HiSysEventBaseListener>(listener);
    return HiSysEventBaseManager::SetDebugMode(baseListener, mode);
}
} // namespace HiviewDFX
} // namespace OHOS
