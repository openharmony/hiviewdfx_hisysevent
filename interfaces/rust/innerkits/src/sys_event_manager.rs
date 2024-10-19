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

use std::ffi::{CString, c_char, c_int, c_uint, c_longlong, c_ulonglong, c_void};

use crate::{EventType, WatchRule, QueryArg, QueryRule};

/// Length limit for event domain definition.
const MAX_LENGTH_OF_EVENT_DOMAIN: usize = 17;

/// Length limit for event name definition.
const MAX_LENGTH_OF_EVENT_NAME: usize = 33;

/// Length limit for event tag definition.
const MAX_LENGTH_OF_EVENT_TAG: usize = 85;

/// Length limit for timezone definition.
const MAX_LENGTH_OF_TIME_ZONE: usize = 6;

/// Length limit for event list.
const MAX_NUMBER_OF_EVENT_LIST: usize = 10;

/// Length limit for event list definition.
const MAX_EVENT_LIST_LEN: usize = 339;

/// This type represent to HiSysEventWatchRule defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
struct HiSysEventWatchRule {
    /// The domain of the event.
    pub domain: [u8; MAX_LENGTH_OF_EVENT_DOMAIN],

    /// The name of the event.
    pub name: [u8; MAX_LENGTH_OF_EVENT_NAME],

    /// The tag of the event.
    pub tag: [u8; MAX_LENGTH_OF_EVENT_TAG],

    /// The rule of match system event.
    pub rule_type: c_int,

    /// The type of match system event.
    pub event_type: c_int,
}

/// This type represent to HiSysEventQueryArg defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
struct HiSysEventQueryArg {
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
struct HiSysEventQueryRuleWrapper {
    // The domain of the event.
    pub domain: [u8; MAX_LENGTH_OF_EVENT_DOMAIN],

    /// List of event name.
    pub event_list: [u8; MAX_EVENT_LIST_LEN],

    /// Size of event name list.
    pub event_list_size: c_uint,

    /// extra condition for event query.
    pub condition: *const c_char,
}

/// This type represent to HiSysEventRecord defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
pub struct HiSysEventRecord {
    /// Domain.
    domain: [u8; MAX_LENGTH_OF_EVENT_DOMAIN],

    /// Event name.
    event_name: [u8; MAX_LENGTH_OF_EVENT_NAME],

    /// Event type.
    event_type: c_int,

    /// Timestamp.
    time: c_ulonglong,

    /// Timezone.
    tz: [u8; MAX_LENGTH_OF_TIME_ZONE],

    /// Process id.
    pid: c_longlong,

    /// Thread id.
    tid: c_longlong,

    /// User id.
    uid: c_longlong,

    /// Trace id.
    trace_id: c_ulonglong,

    /// Span id.
    spand_id: c_ulonglong,

    /// Parent span id.
    pspan_id: c_ulonglong,

    /// Trace flag.
    trace_flag: c_int,

    /// Level.
    level: *const c_char,

    /// Tag.
    tag: *const c_char,

    /// Event content.
    json_str: *const c_char,
}

impl HiSysEventRecord {

    /// Get domain name
    pub fn get_domain(&self) -> String {
        std::str::from_utf8(&self.domain).expect("need valid domain array")
            .trim_end_matches(char::from(0)).to_string()
    }

    /// Get event name
    pub fn get_event_name(&self) -> String {
        std::str::from_utf8(&self.event_name).expect("need valid event name array")
            .trim_end_matches(char::from(0)).to_string()
    }

    /// Get event name
    pub fn get_event_type(&self) -> EventType {
        match self.event_type {
            1 => EventType::Fault,
            2 => EventType::Statistic,
            3 => EventType::Security,
            4 => EventType::Behavior,
            _ => EventType::Fault,
        }
    }

    /// Get time
    pub fn get_time(&self) -> u64 {
        self.time
    }

