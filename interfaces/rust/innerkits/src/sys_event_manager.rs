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

use std::ffi::{CString, c_char, c_int, c_uint, c_longlong, c_ulonglong};

use crate::{WatchRule, Watcher, QueryArg, QueryRule, Querier};

/// Length limit for event domain definition.
pub(crate) const MAX_LENGTH_OF_EVENT_DOMAIN: usize = 17;

/// Length limit for event name definition.
pub(crate) const MAX_LENGTH_OF_EVENT_NAME: usize = 33;

/// Length limit for event tag definition.
pub(crate) const MAX_LENGTH_OF_EVENT_TAG: usize = 85;

/// Length limit for timezone definition.
pub(crate) const MAX_LENGTH_OF_TIME_ZONE: usize = 6;

/// Length limit for event list.
pub(crate) const MAX_NUMBER_OF_EVENT_LIST: usize = 10;

/// Length limit for event list definition.
pub(crate) const MAX_EVENT_LIST_LEN: usize = 594;

/// This type represent to HiSysEventWatchRule defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
pub struct HiSysEventWatchRule {
    /// The domain of the event.
    pub domain: [c_char; MAX_LENGTH_OF_EVENT_DOMAIN],

    /// The name of the event.
    pub name: [c_char; MAX_LENGTH_OF_EVENT_NAME],

    /// The tag of the event.
    pub tag: [c_char; MAX_LENGTH_OF_EVENT_TAG],

    /// The rule of match system event.
    pub rule_type: c_int,

    /// The type of match system event.
    pub event_type: c_int,
}

/// This type represent to HiSysEventQueryArg defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
pub struct HiSysEventQueryArg {
    /// Begin time.
    pub begin_time: c_longlong,

    /// End time.
    pub end_time: c_longlong,

    /// Max number of receive system event.
    pub max_events: c_int,
}

/// This type represent to HiSysEventQueryRuleWrapper defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
pub struct HiSysEventQueryRuleWrapper {
    // The domain of the event.
    pub domain: [c_char; MAX_LENGTH_OF_EVENT_DOMAIN],

    /// List of event name.
    pub event_list: [c_char; MAX_EVENT_LIST_LEN],

    /// Size of event name list.
    pub event_list_size: c_int,

    /// extra condition for event query.
    pub condition: *const c_char,
}

/// Translate WatchRule which defined in rust into HiSysEventWatchRule which defined in C.
pub(crate) fn convert_watch_rule_to_wrapper<const N: usize>(src: &[WatchRule; N], dest: &mut [HiSysEventWatchRule; N]) {
    for i in 0..N {
        crate::utils::trans_slice_to_array(src[i].domain, &mut dest[i].domain);
        crate::utils::trans_slice_to_array(src[i].name, &mut dest[i].name);
        crate::utils::trans_slice_to_array(src[i].tag, &mut dest[i].tag);
        dest[i].rule_type = src[i].rule_type as i32 as c_int;
        dest[i].event_type = src[i].event_type as i32 as c_int;
    }
}

/// Translate Vector<&Str> to [c_char; N].
pub(crate) fn convert_str_vec_to_array<const N: usize>(src: &Vec<&str>, dest: &mut [c_char; N],
    len: &mut c_int) {
    let total_cnt = if src.len() <= N {
        src.len()
    } else {
        N
    };
    *len = total_cnt as c_int;
    let src_str = src.join("|");
    let src_str = &src_str[..];
    crate::utils::trans_slice_to_array(src_str, dest);
}

/// Translate QueryRule which defined in rust into HiSysEventQueryRuleWrapper which defined in C.
pub(crate) fn convert_query_rule_to_wrapper<const N: usize>(src: &[QueryRule; N],
    dest: &mut [HiSysEventQueryRuleWrapper; N]) {
    for i in 0..N {
        crate::utils::trans_slice_to_array(src[i].domain, &mut dest[i].domain);
        convert_str_vec_to_array(&src[i].event_list, &mut dest[i].event_list, &mut dest[i].event_list_size);
        let condition_wrapper = CString::new(src[i].condition).unwrap();
        dest[i].condition = condition_wrapper.into_raw() as *const std::ffi::c_char;
    }
}

/// This type represent to HiSysEventRecord defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
pub struct HiSysEventRecord {
    /// Domain.
    pub domain: [c_char; MAX_LENGTH_OF_EVENT_DOMAIN],

    /// Event name.
    pub event_name: [c_char; MAX_LENGTH_OF_EVENT_NAME],

    /// Event type.
    pub type_: c_int,

    /// Timestamp.
    pub time: c_ulonglong,

    /// Timezone.
    pub tz: [c_char; MAX_LENGTH_OF_TIME_ZONE],

    /// Process id.
    pub pid: c_longlong,

