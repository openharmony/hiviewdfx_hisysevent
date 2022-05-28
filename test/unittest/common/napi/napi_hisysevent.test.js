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

    /**
     * @tc.name: hiSysEventJsUnitTest001
     * @tc.desc: Test hisysevent writing with calling AsyncCallback
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest001', 0, async function (done) {
        console.info('hiSysEventJsUnitTest001 start')
        let result = 0;
        hiSysEvent.write({
            domain: "RELIABILITY",
            name: "STACK",
            eventType: hiSysEvent.EventType.FAULT,
            params: {
                "PID": 1,
                "UID": 1,
                "PACKAGE_NAME": "com.huawei.testHiSysEvent",
                "PROCESS_NAME": "hiview js test suite",
                "MSG": "no msg."
            }
        }, (err, val) => {
            if (err) {
                console.error('in hiSysEventJsUnitTest001 test callback: err.code = ' + err.code);
                result = err.code;
            } else {
                console.info('in hiSysEventJsUnitTest001 test callback: result = ' + val);
                result = val;
            }
            expect(result).assertEqual(0)
            done()
        })
        console.info('hiSysEventJsUnitTest001 end')
    });

    /**
     * @tc.name: hiSysEventJsUnitTest002
     * @tc.desc: Test hisysevent writing with returning Promise.
     * @tc.type: FUNC
     */
    it('hiSysEventJsUnitTest002', 0, async function (done) {
        console.info('hiSysEventJsUnitTest002 start')
        let result = 0;
        hiSysEvent.write({
            domain: "RELIABILITY",
            name: "STACK",
            eventType: hiSysEvent.EventType.FAULT,
            params: {
                "PID": 1,
                "UID": 1,
                "PACKAGE_NAME": "com.huawei.testHiSysEvent",
                "PROCESS_NAME": "hiview js test suite",
                "MSG": "no msg."
            }
        }).then(
            (val) => {
                console.info('in hiSysEventJsUnitTest002 test callback: result = ' + val);
                result = val
                expect(result).assertEqual(0)
                done()
            }
        ).catch(
            (err) => {
                console.error('in hiSysEventJsUnitTest002 test callback: err.code = ' + err.code);
                result = err.code;
                done()
            }
        );
        console.info('hiSysEventJsUnitTest002 end')
    });
});