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

#ifndef HISYSEVENT_CLIENT_QUERY_H
#define HISYSEVENT_CLIENT_QUERY_H

#include <string>
#include <vector>

#include "hisysevent_query_callback.h"

namespace OHOS {
namespace HiviewDFX {
class HiSysEventToolQuery : public OHOS::HiviewDFX::HiSysEventQueryCallBack {
public:
    void OnQuery(const ::std::vector<std::string>& sysEvent,
        const ::std::vector<int64_t>& seq);
    void OnComplete(int32_t reason, int32_t total);
    virtual ~HiSysEventToolQuery() {}
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_CLIENT_QUERY_H
