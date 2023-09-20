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

    function writeSysEvent(customized) {
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: customized
            }, (err, val) => {
                if (err) {
                    console.error(`in hiSysEventJsUnitTest018 test callback: err.code = ${err.code}, error msg is ${err.message}`)
                    expect(false).assertTrue()
                }
            })
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    }

    /**
     * @tc.name: hiSysEventJsUnitTest001
     * @tc.desc: Test hisysevent writing with calling AsyncCallback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest001', 0, async function (done) {
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    expect(false).assertTrue()
                } else {
                    expect(val).assertEqual(0)
                }
                done()
            })
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest002
     * @tc.desc: Test hisysevent writing with returning Promise.
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest002', 0, async function (done) {
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }).then(
                (val) => {
                    expect(val).assertEqual(0)
                    done()
                }
            ).catch(
                (err) => {
                    expect(false).assertTrue()
                    done()
                }
            );
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest003
     * @tc.desc: Test function return of adding/remove hisysevent watcher result.
     * @tc.type: FUNC
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
     * @tc.name: hiSysEventJsUnitTest004
     * @tc.desc: Test watcher callback
     * @tc.type: FUNC
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
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
            })
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
     * @tc.name: hiSysEventJsUnitTest005
     * @tc.desc: Test query callback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest005', 0, async function (done) {
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview napi test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
            })
            setTimeout(() => {
                try {
                    hiSysEvent.query({
                        beginTime: -1,
                        endTime: -1,
                        maxEvents: 2,
                    }, [{
                        domain: "RELIABILITY",
                        names: ["STACK"],
                    }], {
                        onQuery: function (infos) {
                            expect(infos.length >= 0).assertTrue()
                        },
                        onComplete: function(reason, total) {
                            expect(true).assertTrue()
                            done()
                        }
                    })
                } catch (err) {
                    expect(err.code == 201).assertTrue()
                    done()
                }
            }, 1000);
        } catch (err) {
            expect(err.code == 201).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest006
     * @tc.desc: Test query callback with domain which length is over 16
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest006', 0, async function (done) {
        console.info('hiSysEventJsUnitTest006 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview napi test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
            })
            setTimeout(() => {
                try {
                    hiSysEvent.query({
                        beginTime: -1,
                        endTime: -1,
                        maxEvents: 2,
                    }, [{
                        domain: "RELIABILITY_RELIABILITY",
                        names: ["STACK"],
                    }], {
                        onQuery: function (infos) {
                            expect(infos.length >= 0).assertTrue()
                        },
                        onComplete: function(reason, total) {
                            expect(true).assertTrue()
                            done()
                        }
                    })
                } catch (err) {
                    expect(err.code == 11200302 || err.code == 11200304).assertTrue()
                    done()
                }
            }, 1000);
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest007
     * @tc.desc: Test query callback with domain which length is over 32
     * @tc.type: FUNC
     */
     it('hiSysEventJsUnitTest007', 0, async function (done) {
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview napi test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
            })
            setTimeout(() => {
                try {
                    hiSysEvent.query({
                        beginTime: -1,
                        endTime: -1,
                        maxEvents: 2,
                    }, [{
                        domain: "RELIABILITY",
                        names: ["STACK_STACK_STACK_STACK_STACK_STACK"],
                    }], {
                        onQuery: function (infos) {
                            expect(infos.length >= 0).assertTrue()
                        },
                        onComplete: function(reason, total) {
                            expect(true).assertTrue()
                            done()
                        }
                    })
                } catch (err) {
                    expect(err.code == 11200302 || err.code == 11200304).assertTrue()
                    done()
                }
            }, 1000);
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest008
     * @tc.desc: Test hisysevent of invalid domain writing with calling AsyncCallback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest008', 0, async function (done) {
        try {
            hiSysEvent.write({
                domain: "RELIABILITY_RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    expect(err.code == 11200001).assertTrue()
                } else {
                    expect(false).assertTrue()
                }
                done()
            })
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest009
     * @tc.desc: Test hisysevent of invalid event name writing with calling AsyncCallback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest009', 0, async function (done) {
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK_STACK_STACK_STACK_STACK_STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    expect(err.code == 11200002).assertTrue()
                } else {
                    expect(false).assertTrue()
                }
                done()
            })
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest010
     * @tc.desc: Test hisysevent which is over size writing with calling AsyncCallback
     * @tc.type: FUNC
     */
     it('hiSysEventJsUnitTest010', 0, async function (done) {
        let params = {
            PID: 1,
            UID: 1,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "just a testcase",
            MSG: "no msg."
        }
        for (let i = 0; i < 40; i++) {
            params[`bundle${i}`] = Array.from({length: 10 * 1024}).join("ohos")
        }
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: params,
            }, (err, val) => {
                if (err) {
                    expect(err.code == 11200004).assertTrue()
                } else {
                    expect(false).assertTrue()
                }
                done()
            })
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest011
     * @tc.desc: Test hisysevent of invalid param name writing with calling AsyncCallback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest011', 0, async function (done) {
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK_STACK: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    expect(err.code == 11200051).assertTrue()
                } else {
                    expect(false).assertTrue()
                }
                done()
            })
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest012
     * @tc.desc: Test hisysevent with string over limit writing with calling AsyncCallback
     * @tc.type: FUNC
     */
     it('hiSysEventJsUnitTest012', 0, async function (done) {
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: Array.from({length: 10 * 1024 + 10}).join("ohos"),
                    MSG: "no msg."
                }
            }, (err, val) => {
                if (err) {
                    expect(err.code == 11200052).assertTrue()
                } else {
                    expect(false).assertTrue()
                }
                done()
            })
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest013
     * @tc.desc: Test hisysevent with param count over limit writing with calling AsyncCallback
     * @tc.type: FUNC
     */
     it('hiSysEventJsUnitTest013', 0, async function (done) {
        let largeParams = {}
        for (let i = 0; i < 200; i++) {
            largeParams["name" + i] = i
        }
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: largeParams
            }, (err, val) => {
                if (err) {
                    expect(err.code == 11200053).assertTrue()
                } else {
                    expect(false).assertTrue()
                }
                done()
            })
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest014
     * @tc.desc: Test hisysevent with array size over limit writing with calling AsyncCallback
     * @tc.type: FUNC
     */
     it('hiSysEventJsUnitTest014', 0, async function (done) {
        let msgArray = []
        for (let i = 0; i < 200; i++) {
            msgArray[i] = i
        }
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {
                    PID: 1,
                    UID: 1,
                    PACKAGE_NAME: "com.huawei.testHiSysEvent",
                    PROCESS_NAME: "hiview js test suite",
                    MSG: msgArray
                }
            }, (err, val) => {
                if (err) {
                    expect(err.code == 11200054).assertTrue()
                } else {
                    expect(false).assertTrue()
                }
                done()
            })
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest015
     * @tc.desc: Test hisysevent query with sequence
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest015', 0, async function (done) {
        try {
            hiSysEvent.query({
                maxEvents: 10000,
                fromSeq: 100,
                toSeq: 1000,
            }, [{
                domain: "AAFWK",
                names: ["CONNECT_SERVICE"],
            }], {
                onQuery: function (infos) {
                    expect(infos.length >= 0).assertTrue()
                },
                onComplete: function(reason, total, seq) {
                    expect(true).assertTrue()
                    done()
                }
            })
        } catch (err) {
            expect(err.code == 201).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest016
     * @tc.desc: Test hisysevent get max sequence
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest016', 0, async function (done) {
        try {
            hiSysEvent.query({
                maxEvents: 0,
                fromSeq: 0,
                toSeq: 1000,
            }, [{
                domain: "AAFWK",
                names: ["CONNECT_SERVICE"],
            }], {
                onQuery: function (infos) {
                    expect(infos.length >= 0).assertTrue()
                },
                onComplete: function(reason, total, seq) {
                    expect(true).assertTrue()
                    done()
                }
            })
        } catch (err) {
            expect(err.code == 201).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest017
     * @tc.desc: Test writing sysevents more than 100 times in 5 seconds
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest017', 0, async function (done) {
        try {
            for (let index = 0; index < 102; index++) {
                hiSysEvent.write({
                    domain: "USERIAM_PIN",
                    name: "USERIAM_TEMPLATE_CHANGE",
                    eventType: hiSysEvent.EventType.SECURITY,
                    params: {
                        PID: 1,
                        UID: 1,
                        PACKAGE_NAME: "com.huawei.testHiSysEvent",
                        PROCESS_NAME: "hiview js test suite",
                    }
                }, (err, val) => {
                    if (err) {
                        expect(err.code == 11200003).assertTrue()
                        done()
                    }
                })
            }
        } catch (err) {
            expect(false).assertTrue()
            done()
        }
    })

    /**
     * @tc.name: hiSysEventJsUnitTest018
     * @tc.desc: Test query sysevent with 2 conditions: == & ==
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest018', 0, async function (done) {
        writeSysEvent({
            PID: 323232388,
            UID: 1,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        writeSysEvent({
            PID: 1000,
            UID: 1,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        writeSysEvent({
            PID: 1000,
            UID: 1,
            PACKAGE_NAME: "com.huawei.testHiSysEvent2",
            PROCESS_NAME: "hiview js test suite"
        })
        setTimeout(() => {
            try {
                hiSysEvent.query({
                    beginTime: -1,
                    endTime: -1,
                    maxEvents: 5,
                }, [{
                    domain: "RELIABILITY",
                    names: ["STACK"],
                    condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"=","value":1000},' +
                        '{"param":"PACKAGE_NAME","op":"=","value":"com.huawei.testHiSysEvent2"}]}}'
                }], {
                    onQuery: function (infos) {
                        expect(infos.length >= 0).assertTrue()
                    },
                    onComplete: function(reason, total, seq) {
                        expect(total >= 1).assertTrue()
                        done()
                    }
                })
            } catch (err) {
                expect(false).assertTrue()
                done()
            }
        }, 1000)
    })

    /**
     * @tc.name: hiSysEventJsUnitTest019
     * @tc.desc: Test query sysevent with conditions: <= & >=
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest019', 0, async function (done) {
        writeSysEvent({
            PID: 222,
            UID: 10,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        writeSysEvent({
            PID: 222,
            UID: 20,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        writeSysEvent({
            PID: 222,
            UID: 23,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        setTimeout(() => {
            try {
                hiSysEvent.query({
                    beginTime: -1,
                    endTime: -1,
                    maxEvents: 5,
                }, [{
                    domain: "RELIABILITY",
                    names: ["STACK"],
                    condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"<=","value":222},' +
                        '{"param":"UID","op":">=","value":19.0}]}}'
                }], {
                    onQuery: function (infos) {
                        expect(infos.length >= 0).assertTrue()
                    },
                    onComplete: function(reason, total, seq) {
                        expect(total >= 2).assertTrue()
                        done()
                    }
                })
            } catch (err) {
                expect(false).assertTrue()
                done()
            }
        }, 1500)
    })

    /**
     * @tc.name: hiSysEventJsUnitTest020
     * @tc.desc: Test query sysevent with conditions: > & <
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest020', 0, async function (done) {
        writeSysEvent({
            PID: 2009,
            UID: 20001,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        writeSysEvent({
            PID: 2010,
            UID: 20002,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        writeSysEvent({
            PID: 2020,
            UID: 20003,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        setTimeout(() => {
            try {
                hiSysEvent.query({
                    beginTime: -1,
                    endTime: -1,
                    maxEvents: 5,
                }, [{
                    domain: "RELIABILITY",
                    names: ["STACK"],
                    condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":">","value":2000},' +
                        '{"param":"UID","op":"<","value":20003}]}}'
                }], {
                    onQuery: function (infos) {
                        expect(infos.length >= 0).assertTrue()
                    },
                    onComplete: function(reason, total, seq) {
                        expect(total >= 2).assertTrue()
                        done()
                    }
                })
            } catch (err) {
                expect(false).assertTrue()
                done()
            }
        }, 2000)
    })

    /**
     * @tc.name: hiSysEventJsUnitTest021
     * @tc.desc: Test query sysevent with 2 conditions: != & ==
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest021', 0, async function (done) {
        writeSysEvent({
            PID: 22,
            UID: 88888,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        writeSysEvent({
            PID: 23,
            UID: 88888,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        writeSysEvent({
            PID: 24,
            UID: 88888,
            PACKAGE_NAME: "com.huawei.testHiSysEvent",
            PROCESS_NAME: "hiview js test suite"
        })
        setTimeout(() => {
            try {
                hiSysEvent.query({
                    beginTime: -1,
                    endTime: -1,
                    maxEvents: 5,
                }, [{
                    domain: "RELIABILITY",
                    names: ["STACK"],
                    condition: '{"version":"V1","condition":{"and":[{"param":"PID","op":"!=","value":22}, ' +
                        '{"param":"UID","op":"=","value":88888}]}}'
                }], {
                    onQuery: function (infos) {
                        expect(infos.length >= 0).assertTrue()
                    },
                    onComplete: function(reason, total, seq) {
                        expect(total >= 2).assertTrue()
                        done()
                    }
                })
            } catch (err) {
                expect(false).assertTrue()
                done()
            }
        }, 2500)
    })

    /**
     * @tc.name: hiSysEventJsUnitTest022
     * @tc.desc: Test query sysevent with null condition
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest022', 0, async function (done) {
        setTimeout(() => {
            try {
                hiSysEvent.query({
                    beginTime: -1,
                    endTime: -1,
                    maxEvents: 5,
                }, [{
                    domain: "RELIABILITY",
                    names: ["STACK"],
                    condition: null
                }], {
                    onQuery: function (infos) {
                        expect(infos.length >= 0).assertTrue()
                    },
                    onComplete: function(reason, total, seq) {
                        expect(total >  0).assertTrue()
                        done()
                    }
                })
            } catch (err) {
                expect(false).assertTrue()
                done()
            }
        }, 2500)
    })
});