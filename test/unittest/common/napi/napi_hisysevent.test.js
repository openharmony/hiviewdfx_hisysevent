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

  const DEFAULT_QUERY_TIME = -1

  const DEFAULT_ID = 1

  const PERMISSION_ERR = 201

  const PARAM_CHECK_ERR = 401

  const INVALID_QUERY_RULE_ERR = 11200302

  const QUERY_TOO_FREQUENTLY_ERR = 11200304

  const DELAY_1500_MS = 1500

  const DELAY_2000_MS = 2000

  const QUERY_1_ITEM = 1

  const QUERY_2_ITEMS = 2

  const FIRST_ITEM_INDEX = 0

  const SECOND_ITEM_INDEX = 1

  const THRID_ITEM_INDEX = 2

  const defaultEventInfo = {
    PID: DEFAULT_ID,
    UID: DEFAULT_ID,
    PACKAGE_NAME: "com.ohos.testHiSysEvent",
    PROCESS_NAME: "hiview js test suite",
    MSG: "no msg."
  }

  const defaultQueryArg = {
    beginTime: DEFAULT_QUERY_TIME,
    endTime: DEFAULT_QUERY_TIME,
    maxEvents: -1, // -1 means query cnt with no limit
  }

  const queryArgWithOneCnt = {
    beginTime: DEFAULT_QUERY_TIME,
    endTime: DEFAULT_QUERY_TIME,
    maxEvents: QUERY_1_ITEM, // 1 is query count
  }

  const queryArgWithTwoCnt = {
    beginTime: DEFAULT_QUERY_TIME,
    endTime: DEFAULT_QUERY_TIME,
    maxEvents: QUERY_2_ITEMS,
  }

  const queryArgWithFiveCnt = {
    beginTime: DEFAULT_QUERY_TIME,
    endTime: DEFAULT_QUERY_TIME,
    maxEvents: 5, // 5 is query count
  }

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
    writeCustomizedSysEvent(defaultEventInfo)
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

  function BuildQueryCondition(testId)
  {
    return '{"version":"V1","condition":{"and":[{"param":"PID","op":"=","value":'
      + String(testId) + '},{"param":"UID","op":"=","value":'
      + String(testId) + '}]}}'
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
    writeEventWithAsyncWork("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT, defaultEventInfo,
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
    writeEventWithPromise("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT, defaultEventInfo,
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
      expect(err.code == PERMISSION_ERR).assertTrue()
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
          expect(err.code == PERMISSION_ERR).assertTrue()
          done()
        }
      }, 1000)
    } catch (err) {
      expect(err.code == PERMISSION_ERR).assertTrue()
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
        querySysEvent(queryArgWithTwoCnt,
          [{
            domain: "RELIABILITY",
            names: ["STACK"],
          }], (infos) => {
            expect(infos.length >= 0).assertTrue()
          }, (reason, total) => {
            expect(true).assertTrue()
          }, (err) => {
            expect(err.code == PERMISSION_ERR).assertTrue()
          }, done)
      }, 1000);
    } catch (err) {
      expect(err.code == PERMISSION_ERR).assertTrue()
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
        querySysEvent(queryArgWithTwoCnt,
          [{
            domain: "RELIABILITY_RELIABILITY",
            names: ["STACK"],
          }], (infos) => {
            expect(infos.length >= 0).assertTrue()
          }, (reason, total) => {
            expect(true).assertTrue()
          }, (err) => {
            expect(err.code == INVALID_QUERY_RULE_ERR || err.code == QUERY_TOO_FREQUENTLY_ERR).assertTrue()
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
        querySysEvent(queryArgWithTwoCnt,
          [{
            domain: "RELIABILITY",
            names: ["STACK_STACK_STACK_STACK_STACK_STACK"],
          }], (infos) => {
            expect(infos.length >= 0).assertTrue()
          }, (reason, total) => {
            expect(true).assertTrue()
          }, (err) => {
            expect(err.code == INVALID_QUERY_RULE_ERR || err.code == QUERY_TOO_FREQUENTLY_ERR).assertTrue()
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
    writeEventWithAsyncWork("RELIABILITY_RELIABILITY", "STACK", hiSysEvent.EventType.FAULT, defaultEventInfo,
      (err) => {
        expect(err.code == 11200001).assertTrue() // 11200001: error code for invalid domain
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
      defaultEventInfo,
      (err) => {
        expect(err.code == 11200002).assertTrue() // 11200002: error code for invalid event name
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
    for (let i = 0; i < 40; i++) {  // 40 is a test limiting value
      defaultEventInfo[`bundle${i}`] = Array.from({length: 10 * 1024}).join("ohos")
    }
    writeEventWithAsyncWork("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      defaultEventInfo,
      (err) => {
        expect(err.code == 11200004).assertTrue() // 11200004: error code for content over limit
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
        PID: DEFAULT_ID,
        UID: DEFAULT_ID,
        STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK: "com.ohos.testHiSysEvent",
        PROCESS_NAME: "hiview js test suite",
        MSG: "no msg."
      }, (err) => {
        expect(err.code == 11200051).assertTrue() // 11200051: error code for invalid parameter name
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
        PID: DEFAULT_ID,
        UID: DEFAULT_ID,
        PACKAGE_NAME: "com.ohos.testHiSysEvent",
        PROCESS_NAME: Array.from({length: 10 * 1024 + 10}).join("ohos"), // 10 * 1024 + 10 is a threshold value
        MSG: "no msg."
      }, (err) => {
        expect(err.code == 11200052).assertTrue() // 11200052: error code for length of string over limit
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
    for (let i = 0; i < 200; i++) { // 200 is a test limiting value
      largeParams["name" + i] = i
    }
    writeEventWithAsyncWork("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      largeParams, (err) => {
        expect(err.code == 11200053).assertTrue() // 11200053: error code for parameter count over limit
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
    for (let i = 0; i < 200; i++) { // 200 is a test limiting value
      msgArray[i] = i
    }
    writeEventWithAsyncWork("RELIABILITY", "STACK", hiSysEvent.EventType.FAULT,
      {
        PID: DEFAULT_ID,
        UID: DEFAULT_ID,
        PACKAGE_NAME: "com.ohos.testHiSysEvent",
        PROCESS_NAME: "hiview js test suite",
        MSG: msgArray
      }, (err) => {
        expect(err.code == 11200054).assertTrue() // 11200054: error code for array size over limit
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
      maxEvents: 10000, // 10000 is a test value
      fromSeq: 100, // 100 is a test value
      toSeq: 1000, // 1000 is a test value
    }, [{
      domain: "AAFWK",
      names: ["CONNECT_SERVICE"],
    }], (infos) => {
      expect(infos.length >= 0).assertTrue()
    }, (reason, total) => {
      expect(true).assertTrue()
    }, (err) => {
      expect(err.code == PERMISSION_ERR).assertTrue()
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
      maxEvents: 0, // 0 is a test value
      fromSeq: 0, // 0 is a test value
      toSeq: 1000, // 1000 is a test value
    }, [{
      domain: "AAFWK",
      names: ["CONNECT_SERVICE"],
    }], (infos) => {
      expect(infos.length >= 0).assertTrue()
    }, (reason, total) => {
      expect(true).assertTrue()
    }, (err) => {
      expect(err.code == PERMISSION_ERR).assertTrue()
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
      for (let index = 0; index < 102; index++) { // 102 is a test limiting value
        writeEventWithAsyncWork("USERIAM_PIN", "USERIAM_TEMPLATE_CHANGE",
          hiSysEvent.EventType.SECURITY,
          {
            PID: DEFAULT_ID,
            UID: DEFAULT_ID,
            PACKAGE_NAME: "com.ohos.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite",
          }, (err) => {
            expect(err.code == 11200003).assertTrue() // 11200003: error code for abnormnal environment
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
      PID: 323232388,  // 323232388 is a test pid
      UID: DEFAULT_ID,
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest018",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 1000,      // 1000 is a test pid
      UID: DEFAULT_ID,
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest018",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 1000,     // 1000 is a test pid
      UID: DEFAULT_ID,
      PACKAGE_NAME: "com.ohos.testHiSysEvent2",
      PROCESS_NAME: "hiview js test suite"
    })
    setTimeout(() => {
      querySysEvent(queryArgWithFiveCnt,
        [{
          domain: "RELIABILITY",
          names: ["STACK"],
          condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"=","value":1000},' +
            '{"param":"PACKAGE_NAME","op":"=","value":"com.ohos.testHiSysEvent2"}]}}'
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
        }, (reason, total) => {
          expect(total >= QUERY_1_ITEM).assertTrue()
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
    const PID = 222
    writeCustomizedSysEvent({
      PID,
      UID: 10,  // 10 is a test value
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest019",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID,
      UID: 20,  // 20 is a test value
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest019",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID,
      UID: 23,  // 23 is a test value
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest019",
      PROCESS_NAME: "hiview js test suite"
    })
    setTimeout(() => {
      querySysEvent(queryArgWithFiveCnt,
        [{
          domain: "RELIABILITY",
          names: ["STACK"],
          condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"<=","value":222},' +
            '{"param":"UID","op":">=","value":19.0}]}}'
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
        }, (reason, total) => {
          expect(total >= QUERY_2_ITEMS).assertTrue() // 2 is a check value
        }, (err) => {
          expect(false).assertTrue()
        }, done)
    }, DELAY_1500_MS)
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
      PID: 2009,  // 2009 is a test value
      UID: 20001, // 20001 is a test value
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest020",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 2010,  // 2010 is a test value
      UID: 20002, // 20002 is a test value
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest020",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 2020,  // 2020 is a test value
      UID: 20003, // 20003 is a test value
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest020",
      PROCESS_NAME: "hiview js test suite"
    })
    setTimeout(() => {
      querySysEvent(queryArgWithFiveCnt,
        [{
          domain: "RELIABILITY",
          names: ["STACK"],
          condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":">","value":2000},' +
            '{"param":"UID","op":"<","value":20003}]}}'
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
        }, (reason, total) => {
          expect(total >= QUERY_2_ITEMS).assertTrue() // 2 is a test limit
        }, (err) => {
          expect(false).assertTrue()
        }, done)
    }, DELAY_2000_MS)
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
    const UID = 88888
    writeCustomizedSysEvent({
      PID: 22,    // 22 is a test value
      UID,
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest021",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 23,    // 23 is a test value
      UID,
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest021",
      PROCESS_NAME: "hiview js test suite"
    })
    writeCustomizedSysEvent({
      PID: 24,    // 24 is a test value
      UID,
      PACKAGE_NAME: "com.ohos.hiSysEventJsUnitTest021",
      PROCESS_NAME: "hiview js test suite"
    })
    setTimeout(() => {
      querySysEvent(queryArgWithFiveCnt,
        [{
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
      querySysEvent(queryArgWithFiveCnt,
        [{
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
      querySysEvent(defaultQueryArg,
        [{
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
   * @tc.desc: Test query with undefined as beginTime
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest024
   * @tc.number: hiSysEventJsUnitTest024
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest024', 0, async function (done) {
    querySysEvent({
      beginTime: undefined,
      endTime: DEFAULT_QUERY_TIME,
      maxEvents: QUERY_2_ITEMS
    }, [{
      domain: "KERNEL_VENDOR",
      names: ["POWER_KEY"],
    }], (infos) => {
      expect(false).assertTrue()
    }, (reason, total) => {
      expect(false).assertTrue()
    }, (err) => {
      expect(err.code == PARAM_CHECK_ERR).assertTrue()
    }, done)
  })

  /**
   * @tc.desc: Test query with null as endTime
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest025
   * @tc.number: hiSysEventJsUnitTest025
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest025', 0, async function (done) {
    querySysEvent({
      beginTime: DEFAULT_QUERY_TIME,
      endTime: null,
      maxEvents: QUERY_2_ITEMS
    }, [{
      domain: "AAFWK",
      names: ["PROCESS_EXIT"],
    }], (infos) => {
      expect(infos.length >= 0).assertTrue()
    }, (reason, total) => {
      expect(true).assertTrue()
    }, (err) => {
      expect(err.code == PARAM_CHECK_ERR).assertTrue()
    }, done)
  })

  /**
   * @tc.desc: Test query with undefined as querier
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest026
   * @tc.number: hiSysEventJsUnitTest026
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest026', 0, async function (done) {
    try {
      hiSysEvent.query(queryArgWithOneCnt, [{
        domain: "HUKS",
        names: ["FAULT"],
      }], undefined)
    } catch (err) {
      expect(err.code == PARAM_CHECK_ERR).assertTrue()
      done()
    }
  })

  /**
   * @tc.desc: Test query with null as querier
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest027
   * @tc.number: hiSysEventJsUnitTest027
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest027', 0, async function (done) {
    try {
      hiSysEvent.query(queryArgWithOneCnt, [{
        domain: "SAMGR",
        names: ["ONDEMAND_SA_LOAD"],
      }], null)
    } catch (err) {
      expect(err.code == PARAM_CHECK_ERR).assertTrue()
      done()
    }
  })

  /**
   * @tc.desc: Test write with integer number
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest028
   * @tc.number: hiSysEventJsUnitTest028
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest028', 0, async function (done) {
    const POS_INT_NUM = 2232333
    const NEG_INT_NUM = -2232333
    const ZERO_INT_NUM = 0
    const TEST_PID_UID = 282828
    writeCustomizedSysEvent({
      PID: TEST_PID_UID,
      UID: TEST_PID_UID,
      POS_INT_NUM, NEG_INT_NUM, ZERO_INT_NUM,
    })
    setTimeout(() => {
      querySysEvent(queryArgWithOneCnt,
        [{
          domain: "RELIABILITY",
          names: ["STACK"],
          condition: BuildQueryCondition(TEST_PID_UID),
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
          expect(GetParam(infos[0], "POS_INT_NUM")).assertEqual(POS_INT_NUM)
          expect(GetParam(infos[0], "NEG_INT_NUM")).assertEqual(NEG_INT_NUM)
          expect(GetParam(infos[0], "ZERO_INT_NUM")).assertEqual(ZERO_INT_NUM)
        }, (reason, total) => {
          expect(total == QUERY_1_ITEM).assertTrue()
        }, (err) => {
          expect(false).assertTrue()
        }, done)
    }, DELAY_1500_MS)
  })

  /**
   * @tc.desc: Test write with integer number array
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest029
   * @tc.number: hiSysEventJsUnitTest029
   * @tc.type: Function
   * @tc.size: MediumTest
   */
    it('hiSysEventJsUnitTest029', 0, async function (done) {
      const INT_NUM_ARR = [2232333, -2232333, 0]
      const TEST_PID_UID = 292929
      writeCustomizedSysEvent({
        PID: TEST_PID_UID,
        UID: TEST_PID_UID,
        INT_NUM_ARR,
      })
      setTimeout(() => {
        querySysEvent(queryArgWithOneCnt,
          [{
            domain: "RELIABILITY",
            names: ["STACK"],
            condition: BuildQueryCondition(TEST_PID_UID),
          }], (infos) => {
            expect(infos.length >= 0).assertTrue()
            const SEC_INDEX = 1
            const THIRD_INDEX = 2
            expect(GetArrayIemParamByIndex(
              infos[0], "INT_NUM_ARR", FIRST_ITEM_INDEX)).assertEqual(INT_NUM_ARR[FIRST_ITEM_INDEX])
            expect(GetArrayIemParamByIndex(
              infos[0], "INT_NUM_ARR", SECOND_ITEM_INDEX)).assertEqual(INT_NUM_ARR[SECOND_ITEM_INDEX])
            expect(GetArrayIemParamByIndex(
              infos[0], "INT_NUM_ARR", THRID_ITEM_INDEX)).assertEqual(INT_NUM_ARR[THRID_ITEM_INDEX])
          }, (reason, total) => {
            expect(total == QUERY_1_ITEM).assertTrue()
          }, (err) => {
            expect(false).assertTrue()
          }, done)
      }, DELAY_1500_MS)
    })

  /**
   * @tc.desc: Test write with signed big number
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest030
   * @tc.number: hiSysEventJsUnitTest030
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest030', 0, async function (done) {
    const MAX_UINT_BIG_NUM = 18446744073709551615n
    const MAX_INT_BIG_NUM = 9223372036854775807n
    const MIN_INT_BIG_NUM = -9223372036854775808n
    const TEST_PID_UID = 303030
    writeCustomizedSysEvent({
      PID: TEST_PID_UID,
      UID: TEST_PID_UID,
      MAX_UINT_BIG_NUM, MAX_INT_BIG_NUM, MIN_INT_BIG_NUM,
    })
    setTimeout(() => {
      querySysEvent(queryArgWithOneCnt,
        [{
          domain: "RELIABILITY",
          names: ["STACK"],
          condition: BuildQueryCondition(TEST_PID_UID),
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
          expect(BigInt(GetParam(infos[0], "MAX_UINT_BIG_NUM"))).assertEqual(MAX_UINT_BIG_NUM)
          expect(BigInt(GetParam(infos[0], "MAX_INT_BIG_NUM"))).assertEqual(MAX_INT_BIG_NUM)
          expect(BigInt(GetParam(infos[0], "MIN_INT_BIG_NUM"))).assertEqual(MIN_INT_BIG_NUM)
        }, (reason, total) => {
          expect(total == QUERY_1_ITEM).assertTrue()
        }, (err) => {
          expect(false).assertTrue()
        }, done)
    }, DELAY_1500_MS)
  })

  /**
   * @tc.desc: Test write with signed big number array
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest031
   * @tc.number: hiSysEventJsUnitTest031
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest031', 0, async function (done) {
    const SIGNED_BIG_NUM_ARR = [9223372036854775807n, -9223372036854775808n]
    const TEST_PID_UID = 313131
    writeCustomizedSysEvent({
      PID: TEST_PID_UID,
      UID: TEST_PID_UID,
      SIGNED_BIG_NUM_ARR,
    })
    setTimeout(() => {
      querySysEvent(queryArgWithOneCnt,
        [{
          domain: "RELIABILITY",
          names: ["STACK"],
          condition: BuildQueryCondition(TEST_PID_UID),
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
          expect(BigInt(GetArrayIemParamByIndex(
            infos[0], "SIGNED_BIG_NUM_ARR", FIRST_ITEM_INDEX))).assertEqual(SIGNED_BIG_NUM_ARR[FIRST_ITEM_INDEX])
          expect(BigInt(GetArrayIemParamByIndex(
            infos[0], "SIGNED_BIG_NUM_ARR", SECOND_ITEM_INDEX))).assertEqual(SIGNED_BIG_NUM_ARR[SECOND_ITEM_INDEX])
        }, (reason, total) => {
          expect(total == QUERY_1_ITEM).assertTrue()
        }, (err) => {
          expect(false).assertTrue()
        }, done)
    }, DELAY_1500_MS)
  })

  /**
   * @tc.desc: Test write with unsigned big number array
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest032
   * @tc.number: hiSysEventJsUnitTest032
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest032', 0, async function (done) {
    const UNSIGNED_BIG_NUM_ARR = [18446744073709551615n, 0n]
    const TEST_PID_UID = 323232
    writeCustomizedSysEvent({
      PID: TEST_PID_UID,
      UID: TEST_PID_UID,
      UNSIGNED_BIG_NUM_ARR,
    })
    setTimeout(() => {
      querySysEvent(queryArgWithOneCnt,
        [{
          domain: "RELIABILITY",
          names: ["STACK"],
          condition: BuildQueryCondition(TEST_PID_UID),
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
          expect(BigInt(GetArrayIemParamByIndex(
            infos[0], "UNSIGNED_BIG_NUM_ARR", FIRST_ITEM_INDEX))).assertEqual(UNSIGNED_BIG_NUM_ARR[FIRST_ITEM_INDEX])
          expect(BigInt(GetArrayIemParamByIndex(
            infos[0], "UNSIGNED_BIG_NUM_ARR", SECOND_ITEM_INDEX))).assertEqual(UNSIGNED_BIG_NUM_ARR[SECOND_ITEM_INDEX])
        }, (reason, total) => {
          expect(total == QUERY_1_ITEM).assertTrue()
        }, (err) => {
          expect(false).assertTrue()
        }, done)
    }, DELAY_1500_MS)
  })

  /**
   * @tc.desc: Test write with floating number
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest033
   * @tc.number: hiSysEventJsUnitTest033
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest033', 0, async function (done) {
    const POS_FLOATING_NUM = 12345.0
    const NEG_FLOATING_NUM = -12345.0
    const ZERO_FLOATING_NUM = 0.0
    const TEST_PID_UID = 333333
    writeCustomizedSysEvent({
      PID: TEST_PID_UID,
      UID: TEST_PID_UID,
      POS_FLOATING_NUM, NEG_FLOATING_NUM, ZERO_FLOATING_NUM,
    })
    setTimeout(() => {
      querySysEvent(queryArgWithOneCnt,
        [{
          domain: "RELIABILITY",
          names: ["STACK"],
          condition: BuildQueryCondition(TEST_PID_UID),
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
          expect(GetParam(infos[0], "POS_FLOATING_NUM")).assertEqual(POS_FLOATING_NUM)
          expect(GetParam(infos[0], "NEG_FLOATING_NUM")).assertEqual(NEG_FLOATING_NUM)
          expect(GetParam(infos[0], "ZERO_FLOATING_NUM")).assertEqual(ZERO_FLOATING_NUM)
        }, (reason, total) => {
          expect(total == QUERY_1_ITEM).assertTrue()
        }, (err) => {
          expect(false).assertTrue()
        }, done)
    }, DELAY_1500_MS)
  })

  /**
   * @tc.desc: Test write with floating number array
   * @tc.level: Level 0
   * @tc.name: hiSysEventJsUnitTest034
   * @tc.number: hiSysEventJsUnitTest034
   * @tc.type: Function
   * @tc.size: MediumTest
   */
  it('hiSysEventJsUnitTest034', 0, async function (done) {
    const FLOAT_NUM_ARR = [12345.0, -12345.0, 0.0]
    const TEST_PID_UID = 343434
    writeCustomizedSysEvent({
      PID: TEST_PID_UID,
      UID: TEST_PID_UID,
      FLOAT_NUM_ARR,
    })
    setTimeout(() => {
      querySysEvent(queryArgWithOneCnt,
        [{
          domain: "RELIABILITY",
          names: ["STACK"],
          condition: BuildQueryCondition(TEST_PID_UID),
        }], (infos) => {
          expect(infos.length >= 0).assertTrue()
          expect(GetArrayIemParamByIndex(
            infos[0], "FLOAT_NUM_ARR", FIRST_ITEM_INDEX)).assertEqual(FLOAT_NUM_ARR[FIRST_ITEM_INDEX])
          expect(GetArrayIemParamByIndex(
            infos[0], "FLOAT_NUM_ARR", SECOND_ITEM_INDEX)).assertEqual(FLOAT_NUM_ARR[SECOND_ITEM_INDEX])
          expect(GetArrayIemParamByIndex(
            infos[0], "FLOAT_NUM_ARR", THRID_ITEM_INDEX)).assertEqual(FLOAT_NUM_ARR[THRID_ITEM_INDEX])
        }, (reason, total) => {
          expect(total == QUERY_1_ITEM).assertTrue()
        }, (err) => {
          expect(false).assertTrue()
        }, done)
    }, DELAY_1500_MS)
  })
});