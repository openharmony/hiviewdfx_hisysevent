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

//! Rust macros defined in crate hisysevent.

/// As macro __FUNC__ defined in C/C++.
///
/// # Example
///
/// ```
/// extern crate hisysevent;
///
/// let func_name = hisysevent::function!();
///
/// ```
///
#[macro_export]
macro_rules! function {
    () => {{
        fn hook() {}
        fn type_name_of<T>(_: T) -> &'static str {
            std::any::type_name::<T>()
        }
        let name = type_name_of(hook);
        let off_set: usize = 6; // ::hook
        &name[..name.len() - off_set]
    }}
}

/// Build HiSysEventParamType with different type.
#[macro_export]
macro_rules! build_param_type {
    ($param_type:expr, $param_len:expr) => {
        match $param_type as &str {
            "bool" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::BoolArray
                } else {
                    hisysevent::HiSysEventParamType::Bool
                }
            }
            "i8" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::Int8Array
                } else {
                    hisysevent::HiSysEventParamType::Int8
                }
            }
            "u8" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::Uint8Array
                } else {
                    hisysevent::HiSysEventParamType::Uint8
                }
            }
            "i16" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::Int16Array
                } else {
                    hisysevent::HiSysEventParamType::Int16
                }
            }
            "u16" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::Uint16Array
                } else {
                    hisysevent::HiSysEventParamType::Uint16
                }
            }
            "i32" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::Int32Array
                } else {
                    hisysevent::HiSysEventParamType::Int32
                }
            }
            "u32" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::Uint32Array
                } else {
                    hisysevent::HiSysEventParamType::Uint32
                }
            }
            "i64" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::Int64Array
                } else {
                    hisysevent::HiSysEventParamType::Int64
                }
            }
            "u64" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::Uint64Array
                } else {
                    hisysevent::HiSysEventParamType::Uint64
                }
            }
            "f32" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::FloatArray
                } else {
                    hisysevent::HiSysEventParamType::Float
                }
            }
            "f64" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::DoubleArray
                } else {
                    hisysevent::HiSysEventParamType::Double
                }
            }
            "str" => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::ParamTypeStringArray
                } else {
                    hisysevent::HiSysEventParamType::ParamTypeString
                }
            }
            _ => {
                if $param_len > 0 {
                    hisysevent::HiSysEventParamType::BoolArray
                } else {
                    hisysevent::HiSysEventParamType::Bool
                }
            }
        }
    }
}

/// Build array consist of HiSysEventParamValue with different type except string.
#[macro_export]
macro_rules! build_array_params {
    ($param_name_:expr, $param_value_:expr) => {
        {
            let (param_type, param_len) = hisysevent::parse_type_len($param_value_);
            hisysevent::HiSysEventParam {
                param_name: $param_name_,
                param_type: $crate::build_param_type!(param_type, param_len),
                param_value: hisysevent::HiSysEventParamValue {
                    void_ptr_: $param_value_.as_ptr() as *const std::ffi::c_int as *const (),
                },
                array_size: param_len,
            }
        }
    }
}

/// Build array consist of HiSysEventParamValue with string type.
#[macro_export]
macro_rules! build_string_array_params {
    ($param_name_:expr, $param_value_:expr) => {
        hisysevent::build_string_arrays($param_name_, $param_value_)
    }
}

/// Build HiSysEventParamValue with different type except string and bool.
#[macro_export]
macro_rules! build_param_value {
    ($param_type:expr, $val:expr) => {
        match $param_type as &str {
            "i8" => hisysevent::HiSysEventParamValue {
                i8_: $val as i8,
            },
            "u8" => hisysevent::HiSysEventParamValue {
                u8_: $val as u8,
            },
            "i16" => hisysevent::HiSysEventParamValue {
                i16_: $val as i16,
            },
            "u16" => hisysevent::HiSysEventParamValue {
                u16_: $val as u16,
            },
            "i32" => hisysevent::HiSysEventParamValue {
                i32_: $val as i32,
            },
            "u32" => hisysevent::HiSysEventParamValue {
                u32_: $val as u32,
            },
            "i64" => hisysevent::HiSysEventParamValue {
                i64_: $val as i64,
            },
            "u64" => hisysevent::HiSysEventParamValue {
                u64_: $val as u64,
            },
            "f32" => hisysevent::HiSysEventParamValue {
                f32_: $val as f32,
            },
            "f64" => hisysevent::HiSysEventParamValue {
                f64_: $val as f64,
            },
            _ => hisysevent::HiSysEventParamValue {
                b_: false,
            },
        }
    }
}

/// Build HiSysEventParamValue with any number type.
#[macro_export]
macro_rules! build_number_param {
    ($param_name_:expr, $param_value:expr) => {
        {
            let (param_type, param_len) = hisysevent::parse_type_len($param_value);
            hisysevent::HiSysEventParam {
                param_name: $param_name_,
                param_type: $crate::build_param_type!(param_type, param_len),
                param_value: $crate::build_param_value!(param_type, $param_value),
                array_size: param_len,
            }
        }
    }
}

/// Build HiSysEventParamValue with bool type.
#[macro_export]
macro_rules! build_bool_param {
    ($param_name_:expr, $param_value:expr) => {
        {
            let (param_type, param_len) = hisysevent::parse_type_len($param_value);
            hisysevent::HiSysEventParam {
                param_name: $param_name_,
                param_type: hisysevent::HiSysEventParamType::Bool,
                param_value: hisysevent::HiSysEventParamValue {
                    b_: $param_value,
                },
                array_size: param_len,
            }
        }
    }
}

/// Build HiSysEventParamValue with string type.
#[macro_export]
macro_rules! build_str_param {
    ($param_name_:expr, $param_value_:expr) => {
        {
            let (param_type, param_len) = hisysevent::parse_type_len($param_value_);
            hisysevent::HiSysEventParam {
                param_name: $param_name_,
                param_type: hisysevent::HiSysEventParamType::ParamTypeString,
                param_value: {
                    let str_wrapper = std::ffi::CString::new($param_value_).unwrap();
                    hisysevent::HiSysEventParamValue {
                        char_ptr_: str_wrapper.into_raw() as *const std::ffi::c_char,
                    }
                },
                array_size: param_len,
            }
        }
    }
}