    /// Get time zone
    pub fn get_time_zone(&self) -> String {
        std::str::from_utf8(&self.tz).expect("need valid time zone array")
            .trim_end_matches(char::from(0)).to_string()
    }

    /// Get process id
    pub fn get_pid(&self) -> i64 {
        self.pid
    }

    /// Get thread id
    pub fn get_tid(&self) -> i64 {
        self.tid
    }

    /// Get user id
    pub fn get_uid(&self) -> i64 {
        self.uid
    }

    /// Get trace id
    pub fn get_trace_id(&self) -> u64 {
        self.trace_id
    }

    /// Get span id
    pub fn get_span_id(&self) -> u64 {
        self.spand_id
    }

    /// Get parent span id
    pub fn get_parent_span_id(&self) -> u64 {
        self.pspan_id
    }

    /// Get trace flag
    pub fn get_trace_flag(&self) -> i32 {
        self.trace_flag
    }

    /// Get level
    pub fn get_level(&self) -> String {
        let level_arr = unsafe {
            std::ffi::CString::from_raw(self.level as *mut std::ffi::c_char)
        };
        std::str::from_utf8(level_arr.to_bytes()).expect("need valid level pointer")
            .trim_end_matches(char::from(0)).to_owned()
    }

    /// Get tag
    pub fn get_tag(&self) -> String {
        let tag_arr = unsafe {
            std::ffi::CString::from_raw(self.tag as *mut std::ffi::c_char)
        };
        let tag = std::str::from_utf8(tag_arr.to_bytes()).expect("need valid tag pointer")
            .trim_end_matches(char::from(0));
        String::from(tag)
    }

    /// Get json string
    pub fn get_json_str(&self) -> String {
        let json_str_arr = unsafe {
            std::ffi::CString::from_raw(self.json_str as *mut std::ffi::c_char)
        };
        let json_str = std::str::from_utf8(json_str_arr.to_bytes()).expect("need valid json str pointer")
            .trim_end_matches(char::from(0));
        String::from(json_str)
    }
}

/// Callback for handling query result, the query result will be send in several times.
pub type OnQuery = unsafe extern "C" fn (
    callback: *mut c_void,
    records: *const HiSysEventRecord,
    size: c_uint
);

/// Callback when event quering finish.
pub type OnComplete = unsafe extern "C" fn (
    callback: *mut c_void,
    reason: c_int,
    total: c_int
);

/// This type represent a rust HiSysEventRustQuerierC which like C++ HiSysEventRustQuerierC.
#[repr(C)]
struct HiSysEventRustQuerierC {
    pub on_query_cb: *mut c_void,
    pub on_query_wrapper_cb: OnQuery,
    pub on_complete_cb: *mut c_void,
    pub on_complete_wrapper_cb: OnComplete,
}

/// This type represent a rust interfaces.
#[allow(dead_code)]
pub struct Querier {
    /// Native event querier.
    native: *mut HiSysEventRustQuerierC,

    /// Customized rust callback on query.
    on_query_callback: *mut c_void,

    /// Customized rust callback on complete.
    on_complete_callback: *mut c_void,
}

impl Querier {
    /// Create a rust HiSysEventRustQuerierC object with callbacks in a querier.
    pub fn new<F1, F2>(on_query_callback: F1, on_complete_callback: F2) -> Option<Self>
    where
        F1: Fn(&[HiSysEventRecord]) + Send + Sync + 'static,
        F2: Fn(i32, i32) + Send + Sync + 'static,
    {
        let on_query_callback = Box::into_raw(Box::new(on_query_callback));
        let on_complete_callback = Box::into_raw(Box::new(on_complete_callback));
        let native = unsafe {
            CreateRustEventQuerier(on_query_callback as *mut c_void, Self::on_query_callback::<F1>,
                on_complete_callback as *mut c_void, Self::on_complete_callback::<F2>)
        };
        if native.is_null() {
            None
        } else {
            Some(Self {
                native,
                on_query_callback: on_query_callback as *mut c_void,
                on_complete_callback: on_complete_callback as *mut c_void,
            })
        }
    }

