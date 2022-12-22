/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

describe('hiSysEventPermissionJsTest', function () {
    beforeAll(function() {

        /**
         * @tc.setup: setup invoked before all test cases
         */
        console.info('hiSysEventPermissionJsTest beforeAll called')
    })

    afterAll(function() {

        /**
         * @tc.teardown: teardown invoked after all test cases
         */
        console.info('hiSysEventPermissionJsTest afterAll called')
    })

    beforeEach(function() {

        /**
         * @tc.setup: setup invoked before each test case
         */
        console.info('hiSysEventPermissionJsTest beforeEach called')
    })

    afterEach(function() {

        /**
         * @tc.teardown: teardown invoked after each test case
         */
        console.info('hiSysEventPermissionJsTest afterEach called')
    })

    /**
     * @tc.name: hiSysEventPermissionJsTest001
     * @tc.desc: Test the write interface with callbak.
     * @tc.type: FUNC
     */
    it('hiSysEventPermissionJsTest001', 0, async function (done) {
        console.info('hiSysEventPermissionJsTest001 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {}
            }, (err) => {
                expect(false).assertTrue()
                done()
            })
        } catch (err) {
            expect(err.code == 202).assertTrue()
            console.info('hiSysEventPermissionJsTest001 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventPermissionJsTest002
     * @tc.desc: Test the write interface with promise.
     * @tc.type: FUNC
     */
     it('hiSysEventPermissionJsTest002', 0, async function (done) {
        console.info('hiSysEventPermissionJsTest002 start')
        try {
            hiSysEvent.write({
                domain: "RELIABILITY",
                name: "STACK",
                eventType: hiSysEvent.EventType.FAULT,
                params: {}
            }).then(
                () => {
                    expect(false).assertTrue()
                    done()
                }
            ).catch(
                (err) => {
                    expect(false).assertTrue()
                    done()
                }
            );
        } catch (err) {
            expect(err.code == 202).assertTrue()
            console.info('hiSysEventPermissionJsTest002 end')
            done()
        }
    })

    /**
     * @tc.name: hiSysEventPermissionJsTest003
     * @tc.desc: Test the addWatcher interface.
     * @tc.type: FUNC
     */
    it('hiSysEventPermissionJsTest003', 0, function () {
        console.info('hiSysEventPermissionJsTest003 start')
        let watcher = {
            rules: [{
                domain: "RELIABILITY",
                name: "STACK",
                tag: "STABILITY",
                ruleType: hiSysEvent.RuleType.WHOLE_WORD,
            }],
            onEvent: (info) => {},
            onServiceDied: () => {}
        }
        try {
            hiSysEvent.addWatcher(watcher)
        } catch (err) {
            expect(err.code == 202).assertTrue()
            console.info('hiSysEventPermissionJsTest003 end')
        }
    })

    /**
     * @tc.name: hiSysEventPermissionJsTest004
     * @tc.desc: Test the removeWatcher interface.
     * @tc.type: FUNC
     */
     it('hiSysEventPermissionJsTest004', 0, function () {
        console.info('hiSysEventPermissionJsTest004 start')
        let watcher = {
            rules: [{
                domain: "RELIABILITY",
                name: "STACK",
                tag: "STABILITY",
                ruleType: hiSysEvent.RuleType.WHOLE_WORD,
            }],
            onEvent: (info) => {},
            onServiceDied: () => {}
        }
        try {
            hiSysEvent.removeWatcher(watcher)
        } catch (err) {
            expect(err.code == 202).assertTrue()
            console.info('hiSysEventPermissionJsTest004 end')
        }
    })

    /**
     * @tc.name: hiSysEventPermissionJsTest005
     * @tc.desc: Test the query interface.
     * @tc.type: FUNC
     */
    it('hiSysEventPermissionJsTest005', 0, function () {
        console.info('hiSysEventPermissionJsTest005 start')
        try {
            hiSysEvent.query({
                beginTime: 0,
                endTime: 0,
                maxEvents: 100,
            }, [{
                domain: "RELIABILITY",
                names: ["STACK"],
            }], {
                onQuery: function () {},
                onComplete: function() {}
            })
        } catch (err) {
            expect(err.code == 202).assertTrue()
            console.info('hiSysEventPermissionJsTest005 end')
        }
    })
});