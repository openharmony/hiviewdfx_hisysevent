/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "transport.h"

#include <cerrno>
#include <cstddef>
#include <iosfwd>
#include <list>
#include <mutex>
#include <securec.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <unistd.h>

#include "def.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "HISYSEVENT_TRANSPORT"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t BUF_SIZE = 2000;
char g_errMsg[BUF_SIZE] = { 0 };
struct sockaddr_un g_serverAddr = {
    .sun_family = AF_UNIX,
    .sun_path = "/dev/unix/socket/hisysevent",
};
}
Transport Transport::instance_;

Transport& Transport::GetInstance()
{
    return instance_;
}

void Transport::InitRecvBuffer(int socketId)
{
    int oldN = 0;
    socklen_t oldOutSize = static_cast<socklen_t>(sizeof(int));
    if (getsockopt(socketId, SOL_SOCKET, SO_SNDBUF, static_cast<void *>(&oldN), &oldOutSize) < 0) {
        strerror_r(errno, g_errMsg, BUF_SIZE);
        HILOG_DEBUG(LOG_CORE, "get socket send buffer error=%{public}d, msg=%{public}s", errno, g_errMsg);
    }

    int sendBuffSize = MAX_DATA_SIZE;
    if (setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, static_cast<void *>(&sendBuffSize), sizeof(int)) < 0) {
        strerror_r(errno, g_errMsg, BUF_SIZE);
        HILOG_DEBUG(LOG_CORE, "set socket send buffer error=%{public}d, msg=%{public}s", errno, g_errMsg);
    }

    int newN = 0;
    socklen_t newOutSize = static_cast<socklen_t>(sizeof(int));
    if (getsockopt(socketId, SOL_SOCKET, SO_SNDBUF, static_cast<void *>(&newN), &newOutSize) < 0) {
        strerror_r(errno, g_errMsg, BUF_SIZE);
        HILOG_DEBUG(LOG_CORE, "get new socket send buffer error=%{public}d, msg=%{public}s", errno, g_errMsg);
    }
}

int Transport::SendToHiSysEventDataSource(RawData& rawData)
{
    // reopen the socket with an new id each time is neccessary here, which is more efficient than that
    // reuse id of the opened socket and then use a mutex to avoid multi-threading race.
    auto socketId = TEMP_FAILURE_RETRY(socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
    if (socketId < 0) {
        strerror_r(errno, g_errMsg, BUF_SIZE);
        HILOG_DEBUG(LOG_CORE, "create hisysevent client socket failed, error=%{public}d, msg=%{public}s",
            errno, g_errMsg);
        return ERR_DOES_NOT_INIT;
    }
    InitRecvBuffer(socketId);
    auto sendRet = 0;
    auto retryTimes = RETRY_TIMES;
    do {
        sendRet = sendto(socketId, rawData.GetData(), rawData.GetDataLength(), 0,
            reinterpret_cast<sockaddr*>(&g_serverAddr), sizeof(g_serverAddr));
        retryTimes--;
    } while (sendRet < 0 && retryTimes > 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR));
    if (sendRet < 0) {
        strerror_r(errno, g_errMsg, BUF_SIZE);
        if (errno == EACCES) {
            HILOG_DEBUG(LOG_CORE, "sysevent write failed, err=%{public}d", errno);
        } else {
            HILOG_ERROR(LOG_CORE, "sysevent write failed, err=%{public}d", errno);
        }
        close(socketId);
        return ERR_SEND_FAIL;
    }
    close(socketId);
    HILOG_DEBUG(LOG_CORE, "hisysevent send data successful");
    return SUCCESS;
}

void Transport::AddFailedData(RawData& rawData)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (retryDataList_.size() >= RETRY_QUEUE_SIZE) {
        retryDataList_.pop_front();
    }
    retryDataList_.push_back(rawData);
}

void Transport::RetrySendFailedData()
{
    if (retryDataList_.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    while (!retryDataList_.empty()) {
        auto rawData = retryDataList_.front();
        if (SendToHiSysEventDataSource(rawData) != SUCCESS) {
            return;
        }
        retryDataList_.pop_front();
    }
}

int Transport::SendData(RawData& rawData)
{
    if (rawData.IsEmpty()) {
        HILOG_WARN(LOG_CORE, "try to send a empty data.");
        return ERR_EMPTY_EVENT;
    }
    auto rawDataLength = rawData.GetDataLength();
    if (rawDataLength > MAX_DATA_SIZE) {
        return ERR_OVER_SIZE;
    }

    RetrySendFailedData();
    int tryTimes = RETRY_TIMES;
    int retCode = SUCCESS;
    while (tryTimes > 0) {
        tryTimes--;
        retCode = SendToHiSysEventDataSource(rawData);
        if (retCode == SUCCESS) {
            return retCode;
        }
    }

    AddFailedData(rawData);
    return retCode;
}
} // namespace HiviewDFX
} // namespace OHOS