    /// Callback for handling query result, the query result will be send in several times.
    ///
    /// # Safety
    ///
    /// The callback parameter will be kept valid during native
    /// HiSysEventRustQuerierC object lifetime.
    ///
    unsafe extern "C" fn on_query_callback<F>(callback: *mut c_void, records: *const HiSysEventRecord,
        size: c_uint)
    where
        F: Fn(&[HiSysEventRecord]) + Send + Sync + 'static,
    {
        let mut transalted_records: Vec<HiSysEventRecord> = vec![];
        for i in 0..size {
            let record_item = unsafe {
                GetHiSysEventRecordByIndexWrapper(records, size, i as c_uint)
            };
            transalted_records.push(record_item);
        }
        let callback = (callback as *const F).as_ref().unwrap();
        callback(transalted_records.as_slice());
    }

    /// Callback when event quering finish.
    ///
    /// # Safety
    ///
    /// The callback parameter will be kept valid during native
    /// HiSysEventRustQuerierC object lifetime.
    ///
    unsafe extern "C" fn on_complete_callback<F>(callback: *mut c_void, reason: c_int,
        total: c_int)
    where
        F: Fn(i32, i32) + Send + Sync + 'static,
    {
        let callback = (callback as *const F).as_ref().unwrap();
        callback(reason, total);
    }

    /// free memories allocated for Querier
    pub fn try_to_recycle(&self) {
        unsafe {
            RecycleRustEventQuerier(self.as_raw())
        }
    }
}

/// Implement trait AsRawPtr for Querier
///
/// # Safety
///
/// A `Querier` is always constructed with a valid raw pointer
/// to a `HiSysEventRustQuerierC`.
///
unsafe impl AsRawPtr<HiSysEventRustQuerierC> for Querier {
    fn as_raw(&self) -> *const HiSysEventRustQuerierC {
        self.native
    }

    fn as_mut_raw(&mut self) -> *mut HiSysEventRustQuerierC {
        self.native
    }
}

/// Query system event.
pub(crate) fn query(query_arg: &QueryArg, query_rules: &[QueryRule], querier: &Querier) -> i32 {
    let query_arg_wrapper = HiSysEventQueryArg {
        begin_time: query_arg.begin_time as c_longlong,
        end_time: query_arg.end_time as c_longlong,
        max_events: query_arg.max_events as c_int,
    };
    let mut query_rules_wrapper: Vec<HiSysEventQueryRuleWrapper> = vec![];
    for i in 0..query_rules.len() {
        let condition_wrapper = CString::new(query_rules[i].condition).expect("Need a condition for query.");
        query_rules_wrapper.push(HiSysEventQueryRuleWrapper {
            domain: [0; MAX_LENGTH_OF_EVENT_DOMAIN],
            event_list: [0; MAX_EVENT_LIST_LEN],
            event_list_size: MAX_NUMBER_OF_EVENT_LIST as c_uint,
            condition: condition_wrapper.as_ptr() as *const c_char,
        });
        crate::utils::trans_slice_to_array(query_rules[i].domain, &mut query_rules_wrapper[i].domain);
        let src_len = query_rules[i].event_list.len();
        let dest_len = query_rules_wrapper[i].event_list.len();
        let total_cnt = if src_len <= dest_len {
            src_len
        } else {
            dest_len
        };
        query_rules_wrapper[i].event_list_size = total_cnt as c_uint;
        let src_str = query_rules[i].event_list.join("|");
        let src_str = &src_str[..];
        crate::utils::trans_slice_to_array(src_str, &mut query_rules_wrapper[i].event_list);
    }
    // Safty: call C ffi border function, all risks are under control.
    unsafe {
        HiSysEventQueryWrapper(&query_arg_wrapper as *const HiSysEventQueryArg,
            query_rules_wrapper.as_mut_ptr(),
            query_rules.len() as c_uint,
            querier.as_raw(),
        )
    }
}

/// Callback when receive system event.
pub type OnEvent = unsafe extern "C" fn (
    callback: *mut c_void,
    record: HiSysEventRecord
);

/// Callback when hisysevent service shutdown.
pub type OnServiceDied = unsafe extern "C" fn (
    callback: *mut c_void,
);

/// This type represent a rust HiSysEventWatcher which like C++ HiSysEventWatcher.
#[repr(C)]
struct HiSysEventRustWatcherC {
    pub on_event_cb: *mut c_void,
    pub on_event_wrapper_cb: OnEvent,
    pub on_service_died_cb: *mut c_void,
    pub on_service_died_wrapper_cb: OnServiceDied,
}

/// This type represent a rust interfaces.
#[allow(dead_code)]
pub struct Watcher {
    /// Native event listener instance
    native: *mut HiSysEventRustWatcherC,

