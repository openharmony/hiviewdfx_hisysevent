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

//! Rust interfaces for crate hisysevent.

mod sys_event_manager;
mod sys_event;
mod utils;

#[macro_use]
pub mod macros;

use sys_event_manager::{OnEvent, OnServiceDied, OnQuery, OnComplete};

pub use sys_event_manager::{HiSysEventRecord, get_hisysevent_record_by_index};

pub use sys_event::{HiSysEventParam, HiSysEventParamType, HiSysEventParamValue, parse_type_len,
    build_string_arrays};

/// Enumerate system event types.
#[derive(Copy, Clone)]
pub enum EventType {
    /// Fault event.
    Fault = 1,

    /// Statistic event.
    Statistic,

    /// Security event.
    Security,

    /// System behavior event.
    Behavior,
}

/// Implementation for translating EventType into i32.
impl From<EventType> for i32 {
    fn from(src: EventType) -> i32 {
        match src {
            EventType::Fault => 1,
            EventType::Statistic => 2,
            EventType::Security => 3,
            EventType::Behavior => 4,
        }
    }
}

/// Write system event.
pub fn write<const N: usize>(event_domain: &str, event_name: &str, event_type: EventType,
    event_params: &[HiSysEventParam; N]) -> i32 {
    sys_event::write(event_domain, event_name, event_type as std::ffi::c_int, event_params)
}

/// Enumerate search system event rule type.
#[derive(Copy, Clone)]
pub enum RuleType {
    /// Whole word match.
    WholeWord = 1,

    /// Prefix match.
    Prefix,

    /// Regular match.
    Regular,
}

/// Implementation for translating RuleType into i32.
impl From<RuleType> for i32 {
    fn from(src: RuleType) -> i32 {
        match src {
            RuleType::WholeWord => 1,
            RuleType::Prefix => 2,
            RuleType::Regular => 3,
        }
    }
}

/// Definition arguments for query system event information.
#[derive(Copy, Clone)]
pub struct QueryArg {
    /// Begin time.
    pub begin_time: i64,

    /// End time.
    pub end_time: i64,

    /// Max number of receive system event.
    pub max_events: i32,
}

/// Definition event for query system event information.
pub struct QueryRule<'a> {
    /// The domain of the event.
    pub domain: &'a str,

    /// List of event name.
    pub event_list: Vec<&'a str>,

    /// extra condition for event query.
    pub condition: &'a str,
}

/// This type represent to HiSysEventQueryCallbackWrapper defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
pub struct HiSysEventQueryCallback {
    /// Handle query result, the query result will be send in several times.
    pub on_query: OnQuery,

    /// Notify querier execute query has finished.
    pub on_complete: OnComplete,
}

/// Alias for HiSysEventQueryCallback.
pub type Querier = HiSysEventQueryCallback;

/// Query system event.
pub fn query<const N: usize>(query_arg: &QueryArg, query_rules: &[QueryRule; N], querier: &Querier) -> i32 {
    sys_event_manager::query(query_arg, query_rules, querier)
}

/// Definition listener rule for system event information.
#[derive(Copy, Clone)]
pub struct WatchRule<'a> {
    /// The domain of the event.
    pub domain: &'a str,

    /// The name of the event.
    pub name: &'a str,

    /// The tag of the event.
    pub tag: &'a str,

    /// The rule of match system event.
    pub rule_type: RuleType,

    /// The type of match system event.
    pub event_type: EventType,
}

/// Definition watcher for system event information.
/// This type represent to HiSysEventWatcherWrapper defined in C.
#[repr(C)]
#[derive(Copy, Clone)]
pub struct HiSysEventWatcher {
    /// Receive system event.
    pub on_event: OnEvent,

    /// Hisysevent service shutdown.
    pub on_service_died: OnServiceDied,
}

/// Alias for HiSysEventWatcher.
pub type Watcher = HiSysEventWatcher;

/// Add watcher to watch system event.
pub fn add_watcher<const N: usize>(watcher: &Watcher, watch_rules: &[WatchRule; N]) -> i32 {
    sys_event_manager::add_watcher(watcher, watch_rules)
}

/// Remove watcher.
pub fn remove_watcher(watcher: &Watcher) -> i32 {
    sys_event_manager::remove_watcher(watcher)
}

