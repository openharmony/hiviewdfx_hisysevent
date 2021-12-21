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

#ifndef HISYSEVENT_SUBSCRIBE_CALLBACK_BASE_H
#define HISYSEVENT_SUBSCRIBE_CALLBACK_BASE_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class HiSysEventSubscribeCallBackBase {
public:
    explicit HiSysEventSubscribeCallBackBase(void* subHandle) : handle(subHandle) {}
    virtual void OnHandle(const std::string& domain, const std::string& eventName, const int eventType,
        const std::string& eventDetail);
    virtual bool Compare(const std::shared_ptr<OHOS::HiviewDFX::HiSysEventSubscribeCallBackBase> cbHandle,
        const void* env) const;
    virtual const void* GetHandle();
    virtual void OnServiceDied();
    virtual ~HiSysEventSubscribeCallBackBase() {}

protected:
    void *handle;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_SUBSCRIBE_CALLBACK_BASE_H