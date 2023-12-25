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

#include "hilog/log.h"
#include "hisysevent_base_manager.h"
#include "ret_code.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_MANAGER"

namespace OHOS {
namespace HiviewDFX {
std::unordered_map<std::shared_ptr<HiSysEventListener>,
        std::shared_ptr<HiSysEventBaseListener>> HiSysEventManager::listenerToBaseMap_;
std::mutex HiSysEventManager::listenersMutex_;

int32_t HiSysEventManager::AddListener(std::shared_ptr<HiSysEventListener> listener,
    std::vector<ListenerRule>& rules)
{
    if (listener == nullptr) {
        HILOG_WARN(LOG_CORE, "add a null listener is not allowed.");
        return ERR_LISTENER_NOT_EXIST;
    }
    std::lock_guard<std::mutex> lock(listenersMutex_);
    auto baseListener = listenerToBaseMap_[listener];
    if (baseListener == nullptr) {
        baseListener = std::make_shared<HiSysEventBaseListener>(listener);
        listenerToBaseMap_[listener] = baseListener;
    }
    return HiSysEventBaseManager::AddListener(baseListener, rules);
}

int32_t HiSysEventManager::RemoveListener(std::shared_ptr<HiSysEventListener> listener)
{
    if (listener == nullptr) {
        HILOG_WARN(LOG_CORE, "remov a null listener is not allowed.");
        return ERR_LISTENER_NOT_EXIST;
    }
    std::lock_guard<std::mutex> lock(listenersMutex_);
    auto baseListener = listenerToBaseMap_[listener];
    if (baseListener == nullptr) {
        HILOG_WARN(LOG_CORE, "no need to remove a listener which has not been added.");
        return ERR_LISTENER_NOT_EXIST;
    }
    auto ret = HiSysEventBaseManager::RemoveListener(baseListener);
    if (ret == IPC_CALL_SUCCEED) {
        HILOG_DEBUG(LOG_CORE, "remove listener from local cache.");
        listenerToBaseMap_.erase(listener);
    }
    return ret;
}

int32_t HiSysEventManager::Query(struct QueryArg& arg, std::vector<QueryRule>& rules,
    std::shared_ptr<HiSysEventQueryCallback> callback)
{
    auto baseQueryCallback = std::make_shared<HiSysEventBaseQueryCallback>(callback);
    return HiSysEventBaseManager::Query(arg, rules, baseQueryCallback);
}

int32_t HiSysEventManager::SetDebugMode(std::shared_ptr<HiSysEventListener> listener, bool mode)
{
    if (listener == nullptr) {
        HILOG_WARN(LOG_CORE, "set debug mode on a null listener is not allowed.");
        return ERR_LISTENER_NOT_EXIST;
    }
    std::lock_guard<std::mutex> lock(listenersMutex_);
    auto baseListener = listenerToBaseMap_[listener];
    if (baseListener == nullptr) {
        HILOG_WARN(LOG_CORE, "no need to set debug mode on a listener which has not been added.");
        return ERR_LISTENER_NOT_EXIST;
    }
    return HiSysEventBaseManager::SetDebugMode(baseListener, mode);
}
} // namespace HiviewDFX
} // namespace OHOS
