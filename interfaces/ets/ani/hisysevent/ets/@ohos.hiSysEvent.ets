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
    params?: object;
}

class WatcherAni implements hiSysEvent.Watcher {
    rules: hiSysEvent.WatchRule[] = [];
    onEvent: (info: hiSysEvent.SysEventInfo) => void = (info: hiSysEvent.SysEventInfo) => {};
    onServiceDied: () => void = () => {};
}

class QuerierAni implements hiSysEvent.Querier {
    onQuery: (infos: hiSysEvent.SysEventInfo[]) => void = (infos: hiSysEvent.SysEventInfo[]) => {};
    onComplete: (reason: number, total: number) => void = (reason: number, total: number) => {};
}

class QueryArgAni implements hiSysEvent.QueryArg {
    beginTime: number = 0;
    endTime: number = 0;
    maxEvents: number = 0;
    fromSeq?: number | undefined;
    toSeq?: number | undefined;
}

class QueryRuleAni implements hiSysEvent.QueryRule {
    domain: string = "";
    names: string[] = [];
    condition?: string | undefined;
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
        params?: object;
     }

    export interface Watcher {
        rules: WatchRule[];
        onEvent: (info: SysEventInfo) => void;
        onServiceDied: () => void;
    }

    export interface WatchRule {
        domain: string;
        name: string;
        tag?: string;
        ruleType: RuleType;
    }

    export enum RuleType {
        WHOLE_WORD = 1,
        PREFIX = 2,
        REGULAR = 3
    }

    export interface QueryRule {
      domain: string;
      names: string[];
      condition?: string;
    }

    export interface QueryArg {
        beginTime: number;
        endTime: number;
        maxEvents: number;
        fromSeq?: number;
        toSeq?: number;
    }

    export interface Querier {
        onQuery: (infos: SysEventInfo[]) => void;
        onComplete: (reason: number, total: number) => void;
    }

    native function writeSync(info: SysEventInfo): Result;
    export native function addWatcher(watcher: Watcher): void;
    export native function removeWatcher(watcher: Watcher): void;
    export native function query(queryArg: QueryArg, rules: QueryRule[], querier: Querier): void;
    export native function exportSysEvents(queryArg: QueryArg, rules: QueryRule[]): number;
    export native function subscribe(rules: QueryRule[]): number;
    export native function unsubscribe(): void;

    export function write(info: SysEventInfo, callback: AsyncCallback<void>): void {
        let cb = (): Result => { return writeSync(info) }
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
            let cb = (): Result => { return writeSync(info) }
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
