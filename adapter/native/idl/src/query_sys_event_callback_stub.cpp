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

#include "query_sys_event_callback_stub.h"

#include <cstdint>
#include <string>
#include <vector>

#include "ash_mem_utils.h"
#include "errors.h"
#include "hilog/log.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_CALLBACK_STUB"

namespace OHOS {
namespace HiviewDFX {
int32_t QuerySysEventCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    std::u16string descripter = QuerySysEventCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        HILOG_ERROR(LOG_CORE, "read descriptor failed.");
        return ERR_INVALID_VALUE;
    }
    bool ret = false;
    switch (code) {
        case ON_QUERY: {
            std::vector<std::u16string> sysEvent;
            ret = AshMemUtils::ReadBulkData(data, sysEvent);
            if (!ret) {
                HILOG_ERROR(LOG_CORE, "parcel read sys event failed.");
                return ERR_FLATTEN_OBJECT;
            }
            std::vector<int64_t> seq;
            ret = data.ReadInt64Vector(&seq);
            if (!ret) {
                HILOG_ERROR(LOG_CORE, "parcel read seq failed.");
                return ERR_FLATTEN_OBJECT;
            }
            OnQuery(sysEvent, seq);
            return ERR_OK;
        }
        case ON_COMPLETE: {
            int32_t reason = 0;
            ret = data.ReadInt32(reason);
            if (!ret) {
                HILOG_ERROR(LOG_CORE, "parcel read reason failed.");
                return ERR_FLATTEN_OBJECT;
            }
            int32_t total = 0;
            ret = data.ReadInt32(total);
            if (!ret) {
                HILOG_ERROR(LOG_CORE, "parcel read total failed.");
                return ERR_FLATTEN_OBJECT;
            }
            int64_t seq = 0;
            ret = data.ReadInt64(seq);
            if (!ret) {
                HILOG_ERROR(LOG_CORE, "parcel read seq failed.");
                return ERR_FLATTEN_OBJECT;
            }
            OnComplete(reason, total, seq);
            return ERR_OK;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}
} // namespace HiviewDFX
} // namespace OHOS
