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

use std::ffi::{c_int, c_uint};
use hisysevent::{EventType, HiSysEventRecord, QueryRule, Querier, QueryArg, RuleType, Watcher, WatchRule};

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
            hisysevent::build_string_array_params!("STRING_ARRAY", ["STRING1", "STRING2"]),
            hisysevent::build_array_params!("INT32_ARRAY", [8i32, 9i32]),
            hisysevent::build_array_params!("BOOL_ARRAY", [true, false, true]),
            hisysevent::build_array_params!("FLOAT_ARRAY", [1.55f32, 2.33f32, 4.88f32])]
    );
    assert!(ret == 0);
}

///
#[allow(dead_code)]
unsafe extern "C" fn on_query_callback(_records: *const HiSysEventRecord, _size: c_uint) {
}

///
#[allow(dead_code)]
unsafe extern "C" fn on_complete_callback(_reason: c_int, _total: c_int) {
}

///
#[allow(dead_code)]
unsafe extern "C" fn on_event_callback(_record: HiSysEventRecord) {
}

///
#[allow(dead_code)]
unsafe extern "C" fn on_service_died_callback() {
}

#[test]
fn test_hisysevent_add_remove_watcher_001() {
    let watcher = Watcher {
        on_event: on_event_callback,
        on_service_died: on_service_died_callback,
    };
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
    let mut ret = hisysevent::add_watcher(&watcher, &watch_rules);
    assert!(ret == 0);
    ret = hisysevent::write(
        "HIVIEWDFX",
        "PLUGIN_LOAD",
        EventType::Behavior,
        &[hisysevent::build_str_param!("STRING_SINGLE", "test_hisysevent_add_remove_watcher_001")]
    );
    assert!(ret == 0);
    ret = hisysevent::write(
        "HIVIEWDFX",
        "PLUGIN_UNLOAD",
        EventType::Behavior,
        &[hisysevent::build_str_param!("STRING_SINGLE", "test_hisysevent_add_remove_watcher_001")]
    );
    assert!(ret == 0);
    ret = hisysevent::remove_watcher(&watcher);
    assert!(ret == 0);
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
    assert!(ret == 0);
    ret = hisysevent::write(
        "HIVIEWDFX",
        "PLUGIN_UNLOAD",
        EventType::Behavior,
        &[hisysevent::build_str_param!("STRING_SINGLE", "test_hisysevent_query_001")]
    );
    assert!(ret == 0);
    // query event
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
            condition: "{\"version\":\"V1\",\"condition\":{\"and\":[{\"param\":\"NAME\",\"op\":\"=\",\"value\":\"SysEventService\"}]}}",
        },
        QueryRule {
            domain: "HIVIEWDFX",
            event_list: vec![
                "PLUGIN_LOAD",
            ],
            condition: "",
        }
    ];
    let querier = Querier {
        on_query: on_query_callback,
        on_complete: on_complete_callback,
    };
    ret = hisysevent::query(&query_arg, &query_rules, &querier);
    assert!(ret == 0);
}