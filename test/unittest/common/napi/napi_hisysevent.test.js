/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

import hiSysEvent from "@ohos.hiSysEvent"

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('hiSysEventJsUnitTest', function () {
  beforeAll(function() {

    /**
     * @tc.setup: setup invoked before all test cases
     */
    console.info('hiSysEventJsUnitTest beforeAll called')
  })

  afterAll(function() {

    /**
     * @tc.teardown: teardown invoked after all test cases
     */
    console.info('hiSysEventJsUnitTest afterAll called')
  })

  beforeEach(function() {

    /**
     * @tc.setup: setup invoked before each test case
     */
    console.info('hiSysEventJsUnitTest beforeEach called')
  })

  afterEach(function() {

    /**
     * @tc.teardown: teardown invoked after each test case
     */
    console.info('hiSysEventJsUnitTest afterEach called')
  })

  function GetParam(src, key) {
    return src["params"][key]
  }

  function GetArrayIemParamByIndex(src, key, index) {
    let arrayInSrc = src["params"][key]
    if (index > arrayInSrc.length) {
      return undefined
    }
    return arrayInSrc[index]
  }

  function writeEventWithAsyncWork(domain, name, eventType, params, errCallback, normalCallback, done) {
    try {
      hiSysEvent.write({
        domain: domain,
        name: name,
        eventType: eventType,
        params: params
      }, (err, val) => {
        if (err) {
          errCallback(err)
        } else {
          normalCallback(val)
        }
        done()
      })
    } catch (err) {
      expect(false).assertTrue()
      done()
    }
  }

  function writeEventWithPromise(domain, name, eventType, params, errCallback, normalCallback, done) {
    try {
      hiSysEvent.write({
        domain: domain,
        name: name,
        eventType: eventType,
        params: params
      }).then((val) => {
        normalCallback(val)
        done()
      }).catch((err) => {
        errCallback(err)
        done()
      })
    } catch (err) {
      expect(false).assertTrue()
      done()
    }
  }

  function writeCustomizedSysEvent(customized) {
    try {
      hiSysEvent.write({
        domain: "RELIABILITY",
        name: "STACK",
        eventType: hiSysEvent.EventType.FAULT,
        params: customized
      }, (err, val) => {
        if (err) {
          console.error(`callback: err.code = ${err.code}, error msg is ${err.message}`)
          expect(false).assertTrue()
          done()
        }
      })
    } catch (err) {
      expect(false).assertTrue()
      done()
    }
  }

  function writeDefaultSysEvent() {
    writeCustomizedSysEvent({
      PID: 1,
      UID: 1,
      PACKAGE_NAME: "com.ohos.testHiSysEvent",
      PROCESS_NAME: "hiview js test suite",
      MSG: "no msg."
    })
  }

  function querySysEvent(queryArgs, querRules, onQueryCallback, onCompleteCallback,
    errCallback, done) {
    try {
      hiSysEvent.query(queryArgs, querRules, {
        onQuery: function (infos) {
          onQueryCallback(infos)
        },
        onComplete: function(reason, total) {
          onCompleteCallback(reason, total)
          done()
        }
      })
    } catch (err) {
      errCallback(err)
      done()
    }
  }

  /**
   * @tc.desc: Test hisysevent writing with calling AsyncCallback.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest001
   * @tc.number: hiSysEventJsUnitTest001
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest001', 0, async function (done) {
    writeEventWithAsyncWork("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      {
        PID: 1,
        UID: 1,
        PACKAGE_NAME: "com.ohos.testHiSysEvent",
        PROCESS_NAME: "hiview js test suite",
        MSG: "no msg."
      },
      (err) => {
        expect(false).assertTrue()
      },
      (val) => {
        expect(val).assertEqual(0)
      }, done)
  })

  /**
   * @tc.desc: Test hisysevent writing with returning Promise.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest002
   * @tc.number: hiSysEventJsUnitTest002
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest002', 0, async function (done) {
    writeEventWithPromise("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      {
        PID: 1,
        UID: 1,
        PACKAGE_NAME: "com.ohos.testHiSysEvent",
        PROCESS_NAME: "hiview js test suite",
        MSG: "no msg."
      },
      (err) => {
        expect(false).assertTrue()
      },
      (val) => {
        expect(val).assertEqual(0)
      }, done)
  })

  /**
   * @tc.desc: Test function return of adding/remove hisysevent watcher result.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest003
   * @tc.number: hiSysEventJsUnitTest003
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest003', 0, async function (done) {
    let watcher = {
      rules: [{
        domain: "RELIABILITY",
        name: "STACK",
        ruleType: hiSysEvent.RuleType.WHOLE_WORD,
      }],
      onEvent: (info) => {
      },
      onServiceDied: () => {
      }
    }
    try {
      hiSysEvent.addWatcher(watcher)
      hiSysEvent.removeWatcher(watcher)
      expect(true).assertTrue()
      done();
    } catch (err) {
      expect(err.code == 201).assertTrue()
      done()
    }
  })

  /**
   * @tc.desc: Test watcher callback.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest004
   * @tc.number: hiSysEventJsUnitTest004
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest004', 0, async function (done) {
    let watcher = {
      rules: [{
        domain: "RELIABILITY",
        name: "STACK",
        tag: "STABILITY",
        ruleType: hiSysEvent.RuleType.WHOLE_WORD,
      }],
      onEvent: (info) => {
        expect(Object.keys(info).length > 0).assertTrue()
      },
      onServiceDied: () => {
      }
    }
    try {
      hiSysEvent.addWatcher(watcher)
      writeDefaultSysEvent()
      setTimeout(() => {
        try {
          hiSysEvent.removeWatcher(watcher)
          expect(true).assertTrue()
          done()
        } catch (err) {
          expect(err.code == 201).assertTrue()
          done()
        }
      }, 1000)
    } catch (err) {
      expect(err.code == 201).assertTrue()
      done()
    }
  })

  /**
   * @tc.desc: Test query callback.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest005
   * @tc.number: hiSysEventJsUnitTest005
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest005', 0, async function (done) {
    try {
      writeDefaultSysEvent()
      setTimeout(() => {
        querySysEvent({
          beginTime: -1,
          endTime: -1,
          maxEvents: 2,
        }, [{
          domain: "RELIABILITY",
          names: ["STACK"],
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
        }, (reason, total) => {
          expect(true).assertTrue()
        }, (err) => {
          expect(err.code == 201).assertTrue()
        }, done)
      }, 1000);
    } catch (err) {
      expect(err.code == 201).assertTrue()
      done()
    }
  })

  /**
   * @tc.desc: Test query callback with domain which length is over 16.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest006
   * @tc.number: hiSysEventJsUnitTest006
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest006', 0, async function (done) {
    console.info('hiSysEventJsUnitTest006 start')
    try {
      writeDefaultSysEvent()
      setTimeout(() => {
        querySysEvent({
          beginTime: -1,
          endTime: -1,
          maxEvents: 2,
        }, [{
          domain: "RELIABILITY_RELIABILITY",
          names: ["STACK"],
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
        }, (reason, total) => {
          expect(true).assertTrue()
        }, (err) => {
          expect(err.code == 11200302 || err.code == 11200304).assertTrue()
        }, done)
      }, 1000);
    } catch (err) {
      expect(false).assertTrue()
      done()
    }
  })

  /**
   * @tc.desc: Test query callback with domain which length is over 32.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest007
   * @tc.number: hiSysEventJsUnitTest007
   * @tc.type: Function
   * @tc.size: MediumTest
   */
   it('hiSysEventJsUnitTest007', 0, async function (done) {
    try {
      writeDefaultSysEvent()
      setTimeout(() => {
        querySysEvent({
          beginTime: -1,
          endTime: -1,
          maxEvents: 2,
        }, [{
          domain: "RELIABILITY",
          names: ["STACK_STACK_STACK_STACK_STACK_STACK"],
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
        }, (reason, total) => {
          expect(true).assertTrue()
        }, (err) => {
          expect(err.code == 11200302 || err.code == 11200304).assertTrue()
        }, done)
      }, 1000);
    } catch (err) {
      expect(false).assertTrue()
      done()
    }
  })

  /**
   * @tc.desc: Test hisysevent of invalid domain writing with calling AsyncCallback.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest008
   * @tc.number: hiSysEventJsUnitTest008
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest008', 0, async function (done) {
    writeEventWithAsyncWork("RELIABILITY_RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      {
        PID: 1,
        UID: 1,
        PACKAGE_NAME: "com.ohos.testHiSysEvent",
        PROCESS_NAME: "hiview js test suite",
        MSG: "no msg."
      }, (err) => {
        expect(err.code == 11200001).assertTrue()
      }, (val) => {
        expect(false).assertTrue()
      }, done)
  })

  /**
   * @tc.desc: Test hisysevent of invalid event name writing with calling AsyncCallback.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest009
   * @tc.number: hiSysEventJsUnitTest009
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest009', 0, async function (done) {
    writeEventWithAsyncWork("RELIABILITY", "STACK_STACK_STACK_STACK_STACK_STACK", hiSysEvent.EventType.FAULT,
      {
        PID: 1,
        UID: 1,
        PACKAGE_NAME: "com.ohos.testHiSysEvent",
        PROCESS_NAME: "hiview js test suite",
        MSG: "no msg."
      }, (err) => {
        expect(err.code == 11200002).assertTrue()
      }, (val) => {
        expect(false).assertTrue()
      }, done)
  })

  /**
   * @tc.desc: Test hisysevent which is over size writing with calling AsyncCallback.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest010
   * @tc.number: hiSysEventJsUnitTest010
   * @tc.type: Function
   * @tc.size: MediumTest
   */
   it('hiSysEventJsUnitTest010', 0, async function (done) {
    let params = {
      PID: 1,
      UID: 1,
      PACKAGE_NAME: "com.ohos.testHiSysEvent",
      PROCESS_NAME: "just a testcase",
      MSG: "no msg."
    }
    for (let i = 0; i < 40; i++) {
      params[`bundle${i}`] = Array.from({length: 10 * 1024}).join("ohos")
    }
    writeEventWithAsyncWork("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      params, (err) => {
        expect(err.code == 11200004).assertTrue()
      }, (val) => {
        expect(false).assertTrue()
      }, done)
  })

  /**
   * @tc.desc: Test hisysevent of invalid param name writing with calling AsyncCallback.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest011
   * @tc.number: hiSysEventJsUnitTest011
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest011', 0, async function (done) {
    writeEventWithAsyncWork("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      {
        PID: 1,
        UID: 1,
        STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK: "com.ohos.testHiSysEvent",
        PROCESS_NAME: "hiview js test suite",
        MSG: "no msg."
      }, (err) => {
        expect(err.code == 11200051).assertTrue()
      }, (val) => {
        expect(false).assertTrue()
      }, done)
  })

  /**
   * @tc.desc: Test hisysevent with string over limit writing with calling AsyncCallback.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest012
   * @tc.number: hiSysEventJsUnitTest012
   * @tc.type: Function
   * @tc.size: MediumTest
   */
   it('hiSysEventJsUnitTest012', 0, async function (done) {
    writeEventWithAsyncWork("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      {
        PID: 1,
        UID: 1,
        PACKAGE_NAME: "com.ohos.testHiSysEvent",
        PROCESS_NAME: Array.from({length: 10 * 1024 + 10}).join("ohos"),
        MSG: "no msg."
      }, (err) => {
        expect(err.code == 11200052).assertTrue()
      }, (val) => {
        expect(false).assertTrue()
      }, done)
  })

  /**
   * @tc.desc: Test hisysevent with param count over limit writing with calling AsyncCallback.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest013
   * @tc.number: hiSysEventJsUnitTest013
   * @tc.type: Function
   * @tc.size: MediumTest
   */
   it('hiSysEventJsUnitTest013', 0, async function (done) {
    let largeParams = {}
    for (let i = 0; i < 200; i++) {
      largeParams["name" + i] = i
    }
    writeEventWithAsyncWork("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      largeParams, (err) => {
        expect(err.code == 11200053).assertTrue()
      }, (val) => {
        expect(false).assertTrue()
      }, done)
  })

  /**
   * @tc.desc: Test hisysevent with array size over limit writing with calling AsyncCallback.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest014
   * @tc.number: hiSysEventJsUnitTest014
   * @tc.type: Function
   * @tc.size: MediumTest
   */
   it('hiSysEventJsUnitTest014', 0, async function (done) {
    let msgArray = []
    for (let i = 0; i < 200; i++) {
      msgArray[i] = i
    }
    writeEventWithAsyncWork("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      {
        PID: 1,
        UID: 1,
        PACKAGE_NAME: "com.ohos.testHiSysEvent",
        PROCESS_NAME: "hiview js test suite",
        MSG: msgArray
      }, (err) => {
        expect(err.code == 11200054).assertTrue()
      }, (val) => {
        expect(false).assertTrue()
      }, done)
  })

  /**
   * @tc.desc: Test hisysevent query with sequence.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest015
   * @tc.number: hiSysEventJsUnitTest015
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest015', 0, async function (done) {
    querySysEvent({
      maxEvents: 10000,
      fromSeq: 100,
      toSeq: 1000,
    }, [{
      domain: "AAFWK",
      names: ["CONNECT_SERVICE"],
    }], (infos) => {
      expect(infos.length >= 0).assertTrue()
    }, (reason, total) => {
      expect(true).assertTrue()
    }, (err) => {
      expect(err.code == 201).assertTrue()
    }, done)
  })

  /**
   * @tc.desc: Test hisysevent get max sequence.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest016
   * @tc.number: hiSysEventJsUnitTest016
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest016', 0, async function (done) {
    querySysEvent({
      maxEvents: 0,
      fromSeq: 0,
      toSeq: 1000,
    }, [{
      domain: "AAFWK",
      names: ["CONNECT_SERVICE"],
    }], (infos) => {
      expect(infos.length >= 0).assertTrue()
    }, (reason, total) => {
      expect(true).assertTrue()
    }, (err) => {
      expect(err.code == 201).assertTrue()
    }, done)
  })

  /**
   * @tc.desc: Test writing sysevents more than 100 times in 5 seconds.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest017
   * @tc.number: hiSysEventJsUnitTest017
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest017', 0, async function (done) {
    try {
      for (let index = 0; index < 102; index++) {
        writeEventWithAsyncWork("USERIAM_PIN", "USERIAM_TEMPLATE_CHANGE",
          hiSysEvent.EventType.SECURITY,
          {
            PID: 1,
            UID: 1,
            PACKAGE_NAME: "com.ohos.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite",
          }, (err) => {
            expect(err.code == 11200003).assertTrue()
          }, (val) => {}, done)
      }
    } catch (err) {
      expect(false).assertTrue()
      done()
    }
  })

  /**
   * @tc.desc: Test query sysevent with 2 conditions: == & ==.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest018
   * @tc.number: hiSysEventJsUnitTest018
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest018', 0, async function (done) {
    writeCustomizedSysEvent({
      PID: 323232388,
      UID: 1,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest018",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 1000,
      UID: 1,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest018",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 1000,
      UID: 1,
      PACKAGE_NAME: "com.ohos.testHiSysEvent2",
      PROCESS_NAME: "hiview js test suite"
    })
    setTimeout(() => {
      querySysEvent({
        beginTime: -1,
        endTime: -1,
        maxEvents: 5,
      }, [{
        domain: "RELIABILITY",
        names: ["STACK"],
        condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"=","value":1000},' +
          '{"param":"PACKAGE_NAME","op":"=","value":"com.ohos.testHiSysEvent2"}]}}'
      }], (infos) => {
        expect(infos.length >= 0).assertTrue()
      }, (reason, total) => {
        expect(total >= 1).assertTrue()
      }, (err) => {
        expect(false).assertTrue()
      }, done)
    }, 1000)
  })

  /**
   * @tc.desc: Test query sysevent with conditions: <= & >=.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest019
   * @tc.number: hiSysEventJsUnitTest019
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest019', 0, async function (done) {
    writeCustomizedSysEvent({
      PID: 222,
      UID: 10,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest019",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 222,
      UID: 20,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest019",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 222,
      UID: 23,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest019",
      PROCESS_NAME: "hiview js test suite"
    })
    setTimeout(() => {
      querySysEvent({
        beginTime: -1,
        endTime: -1,
        maxEvents: 5,
      }, [{
        domain: "RELIABILITY",
        names: ["STACK"],
        condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"<=","value":222},' +
          '{"param":"UID","op":">=","value":19.0}]}}'
      }], (infos) => {
        expect(infos.length >= 0).assertTrue()
      }, (reason, total) => {
        expect(total >= 2).assertTrue()
      }, (err) => {
        expect(false).assertTrue()
      }, done)
    }, 1500)
  })

  /**
   * @tc.desc: Test query sysevent with conditions: > & <.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest020
   * @tc.number: hiSysEventJsUnitTest020
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest020', 0, async function (done) {
    writeCustomizedSysEvent({
      PID: 2009,
      UID: 20001,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest020",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 2010,
      UID: 20002,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest020",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 2020,
      UID: 20003,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest020",
      PROCESS_NAME: "hiview js test suite"
    })
    setTimeout(() => {
      querySysEvent({
        beginTime: -1,
        endTime: -1,
        maxEvents: 5,
      }, [{
        domain: "RELIABILITY",
        names: ["STACK"],
        condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":">","value":2000},' +
          '{"param":"UID","op":"<","value":20003}]}}'
      }], (infos) => {
        expect(infos.length >= 0).assertTrue()
      }, (reason, total) => {
        expect(total >= 2).assertTrue()
      }, (err) => {
        expect(false).assertTrue()
      }, done)
    }, 2000)
  })

  /**
   * @tc.desc: Test query sysevent with 2 conditions: != & ==.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest021
   * @tc.number: hiSysEventJsUnitTest021
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest021', 0, async function (done) {
    writeCustomizedSysEvent({
      PID: 22,
      UID: 88888,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest021",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 23,
      UID: 88888,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest021",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 24,
      UID: 88888,
      PACKAGE_NAME: "com.huawei.hiSysEventJsUnitTest021",
      PROCESS_NAME: "hiview js test suite"
    })
    setTimeout(() => {
      querySysEvent({
        beginTime: -1,
        endTime: -1,
        maxEvents: 5,
      }, [{
        domain: "RELIABILITY",
        names: ["STACK"],
        condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"!=","value":22}, ' +
          '{"param":"UID","op":"=","value":88888}]}}'
      }], (infos) => {
        expect(infos.length >= 0).assertTrue()
      }, (reason, total) => {
        expect(total >= 2).assertTrue()
      }, (err) => {
        expect(false).assertTrue()
      }, done)
    }, 2500)
  })

  /**
   * @tc.desc: Test query sysevent with null condition.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest022
   * @tc.number: hiSysEventJsUnitTest022
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest022', 0, async function (done) {
    setTimeout(() => {
      querySysEvent({
        beginTime: -1,
        endTime: -1,
        maxEvents: 5,
      }, [{
        domain: "RELIABILITY",
        names: ["STACK"],
        condition: null
      }], (infos) => {
        expect(infos.length >= 0).assertTrue()
      }, (reason, total) => {
        expect(total > 0).assertTrue()
      }, (err) => {
        expect(false).assertTrue()
      }, done)
    }, 2500)
  })

  /**
   * @tc.desc: Test query sysevent with default query argument.
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest023
   * @tc.number: hiSysEventJsUnitTest023
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest023', 0, async function (done) {
    setTimeout(() => {
      querySysEvent({
        beginTime: -1,
        endTime: -1,
        maxEvents: -1,
      }, [{
        domain: "RELIABILITY",
        names: ["STACK"],
        condition: null
      }], (infos) => {
        expect(infos.length >= 0).assertTrue()
      }, (reason, total) => {
        expect(total > 0).assertTrue()
      }, (err) => {
        expect(false).assertTrue()
      }, done)
    }, 2500)
  })

  /**
   * @tc.desc: Test write with integer number
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest024
   * @tc.number: hiSysEventJsUnitTest024
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest024', 0, async function (done) {
    writeCustomizedSysEvent({
      PID: 2222222,
      UID: 2222222,
      FIRST_INT_VAL: 1,
      SECOND_INT_VAL: -1,
      THIRD_INT_VAL: 123456789,
      FORTH_INT_VAL: -123456789,
    })
    setTimeout(() => {
      querySysEvent({
        beginTime: -1,
        endTime: -1,
        maxEvents: 1,
      }, [{
        domain: "RELIABILITY",
        names: ["STACK"],
        condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"=","value":2222222},' +
          '{"param":"UID","op":"=","value":2222222}]}}'
      }], (infos) => {
        expect(infos.length >= 0).assertTrue()
        expect(GetParam(infos[0], "FIRST_INT_VAL")).assertEqual(1)
        expect(GetParam(infos[0], "SECOND_INT_VAL")).assertEqual(-1)
        expect(GetParam(infos[0], "THIRD_INT_VAL")).assertEqual(123456789)
        expect(GetParam(infos[0], "FORTH_INT_VAL")).assertEqual(-123456789)
      }, (reason, total) => {
        expect(total == 1).assertTrue()
      }, (err) => {
        expect(false).assertTrue()
      }, done)
    }, 1500)
  })

  /**
   * @tc.desc: Test write with big integer number
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest025
   * @tc.number: hiSysEventJsUnitTest025
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest025', 0, async function (done) {
    writeCustomizedSysEvent({
      PID: 3333333,
      UID: 3333333,
      FIRST_BIG_INT_VAL: 1n,
      SECOND_BIG_INT_VAL: -1n,
      THIRD_BIG_INT_VAL: 123456789n,
      FORTH_BIG_INT_VAL: -123456789n,
    })
    setTimeout(() => {
      querySysEvent({
        beginTime: -1,
        endTime: -1,
        maxEvents: 1,
      }, [{
        domain: "RELIABILITY",
        names: ["STACK"],
        condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"=","value":3333333},' +
          '{"param":"UID","op":"=","value":3333333}]}}'
      }], (infos) => {
        expect(infos.length >= 0).assertTrue()
        expect(GetParam(infos[0], "FIRST_BIG_INT_VAL")).assertEqual(1)
        expect(GetParam(infos[0], "SECOND_BIG_INT_VAL")).assertEqual(-1)
        expect(GetParam(infos[0], "THIRD_BIG_INT_VAL")).assertEqual(123456789)
        expect(GetParam(infos[0], "FORTH_BIG_INT_VAL")).assertEqual(-123456789)
      }, (reason, total) => {
        expect(total == 1).assertTrue()
      }, (err) => {
        expect(false).assertTrue()
      }, done)
    }, 1500)
  })

  /**
   * @tc.desc: Test write with max or min big integer number
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest026
   * @tc.number: hiSysEventJsUnitTest026
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest026', 0, async function (done) {
    writeCustomizedSysEvent({
      PID: 4444444,
      UID: 4444444,
      UINT64_MAX: 18446744073709551615n,
      INT64_MAX: 9223372036854775807n,
      INT64_MIN: -9223372036854775808n,
    })
    setTimeout(() => {
      querySysEvent({
        beginTime: -1,
        endTime: -1,
        maxEvents: 1,
      }, [{
        domain: "RELIABILITY",
        names: ["STACK"],
        condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"=","value":4444444},' +
          '{"param":"UID","op":"=","value":4444444}]}}'
      }], (infos) => {
        expect(infos.length >= 0).assertTrue()
        expect(GetParam(infos[0], "UINT64_MAX")).assertEqual(18446744073709551615n)
        expect(GetParam(infos[0], "INT64_MAX")).assertEqual(9223372036854775807n)
        expect(GetParam(infos[0], "INT64_MIN")).assertEqual(-9223372036854775808n)
      }, (reason, total) => {
        expect(total == 1).assertTrue()
      }, (err) => {
        expect(false).assertTrue()
      }, done)
    }, 1500)
  })

  /**
   * @tc.desc: Test write with big integer number array
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest027
   * @tc.number: hiSysEventJsUnitTest027
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest027', 0, async function (done) {
    writeCustomizedSysEvent({
      PID: 55555555,
      UID: 55555555,
      FIRST_BIG_INT_ARR: [4n, 5n, 6n],
      SECOND_BIG_INT_ARR: [-4n, -5n, -6n],
      THIRD_BIG_INT_ARR: [123456789n, -2232333n, 2222223344n],
      FORTH_BIG_INT_ARR: [-123456789n, -2232333n, -2222223344n],
    })
    setTimeout(() => {
      querySysEvent({
        beginTime: -1,
        endTime: -1,
        maxEvents: 1,
      }, [{
        domain: "RELIABILITY",
        names: ["STACK"],
        condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"=","value":55555555},' +
          '{"param":"UID","op":"=","value":55555555}]}}'
      }], (infos) => {
        expect(infos.length >= 0).assertTrue()
        expect(GetArrayIemParamByIndex(infos[0], "FIRST_BIG_INT_ARR", 1)).assertEqual(5)
        expect(GetArrayIemParamByIndex(infos[0], "SECOND_BIG_INT_ARR", 2)).assertEqual(-6)
        expect(GetArrayIemParamByIndex(infos[0], "THIRD_BIG_INT_ARR", 1)).assertEqual(-2232333)
        expect(GetArrayIemParamByIndex(infos[0], "FORTH_BIG_INT_ARR", 2)).assertEqual(-2222223344n)
      }, (reason, total) => {
        expect(total == 1).assertTrue()
      }, (err) => {
        expect(false).assertTrue()
      }, done)
    }, 1500)
  })

  /**
   * @tc.desc: Test write with integer number array
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest028
   * @tc.number: hiSysEventJsUnitTest028
   * @tc.type: Function
   * @tc.size: MediumTest
   */
    it('hiSysEventJsUnitTest028', 0, async function (done) {
      writeCustomizedSysEvent({
        PID: 66666666,
        UID: 66666666,
        FIRST_INT_ARR: [1, 2, 3],
        SECOND_INT_ARR: [-1, -2, -3],
        THIRD_INT_ARR: [123456789, -2232333, 2222223344],
        FORTH_INT_ARR: [-123456, 222333, -222222],
      })
      setTimeout(() => {
        querySysEvent({
          beginTime: -1,
          endTime: -1,
          maxEvents: 1,
        }, [{
          domain: "RELIABILITY",
          names: ["STACK"],
          condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"=","value":66666666},' +
            '{"param":"UID","op":"=","value":66666666}]}}'
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
          expect(GetArrayIemParamByIndex(infos[0], "FIRST_INT_ARR", 1)).assertEqual(2)
          expect(GetArrayIemParamByIndex(infos[0], "SECOND_INT_ARR", 2)).assertEqual(-3)
          expect(GetArrayIemParamByIndex(infos[0], "THIRD_INT_ARR", 1)).assertEqual(-2232333)
          expect(GetArrayIemParamByIndex(infos[0], "FORTH_INT_ARR", 2)).assertEqual(-222222)
        }, (reason, total) => {
          expect(total == 1).assertTrue()
        }, (err) => {
          expect(false).assertTrue()
        }, done)
      }, 1500)
    })
});