    /// Customized rust callback on event
    on_event_callback: *mut c_void,

    /// Customized rust callback on service died
    on_service_died_callback: *mut c_void,
}

impl Watcher {
    /// Create a rust HiSysEventRustWatcherC object with callbacks in a watcher.
    pub fn new<F1, F2>(on_event_callback: F1, on_service_died_callback: F2) -> Option<Self>
    where
        F1: Fn(HiSysEventRecord) + Send + Sync + 'static,
        F2: Fn() + Send + Sync + 'static,
    {
        let on_event_callback = Box::into_raw(Box::new(on_event_callback));
        let on_service_died_callback = Box::into_raw(Box::new(on_service_died_callback));
        let native = unsafe {
            CreateRustEventWatcher(on_event_callback as *mut c_void, Self::on_event::<F1>,
                on_service_died_callback as *mut c_void, Self::on_service_died::<F2>)
        };
        if native.is_null() {
            None
        } else {
            Some(Self {
                native,
                on_event_callback: on_event_callback as *mut c_void,
                on_service_died_callback: on_service_died_callback as *mut c_void,
            })
        }
    }

    /// Callback when receive system event.
    ///
    /// # Safety
    ///
    /// The callback parameter will be kept valid during native
    /// HiSysEventRustWatcherC object lifetime.
    ///
    unsafe extern "C" fn on_event<F>(callback: *mut c_void, record: HiSysEventRecord)
    where
        F: Fn(HiSysEventRecord) + Send + Sync + 'static,
    {
        let callback = (callback as *const F).as_ref().unwrap();
        callback(record);
    }

    /// Callback when hisysevent service shutdown.
    ///
    /// # Safety
    ///
    /// The callback parameter will be kept valid during native
    /// HiSysEventRustWatcherC object lifetime.
    ///
    unsafe extern "C" fn on_service_died<F>(callback: *mut c_void)
    where
        F: Fn() + Send + Sync + 'static,
    {
        let callback = (callback as *const F).as_ref().unwrap();
        callback();
    }

    /// free memories allocated for Watcher
    pub fn try_to_recycle(&self) {
        unsafe {
            RecycleRustEventWatcher(self.as_raw())
        }
    }
}

/// Implement trait AsRawPtr for Watcher
///
/// # Safety
///
/// A `Watcher` is always constructed with a valid raw pointer
/// to a `HiSysEventRustWatcherC`.
///
unsafe impl AsRawPtr<HiSysEventRustWatcherC> for Watcher {
    fn as_raw(&self) -> *const HiSysEventRustWatcherC {
        self.native
    }

