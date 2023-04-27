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

#ifndef HISYSEVENT_RUST_WRAPPER_QUERIER_C_H
#define HISYSEVENT_RUST_WRAPPER_QUERIER_C_H

#include <mutex>

#include "hisysevent_c_wrapper.h"
#include "hisysevent_query_callback.h"
#include "hisysevent_record_c.h"

struct HiSysEventRustQuerier : public OHOS::HiviewDFX::HiSysEventQueryCallback {
public:
    HiSysEventRustQuerier(HiSysEventRustQuerierC* querier);
    ~HiSysEventRustQuerier();

public:
    virtual void OnQuery(std::shared_ptr<std::vector<OHOS::HiviewDFX::HiSysEventRecord>> sysEvents) override;
    virtual void OnComplete(int32_t reason, int32_t total) override;

public:
    void RecycleQuerier(HiSysEventRustQuerierC* querier);

private:
    HiSysEventRustQuerierC* querier_;
    std::mutex querierMutex_;
};

#endif // HISYSEVENT_RUST_WRAPPER_QUERIER_C_H