    /// Thread id.
    pub tid: c_longlong,

    /// User id.
    pub uid: c_longlong,

    /// Trace id.
    pub trace_id: c_ulonglong,

    /// Span id.
    pub spand_id: c_ulonglong,

    /// Parent span id.
    pub pspan_id: c_ulonglong,

    /// Trace flag.
    pub trace_flag: c_int,

    /// Level.
    pub level: *const c_char,

    /// Tag.
    pub tag: *const c_char,

    /// Event content.
    pub json_str: *const c_char,
}

/// Callback for handling query result, the query result will be send in several times.
pub type OnQuery = unsafe extern "C" fn (records: *const HiSysEventRecord, size: c_uint);

/// Callback when event quering finish.
pub type OnComplete = unsafe extern "C" fn (reason: c_int, total: c_int);

/// Query system event.
pub fn query<const N: usize>(query_arg: &QueryArg, query_rules: &[QueryRule; N], querier: &Querier) -> i32 {
    let query_arg_wrapper = HiSysEventQueryArg {
        begin_time: query_arg.begin_time as c_longlong,
        end_time: query_arg.end_time as c_longlong,
        max_events: query_arg.max_events as c_int,
    };
    unsafe {
        let init_wrapper = CString::new("").unwrap();
        let mut query_rules_wrapper = [
            HiSysEventQueryRuleWrapper {
                domain: [0; MAX_LENGTH_OF_EVENT_DOMAIN],
                event_list: [0; MAX_EVENT_LIST_LEN],
                event_list_size: MAX_NUMBER_OF_EVENT_LIST as c_int,
                condition: init_wrapper.as_ptr() as *const c_char,
            }; N
        ];
        convert_query_rule_to_wrapper(query_rules, &mut query_rules_wrapper);
        let querier_dest = std::boxed::Box::<Querier>::new(*querier);
        HiSysEventQueryWrapper(&query_arg_wrapper as *const HiSysEventQueryArg,
            &query_rules_wrapper as *const [HiSysEventQueryRuleWrapper; N] as *const HiSysEventQueryRuleWrapper,
            N as c_uint,
            std::boxed::Box::<Querier>::into_raw(querier_dest) as *const Querier
        )
    }
}

/// Callback when receive system event.
pub type OnEvent = unsafe extern "C" fn (record: HiSysEventRecord);

/// Callback when hisysevent service shutdown.
pub type OnServiceDied = unsafe extern "C" fn ();

/// Add watcher to watch system event.
pub fn add_watcher<const N: usize>(watcher: &Watcher, watch_rules: &[WatchRule; N]) -> i32 {
    unsafe {
        let mut watch_rules_wrapper = [
            HiSysEventWatchRule {
                domain: [0; MAX_LENGTH_OF_EVENT_DOMAIN],
                name: [0; MAX_LENGTH_OF_EVENT_NAME],
                tag: [0; MAX_LENGTH_OF_EVENT_TAG],
                rule_type: 0,
                event_type: 0,
            }; N
        ];
        convert_watch_rule_to_wrapper(watch_rules, &mut watch_rules_wrapper);
        let dest = std::boxed::Box::<Watcher>::new(*watcher);
        HiSysEventAddWatcherWrapper(std::boxed::Box::<Watcher>::into_raw(dest) as *const Watcher,
            &watch_rules_wrapper as *const [HiSysEventWatchRule; N] as *const HiSysEventWatchRule,
            N as c_uint)
    }
}

/// Remove watcher.
pub fn remove_watcher(watcher: &Watcher) -> i32 {
    unsafe {
        HiSysEventRemoveWatcherWrapper(watcher as *const Watcher)
    }
}

/// Get HiSysEventRecord from record array by index.
///
/// # Safety
///
pub unsafe fn get_hisysevent_record_by_index(records: *const HiSysEventRecord, total: i32,
    index: i32) -> HiSysEventRecord {
    GetHiSysEventRecordByIndexWrapper(records, total as std::ffi::c_int, index as std::ffi::c_int)
}

extern "C" {
    /// ffi border function.
    pub fn HiSysEventAddWatcherWrapper(watcher: *const Watcher, rules: *const HiSysEventWatchRule,
        rule_size: c_uint) -> c_int;

    /// ffi border function.
    pub fn HiSysEventRemoveWatcherWrapper(watcher: *const Watcher) -> c_int;

    /// ffi border function.
    pub fn HiSysEventQueryWrapper(query_arg: *const HiSysEventQueryArg, rules: *const HiSysEventQueryRuleWrapper,
        rule_size: c_uint, querier: *const Querier) -> c_int;

    /// ffi border function.
    pub fn GetHiSysEventRecordByIndexWrapper(records: *const HiSysEventRecord, total: c_int,
        index: c_int) -> HiSysEventRecord;
}