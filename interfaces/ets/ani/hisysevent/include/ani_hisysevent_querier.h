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

#ifndef ANI_HISYSEVENT_QUERIER_H
#define ANI_HISYSEVENT_QUERIER_H

#include <functional>
#include <string>
#include <vector>
#include <sys/syscall.h>
#include <unistd.h>

#include "hisysevent_base_query_callback.h"
#include "ani_callback_manager.h"
#include "ani_callback_context.h"

namespace OHOS {
namespace HiviewDFX {
using ON_COMPLETE_FUNC = std::function<void(ani_vm*, ani_ref)>;
class AniHiSysEventQuerier : public HiSysEventBaseQueryCallback {
public:
    AniHiSysEventQuerier(CallbackContextAni *context, ON_COMPLETE_FUNC handler);
    virtual ~AniHiSysEventQuerier();

public:
    virtual void OnQuery(const ::std::vector<std::string>& sysEvents,
        const std::vector<int64_t>& seq) override;
    virtual void OnComplete(int32_t reason, int32_t total, int64_t seq) override;

private:
    CallbackContextAni *callbackContextAni;
    ON_COMPLETE_FUNC onCompleteHandler;
    std::shared_ptr<AniCallbackManager> aniCallbackManager;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // ANI_HISYSEVENT_QUERIER_H
