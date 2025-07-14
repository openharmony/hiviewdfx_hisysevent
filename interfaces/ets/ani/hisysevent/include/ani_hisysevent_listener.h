/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef ANI_HISYSEVENT_LISTENER_H
#define ANI_HISYSEVENT_LISTENER_H

#include <ani.h>
#include <string>
#include <sys/syscall.h>
#include <unistd.h>
#include "ani_callback_context.h"
#include "hisysevent_base_listener.h"
#include "ani_callback_manager.h"

namespace OHOS {
namespace HiviewDFX {
class AniHiSysEventListener : public HiSysEventBaseListener {
public:
    AniHiSysEventListener(CallbackContextAni *context);
    virtual ~AniHiSysEventListener();

public:
    virtual void OnEvent(const std::string& domain, const std::string& eventName, const int eventType,
        const std::string& eventDetail) override;
    virtual void OnServiceDied() override;

private:
    CallbackContextAni *callbackContextAni;
    std::shared_ptr<AniCallbackManager> aniCallbackManager;

friend class HiSysEventManager;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // ANI_HISYSEVENT_LISTENER_H
