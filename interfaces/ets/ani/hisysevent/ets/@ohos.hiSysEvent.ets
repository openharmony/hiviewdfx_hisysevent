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

import { AsyncCallback, BusinessError } from '@ohos.base';

class SysEventInfoAni implements hiSysEvent.SysEventInfo {
    domain: string = "";
    name: string = "";
    eventType: hiSysEvent.EventType = hiSysEvent.EventType.FAULT;
    params?:
        undefined | Record<string, number | string | boolean | bigint | number[] | string[] | boolean[] | bigint[]>;
}

class HiSysEventAni {
    native static write(info: hiSysEvent.SysEventInfo): Result;
}

interface Result {
    code: number;
    message: string;
}

class ResultInner implements Result {
    code: number = 0;
    message: string = "";
}

export default namespace hiSysEvent {
    loadLibrary("hisysevent_ani");
    export enum EventType {
        FAULT = 1,
        STATISTIC = 2,
        SECURITY = 3,
        BEHAVIOR = 4,
    }

    export interface SysEventInfo {
        domain: string;
        name: string;
        eventType: EventType;
        params?: Record<string, number | string | boolean | bigint | number[] | string[] | boolean[] | bigint[]>;
     }

    export function write(info: SysEventInfo, callback: AsyncCallback<void>): void {
        let cb = (): Result => { return HiSysEventAni.write(info) }
        taskpool.execute(cb).then((ret: NullishType): void => {
            let retError = new BusinessError<void>();
            let writeResult: Result = ret as Result;
            retError.code = writeResult.code as number;
            retError.message = writeResult.message as string;
            callback(retError, undefined);
        });
    }

    export function write(info: SysEventInfo): Promise<void> {
        return new Promise<void>((resolve: (v: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
            let cb = (): Result => { return HiSysEventAni.write(info) }
            taskpool.execute(cb).then((ret: NullishType): void => {
                let retError = new BusinessError<void>();
                let writeResult: Result = ret as Result;
                if (writeResult.code === 0) {
                    resolve(undefined);
                } else {
                    retError.code = writeResult.code as number;
                    retError.message = writeResult.message as string;
                    reject(retError)
                }
            });
        });
    }
}
