/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef HISYSEVENT_LISTENER_C_H
#define HISYSEVENT_LISTENER_C_H

#include <string>

#include "hisysevent_listener.h"
#include "hisysevent_record_c.h"

using OnEventFunc = void (*) (HiSysEventRecord);
using OnServiceDiedFunc = void (*) ();

class HiSysEventListenerC : public OHOS::HiviewDFX::HiSysEventListener {
public:
    HiSysEventListenerC(OnEventFunc onEvent, OnServiceDiedFunc onServiceDied)
    {
        onEvent_ = onEvent;
        onServiceDied_ = onServiceDied;
    }

    virtual ~HiSysEventListenerC() {}

public:
   void OnEvent(std::shared_ptr<OHOS::HiviewDFX::HiSysEventRecord> sysEvent) override;
   void OnServiceDied() override;

private:
    OnEventFunc onEvent_;
    OnServiceDiedFunc onServiceDied_;
};

#endif // HISYSEVENT_LISTENER_C_H
