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

#ifndef HISYSEVENT_DELEGATE_H
#define HISYSEVENT_DELEGATE_H

#include <mutex>
#include <string>
#include <thread>

#include "hisysevent_listener_proxy.h"
#include "hisysevent_manager.h"
#include "hisysevent_query_callback_base.h"
#include "sys_event_query_rule.h"
#include "singleton.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "sys_event_rule.h"

namespace OHOS {
namespace HiviewDFX {
class HiSysEventDelegate : public Singleton<HiSysEventDelegate> {
public:
    HiSysEventDelegate() {}
    int AddEventListener(const std::shared_ptr<HiSysEventSubscribeCallBackBase> listener,
        const std::vector<ListenerRule>& rules, const void* compFactor);

    void RemoveListener(const std::shared_ptr<HiSysEventSubscribeCallBackBase> listener, void* compFactor);

    bool QueryHiSysEvent(const struct QueryArg& queryArg,
        const std::vector<QueryRule>& queryRules,
        const std::shared_ptr<HiSysEventQueryCallBackBase> queryCallBack) const;

    bool SetDebugMode(const std::shared_ptr<HiSysEventSubscribeCallBackBase> listener,
        const bool mode, void* compFactor);

    ~HiSysEventDelegate() {}
    static void BinderFunc();

private:
    void ConvertListenerRule(const std::vector<ListenerRule>& rules,
        std::vector<SysEventRule>& sysRules) const;
    void ConvertQueryRule(const std::vector<QueryRule>& rules,
        std::vector<SysEventQueryRule>& sysRules) const;
    std::vector<sptr<HiSysEventListenerProxy>> listenerList;
    sptr<IRemoteObject> GetSysEventService() const;
    std::mutex mutexLock;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_DELEGATE_H