    fn as_mut_raw(&mut self) -> *mut HiSysEventRustWatcherC {
        self.native
    }
}

/// Add watcher to watch system event.
pub(crate) fn add_watcher(watcher: &Watcher, watch_rules: &[WatchRule]) -> i32 {
    let mut watch_rules_wrapper: Vec<HiSysEventWatchRule> = vec![];
    for i in 0..watch_rules.len() {
        watch_rules_wrapper.push(HiSysEventWatchRule {
            domain: [0; MAX_LENGTH_OF_EVENT_DOMAIN],
            name: [0; MAX_LENGTH_OF_EVENT_NAME],
            tag: [0; MAX_LENGTH_OF_EVENT_TAG],
            rule_type: 0,
            event_type: 0,
        });
        crate::utils::trans_slice_to_array(watch_rules[i].domain, &mut watch_rules_wrapper[i].domain);
        crate::utils::trans_slice_to_array(watch_rules[i].name, &mut watch_rules_wrapper[i].name);
        crate::utils::trans_slice_to_array(watch_rules[i].tag, &mut watch_rules_wrapper[i].tag);
        watch_rules_wrapper[i].rule_type = watch_rules[i].rule_type as i32 as c_int;
        watch_rules_wrapper[i].event_type = watch_rules[i].event_type as i32 as c_int;
    }
    // Safty: call C ffi border function, all risks are under control.
    unsafe {
        HiSysEventAddWatcherWrapper(watcher.as_raw(),
            watch_rules_wrapper.as_mut_ptr(),
            watch_rules.len() as c_uint)
    }
}

/// Remove watcher.
pub(crate) fn remove_watcher(watcher: &Watcher) -> i32 {
    // Safty: call C ffi border function, all risks are under control.
    unsafe {
        HiSysEventRemoveWatcherWrapper(watcher.as_raw())
    }
}

/// Trait for transparent Rust wrappers around native raw pointer types.
///
/// # Safety
///
/// The pointer return by this trait's methods should be immediately passed to
/// native and not stored by Rust. The pointer is valid only as long as the
/// underlying native object is alive, so users must be careful to take this into
/// account, as Rust cannot enforce this.
///
/// For this trait to be a correct implementation, `T` must be a valid native
/// type. Since we cannot constrain this via the type system, this trait is
/// marked as unsafe.
///
unsafe trait AsRawPtr<T> {
    /// Return a pointer to the native version of `self`
    fn as_raw(&self) -> *const T;

    /// Return a mutable pointer to the native version of `self`
    fn as_mut_raw(&mut self) -> *mut T;
}

extern "C" {
    /// ffi border function.
    fn HiSysEventAddWatcherWrapper(watcher: *const HiSysEventRustWatcherC, rules: *const HiSysEventWatchRule,
        rule_size: c_uint) -> c_int;

    /// ffi border function.
    fn HiSysEventRemoveWatcherWrapper(watcher: *const HiSysEventRustWatcherC) -> c_int;

    /// ffi border function.
    fn HiSysEventQueryWrapper(query_arg: *const HiSysEventQueryArg, rules: *const HiSysEventQueryRuleWrapper,
        rule_size: c_uint, querier: *const HiSysEventRustQuerierC) -> c_int;

    /// ffi border function.
    fn GetHiSysEventRecordByIndexWrapper(records: *const HiSysEventRecord, total: c_uint,
        index: c_uint) -> HiSysEventRecord;

    /// ffi border function.
    fn CreateRustEventWatcher(on_event_callback: *const c_void,
        on_event_wrapper_callback: OnEvent,
        on_service_died_callback: *const c_void,
        on_service_died_wrapper_callback: OnServiceDied) -> *mut HiSysEventRustWatcherC;

    /// ffi border function.
    fn RecycleRustEventWatcher(watcher: *const HiSysEventRustWatcherC);

    /// ffi border function.
    fn CreateRustEventQuerier(on_query_callback: *const c_void,
        on_query_wrapper_callback: OnQuery,
        on_complete_callback: *const c_void,
        on_complete_wrapper_callback: OnComplete) -> *mut HiSysEventRustQuerierC;

    /// ffi border function.
    fn RecycleRustEventQuerier(querier: *const HiSysEventRustQuerierC);
}
