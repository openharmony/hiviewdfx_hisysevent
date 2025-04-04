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

#ifndef HISYSEVENT_MANAGER_H
#define HISYSEVENT_MANAGER_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "hisysevent_base_listener.h"
#include "hisysevent_listener.h"
#include "hisysevent_query_callback.h"
#include "hisysevent_rules.h"

namespace OHOS {
namespace HiviewDFX {
class HiSysEventManager {
public:
    HiSysEventManager() = default;
    ~HiSysEventManager() {}

public:
    /**
     * @brief Add a watcher on event writing.
     * @param listener  event watcher.
     * @param rules    rules for watcher.
     * @return 0 means success, others means failure.
     */
    static int32_t AddListener(std::shared_ptr<HiSysEventListener> listener,
        std::vector<ListenerRule>& rules);

    /**
     * @brief Remove a watcher.
     * @param listener event watcher.
     * @return 0 means success, others means failure.
     */
    static int32_t RemoveListener(std::shared_ptr<HiSysEventListener> listener);

    /**
     * @brief Query event.
     * @param arg      arg of query.
     * @param rules    rules of query.
     * @param callback callback of query.
     * @return 0 means success, others means failure.
     */
    static int32_t Query(struct QueryArg& arg, std::vector<QueryRule>& rules,
        std::shared_ptr<HiSysEventQueryCallback> callback);

private:
    static std::unordered_map<std::shared_ptr<HiSysEventListener>,
        std::shared_ptr<HiSysEventBaseListener>> listenerToBaseMap_;
    static std::mutex listenersMutex_;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_MANAGER_H

