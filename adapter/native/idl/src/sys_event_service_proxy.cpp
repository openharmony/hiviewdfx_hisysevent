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

#include "sys_event_service_proxy.h"

#include "errors.h"
#include "hilog/log.h"
#include "parcelable_vector_rw.h"
#include "query_argument.h"
#include "ret_code.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_SERVICE_PROXY"

namespace OHOS {
namespace HiviewDFX {
int32_t SysEventServiceProxy::AddListener(const std::vector<SysEventRule>& rules,
    const sptr<ISysEventCallback>& callback)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR(LOG_CORE, "SysEventService Remote is NULL.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HILOG_ERROR(LOG_CORE, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    bool ret = WriteVectorToParcel(data, rules);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel write rules failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    if (callback == nullptr) {
        return ERR_PARCEL_DATA_IS_NULL;
    }
    ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel write callback failed.");
        return ERR_CAN_NOT_WRITE_REMOTE_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(ADD_SYS_EVENT_LISTENER, data, reply, option);
    if (res != ERR_OK) {
        HILOG_ERROR(LOG_CORE, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int32_t result;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}

int32_t SysEventServiceProxy::RemoveListener(const sptr<ISysEventCallback> &callback)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR(LOG_CORE, "SysEventService Remote is null.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HILOG_ERROR(LOG_CORE, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    if (callback == nullptr) {
        return ERR_PARCEL_DATA_IS_NULL;
    }
    bool ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel write object in callback failed.");
        return ERR_CAN_NOT_WRITE_REMOTE_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(REMOVE_SYS_EVENT_LISTENER, data, reply, option);
    if (res != ERR_OK) {
        HILOG_ERROR(LOG_CORE, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int32_t result;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}

int32_t SysEventServiceProxy::Query(const QueryArgument& queryArgument, const std::vector<SysEventQueryRule>& rules,
    const sptr<IQuerySysEventCallback>& callback)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR(LOG_CORE, "SysEventService Remote is null.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HILOG_ERROR(LOG_CORE, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    if (!data.WriteParcelable(&queryArgument)) {
        HILOG_ERROR(LOG_CORE, "parcel write query arguments failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    bool ret = WriteVectorToParcel(data, rules);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel write query rules failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    if (callback == nullptr) {
        return ERR_PARCEL_DATA_IS_NULL;
    }
    ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel write callback failed.");
        return ERR_CAN_NOT_WRITE_REMOTE_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(QUERY_SYS_EVENT, data, reply, option);
    if (res != ERR_OK) {
        HILOG_ERROR(LOG_CORE, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int32_t result;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}

int32_t SysEventServiceProxy::SetDebugMode(const sptr<ISysEventCallback>& callback, bool mode)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR(LOG_CORE, "SysEventService Remote is null.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HILOG_ERROR(LOG_CORE, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    if (callback == nullptr) {
        return ERR_PARCEL_DATA_IS_NULL;
    }
    bool ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel write callback failed.");
        return ERR_CAN_NOT_WRITE_REMOTE_OBJECT;
    }
    ret = data.WriteBool(mode);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel write mode failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(SET_DEBUG_MODE, data, reply, option);
    if (res != ERR_OK) {
        HILOG_ERROR(LOG_CORE, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int32_t result;
    ret = reply.ReadInt32(result);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}

int64_t SysEventServiceProxy::AddSubscriber(const std::vector<SysEventQueryRule> &rules)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR(LOG_CORE, "SysEventService Remote is NULL.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HILOG_ERROR(LOG_CORE, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    bool ret = WriteVectorToParcel(data, rules);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel write rules failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(ADD_SYS_EVENT_SUBSCRIBER, data, reply, option);
    if (res != ERR_OK) {
        HILOG_ERROR(LOG_CORE, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int64_t result;
    ret = reply.ReadInt64(result);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}

int32_t SysEventServiceProxy::RemoveSubscriber()
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR(LOG_CORE, "SysEventService Remote is NULL.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HILOG_ERROR(LOG_CORE, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(REMOVE_SYS_EVENT_SUBSCRIBER, data, reply, option);
    if (res != ERR_OK) {
        HILOG_ERROR(LOG_CORE, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int32_t result;
    auto ret = reply.ReadInt32(result);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}

int64_t SysEventServiceProxy::Export(const QueryArgument &queryArgument, const std::vector<SysEventQueryRule> &rules)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR(LOG_CORE, "SysEventService Remote is NULL.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HILOG_ERROR(LOG_CORE, "write descriptor failed.");
        return ERR_CAN_NOT_WRITE_DESCRIPTOR;
    }
    if (!data.WriteParcelable(&queryArgument)) {
        HILOG_ERROR(LOG_CORE, "parcel write export arguments failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    bool ret = WriteVectorToParcel(data, rules);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel write export rules failed.");
        return ERR_CAN_NOT_WRITE_PARCEL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(EXPORT_SYS_EVENT, data, reply, option);
    if (res != ERR_OK) {
        HILOG_ERROR(LOG_CORE, "send request failed, error is %{public}d.", res);
        return ERR_CAN_NOT_SEND_REQ;
    }
    int64_t result;
    ret = reply.ReadInt64(result);
    if (!ret) {
        HILOG_ERROR(LOG_CORE, "parcel read result failed.");
        return ERR_CAN_NOT_READ_PARCEL;
    }
    return result;
}

} // namespace HiviewDFX
} // namespace OHOS

