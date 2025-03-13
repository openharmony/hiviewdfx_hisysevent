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

class SysEventInfoAni implements hiSysEvent.SysEventInfo {
  domain: string = ""
  name: string = ""
  eventType: hiSysEvent.EventType = hiSysEvent.EventType.FAULT
  params?: undefined | Record<string, number | string | boolean | bigint | number[] | string[] | boolean[] | bigint[]>
}

class HiSysEventAni {
  native static write(info: hiSysEvent.SysEventInfo): int;
}

export class BusinessError<T = void> {
    code: int = 0;
    data?: T;
}

export type AsyncCallback<T, E = void> = (err: BusinessError<E>, data?: T) => void;

export default namespace hiSysEvent {
  loadLibrary("ani_hisysevent");
  export enum EventType {
      FAULT = 1,
      STATISTIC = 2,
      SECURITY = 3,
      BEHAVIOR = 4
  }

  export interface SysEventInfo {
      domain: string
      name: string
      eventType: EventType
      params?: Record<string, number | string | boolean | bigint | number[] | string[] | boolean[] | bigint[]>
  }

  export function write(info: SysEventInfo, callback: AsyncCallback<int, void>): void {
      let task = taskpool.execute(HiSysEventAni.write, info);
      task.then((ret: NullishType) => {
          let eNull = new BusinessError<void>();
          if (ret === null || ret === undefined) {
              eNull.code = -1;
              callback(eNull, undefined);
          } else {
              eNull.code = 0;
              callback(eNull, ret as int);
          }
      });
  }

  export function write(info: SysEventInfo): Promise<int> {
      return new Promise<int>((resolve: (v: int) => void, reject: (e: BusinessError<string>) => void) => {
          let task = taskpool.execute(HiSysEventAni.write, info);
          task.then((ret: NullishType) => {
              if (ret === null || ret === undefined) {
                  let retError = new BusinessError<string>();
                  retError.code = -1;
                  retError.data = "Operation failed";
                  reject(retError);
              } else {
                  resolve(ret as int);
              }
          });
      });
  }
}
