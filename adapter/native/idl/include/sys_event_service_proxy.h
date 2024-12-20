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

#ifndef OHOS_HIVIEWDFX_SYS_EVENT_SERVICE_PROXY_H
#define OHOS_HIVIEWDFX_SYS_EVENT_SERVICE_PROXY_H

#include <string>

#include "iremote_proxy.h"
#include "isys_event_service.h"
#include "query_argument.h"

namespace OHOS {
namespace HiviewDFX {
class SysEventServiceProxy : public IRemoteProxy<ISysEventService> {
public:
    explicit SysEventServiceProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISysEventService>(impl) {}
    virtual ~SysEventServiceProxy() = default;
    DISALLOW_COPY_AND_MOVE(SysEventServiceProxy);

    int32_t AddListener(const std::vector<SysEventRule>& rules, const sptr<ISysEventCallback>& callback);
    int32_t RemoveListener(const sptr<ISysEventCallback>& callback);
    int32_t Query(const QueryArgument& queryArgument, const std::vector<SysEventQueryRule>& rules,
        const sptr<IQuerySysEventCallback>& callback);
    int32_t SetDebugMode(const sptr<ISysEventCallback>& callback, bool mode);
    int64_t AddSubscriber(const std::vector<SysEventQueryRule> &rules);
    int32_t RemoveSubscriber();
    int64_t Export(const QueryArgument &queryArgument, const std::vector<SysEventQueryRule> &rules);

private:
    static inline BrokerDelegator<SysEventServiceProxy> delegator_;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_SYS_EVENT_SERVICE_PROXY_H
