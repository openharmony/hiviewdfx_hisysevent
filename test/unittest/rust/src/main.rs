/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

extern crate hisysevent;

use hisysevent::{EventType, HiSysEventRecord, RuleType, Watcher, WatchRule};
use hisysevent::{QueryArg, QueryRule, Querier};

const SUCCEED: i32 = 0;
const LISTENER_NOT_EXIST: i32 = -10;
const QUERY_CNT: i32 = 2;

#[test]
fn test_hisysevent_write_001() {
    let ret = hisysevent::write(
        "HIVIEWDFX",
        "PLUGIN_LOAD",
        EventType::Behavior,
        &[hisysevent::build_number_param!("INT32_SINGLE", 100i32),
            hisysevent::build_str_param!("STRING_SINGLE", "test_hisysevent_write_001"),
            hisysevent::build_bool_param!("BOOL_SINGLE", true),
            hisysevent::build_number_param!("FLOAT_SINGLE", 4.03f32),
            hisysevent::build_string_array_params!("STRING_ARRAY", &["STRING1", "STRING2"]),
            hisysevent::build_array_params!("INT32_ARRAY", &[8i32, 9i32]),
            hisysevent::build_array_params!("BOOL_ARRAY", &[true, false, true]),
            hisysevent::build_array_params!("FLOAT_ARRAY", &[1.55f32, 2.33f32, 4.88f32])]
    );
    assert!(ret == SUCCEED);
}

#[test]
fn test_hisysevent_add_remove_watcher_001() {
    let watch_rules = [
        WatchRule {
            domain: "HIVIEWDFX",
            name: "PLUGIN_LOAD",
            tag: "",
            rule_type: RuleType::WholeWord,
            event_type: EventType::Behavior,
        },
        WatchRule {
            domain: "HIVIEWDFX",
            name: "PLUGIN_UNLOAD",
            tag: "",
            rule_type: RuleType::WholeWord,
            event_type: EventType::Behavior,
        }
    ];
    // step1: construct a mut watcher.
    let watcher = Watcher::new(|record: HiSysEventRecord| {
        assert!(record.get_domain() == "HIVIEWDFX");
    }, || {
        // do nothing.
    }).expect("Construct a watcher by Watcher::new");
    // step2: add this watcher.
    let mut ret = hisysevent::add_watcher(&watcher, &watch_rules);
    assert!(ret == SUCCEED);
    ret = hisysevent::write(
        "HIVIEWDFX",
        "PLUGIN_LOAD",
        EventType::Behavior,
        &[hisysevent::build_str_param!("STRING_SINGLE", "test_hisysevent_add_remove_watcher_001")]
    );
    assert!(ret == SUCCEED);
    ret = hisysevent::write(
        "HIVIEWDFX",
        "PLUGIN_UNLOAD",
        EventType::Behavior,
        &[hisysevent::build_str_param!("STRING_SINGLE", "test_hisysevent_add_remove_watcher_001")]
    );
    assert!(ret == SUCCEED);
    // step3: remove this watcher.
    ret = hisysevent::remove_watcher(&watcher);
    assert!(ret == SUCCEED);
    // step4: recycle allocated memories of this watcher.
    watcher.try_to_recycle();
    ret = hisysevent::add_watcher(&watcher, &watch_rules);
    assert!(ret == LISTENER_NOT_EXIST);
}

#[test]
fn test_hisysevent_query_001() {
    // write two events at first.
    let mut ret = hisysevent::write(
        "HIVIEWDFX",
        "PLUGIN_LOAD",
        EventType::Behavior,
        &[hisysevent::build_str_param!("STRING_SINGLE", "test_hisysevent_query_001")]
    );
    assert!(ret == SUCCEED);
    ret = hisysevent::write(
        "HIVIEWDFX",
        "PLUGIN_UNLOAD",
        EventType::Behavior,
        &[hisysevent::build_str_param!("STRING_SINGLE", "test_hisysevent_query_001")]
    );
    assert!(ret == SUCCEED);
    // query event.
    let query_arg = QueryArg {
        begin_time: -1,
        end_time: -1,
        max_events: 2,
    };
    let query_rules = [
        QueryRule {
            domain: "HIVIEWDFX",
            event_list: vec![
                "PLUGIN_LOAD",
                "PLUGIN_UNLOAD",
            ],
            condition: "{\"version\":\"V1\",\"condition\":{\"and\":[{\"param\":\"
                NAME\",\"op\":\"=\",\"value\":\"SysEventService\"}]}}",
        },
        QueryRule {
            domain: "HIVIEWDFX",
            event_list: vec![
                "PLUGIN_LOAD",
            ],
            condition: "",
        }
    ];
    // step1: construct a querier.
    let querier = Querier::new(|records: &[HiSysEventRecord]| {
        for item in records {
            assert!(item.get_domain() == "HIVIEWDFX");
        }
    }, |reason: i32, total: i32| {
        assert!(reason == SUCCEED);
        assert!(total == QUERY_CNT);
    }).expect("Construct a querier by Querier::new");
    // step2: query.
    ret = hisysevent::query(&query_arg, &query_rules, &querier);
    assert!(ret == SUCCEED);
    // step3: recycle allocated memories of this Querier.
    querier.try_to_recycle();
    ret = hisysevent::query(&query_arg, &query_rules, &querier);
    assert!(ret == LISTENER_NOT_EXIST);
}
