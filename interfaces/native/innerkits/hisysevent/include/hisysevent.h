/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#ifndef HI_SYS_EVENT_H
#define HI_SYS_EVENT_H
#include "hisysevent_c.h"

#ifdef __cplusplus

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "def.h"
#include "inner_writer.h"
#include "write_controller.h"

/*
 * Usage: define string macro "DOMAIN_MASKS" to disable one or more components.
 *     Method1: add macro in this header file, e.g.          #define DOMAIN_MASKS "AAFWK|APPEXECFWK|ACCOUNT"
 *     Method1: addd cflags in build.gn file, e.g.           -D DOMAIN_MASKS="AAFWK|APPEXECFWK|ACCOUNT"
 */
namespace OHOS {
namespace HiviewDFX {
// init domain masks
#ifndef DOMAIN_MASKS
#define DOMAIN_MASKS ""
#endif

static constexpr char DOMAIN_MASKS_DEF[] = DOMAIN_MASKS;

// split domain masks by '|', then compare with str
template<const std::string_view& str, const std::string_view& masks, std::string::size_type pos = 0>
struct IsMaskedImpl {
    inline static constexpr auto newpos = masks.find('|', pos);
    inline static constexpr bool value =
        IsMaskedImpl<str, masks, (newpos != std::string_view::npos) ? pos + 1 : newpos>::value ||
        (pos != newpos && pos != masks.size() &&
            masks.substr(pos, (newpos != std::string_view::npos) ? newpos - pos : str.size() - pos).compare(str) == 0);
};

template<const std::string_view& str, const std::string_view& masks>
struct IsMaskedImpl<str, masks, std::string_view::npos> {
    inline static constexpr bool value = false;
};

template<const char* domain, const char* domainMasks>
struct IsMaskedCvt {
    inline static constexpr std::string_view domainView {domain};
    inline static constexpr std::string_view domainMasksView {domainMasks};
    inline static constexpr bool value = IsMaskedImpl<domainView, domainMasksView>::value;
};

template<const char* domain>
inline static constexpr bool isMasked = IsMaskedCvt<domain, DOMAIN_MASKS_DEF>::value;

class HiSysEvent {
public:
    friend class NapiHiSysEventAdapter;

    // system event domain list
    class Domain {
    public:
        static constexpr char AAFWK[] = "AAFWK";
        static constexpr char ACCESS_TOKEN[] = "ACCESS_TOKEN";
        static constexpr char ACCESSIBILITY[] = "ACCESSIBILITY";
        static constexpr char ACCOUNT[] = "ACCOUNT";
        static constexpr char ACE[] = "ACE";
        static constexpr char AI[] = "AI";
        static constexpr char APPEXECFWK[] = "APPEXECFWK";
        static constexpr char AUDIO[] = "AUDIO";
        static constexpr char AV_SESSION[] = "AV_SESSION";
        static constexpr char AV_CODEC[] = "AV_CODEC";
        static constexpr char BACKGROUND_TASK[] = "BACKGROUND_TASK";
        static constexpr char BARRIER_FREE[] = "BARRIERFREE";
        static constexpr char BATTERY[] = "BATTERY";
        static constexpr char BIOMETRICS[] = "BIOMETRICS";
        static constexpr char BLUETOOTH[] = "BLUETOOTH";
        static constexpr char BT_SERVICE[] = "BT_SERVICE";
        static constexpr char BUNDLE_MANAGER[] = "BUNDLE_MANAGER";
        static constexpr char CAMERA[] = "CAMERA";
        static constexpr char CAST_ENGINE[] = "CASTENGINE";
        static constexpr char CCRUNTIME[] = "CCRUNTIME";
        static constexpr char CERT_MANAGER[] = "CERT_MANAGER";
        static constexpr char CODE_SIGN[] = "CODE_SIGN";
        static constexpr char COMMONEVENT[] = "COMMONEVENT";
        static constexpr char COMMUNICATION[] = "COMMUNICATION";
        static constexpr char CONTAINER[] = "CONTAINER";
        static constexpr char CUSTOMIZATION_CONFIG[] = "CUST_CONFIG";
        static constexpr char CUSTOMIZATION_EDM[] = "CUST_EDM";
        static constexpr char DEVELOPTOOLS[] = "DEVELOPTOOLS";
        static constexpr char DEVICE_AUTH[] = "DEVICE_AUTH";
        static constexpr char DEVICE_PROFILE[] = "DEVICE_PROFILE";
        static constexpr char DISPLAY[] = "DISPLAY";
        static constexpr char DISTRIBUTED_AUDIO[] = "DISTAUDIO";
        static constexpr char DISTRIBUTED_CAMERA[] = "DISTCAMERA";
        static constexpr char DISTRIBUTED_DATAMGR[] = "DISTDATAMGR";
        static constexpr char DISTRIBUTED_DEVICE_MANAGER[] = "DISTDM";
        static constexpr char DISTRIBUTED_HARDWARE_FWK[] = "DISTHWFWK";
        static constexpr char DISTRIBUTED_INPUT[] = "DISTINPUT";
        static constexpr char DISTRIBUTED_SCHEDULE[] = "DISTSCHEDULE";
        static constexpr char DISTRIBUTED_SCREEN[] = "DISTSCREEN";
        static constexpr char DLP_CRE_SERVICE[] = "DLP_CRE_SERVICE";
        static constexpr char DLP[] = "DLP";
        static constexpr char DSLM[] = "DSLM";
        static constexpr char DSOFTBUS[] = "DSOFTBUS";
        static constexpr char FILEMANAGEMENT[] = "FILEMANAGEMENT";
        static constexpr char FFRT[] = "FFRT";
        static constexpr char FORM_MANAGER[] = "FORM_MANAGER";
        static constexpr char FRAMEWORK[] = "FRAMEWORK";
        static constexpr char GLOBAL_I18N[] = "GLOBAL_I18N";
        static constexpr char GLOBAL_RESMGR[] = "GLOBAL_RESMGR";
        static constexpr char GLOBAL[] = "GLOBAL";
        static constexpr char GRAPHIC[] = "GRAPHIC";
        static constexpr char HDF_USB[] = "HDF_USB";
        static constexpr char HIVIEWDFX[] = "HIVIEWDFX";
        static constexpr char HUKS[] = "HUKS";
        static constexpr char IAWARE[] = "IAWARE";
        static constexpr char IMS_UT[] = "IMS_UT";
        static constexpr char INPUTMETHOD[] = "INPUTMETHOD";
        static constexpr char INTELLI_ACCESSORIES[] = "INTELLIACC";
        static constexpr char INTELLI_TV[] = "INTELLITV";
        static constexpr char ISHARE[] = "ISHARE";
        static constexpr char IVI_HARDWARE[] = "IVIHARDWARE";
        static constexpr char LOCATION[] = "LOCATION";
        static constexpr char MEDICAL_SENSOR[] = "MEDICAL_SENSOR";
        static constexpr char MEMMGR[] = "MEMMGR";
        static constexpr char MISCDEVICE[] = "MISCDEVICE";
        static constexpr char MSDP[] = "MSDP";
        static constexpr char MULTI_MEDIA[] = "MULTIMEDIA";
        static constexpr char MULTI_MODAL_INPUT[] = "MULTIMODALINPUT";
        static constexpr char NETMANAGER_STANDARD[] = "NETMANAGER";
        static constexpr char NFC[] = "NFC";
        static constexpr char NOTIFICATION[] = "NOTIFICATION";
        static constexpr char PASTEBOARD[] = "PASTEBOARD";
        static constexpr char POWER[] = "POWER";
        static constexpr char PUSH_MANAGER[] = "PUSH_MANAGER";
        static constexpr char RELIABILITY[] = "RELIABILITY";
        static constexpr char REQUEST[] = "REQUEST";
        static constexpr char ROUTER[] = "ROUTER";
        static constexpr char RSS[] = "RSS";
        static constexpr char SAMGR[] = "SAMGR";
        static constexpr char SECURITY_GUARD[] = "SECURITY_GUARD";
        static constexpr char SEC_COMPONENT[] = "SEC_COMPONENT";
        static constexpr char SENSOR[] = "SENSOR";
        static constexpr char SOURCE_CODE_TRANSFORMER[] = "SRCTRANSFORMER";
        static constexpr char STARTUP[] = "STARTUP";
        static constexpr char STATS[] = "STATS";
        static constexpr char TELEPHONY[] = "TELEPHONY";
        static constexpr char THEME[] = "THEME";
        static constexpr char THERMAL[] = "THERMAL";
        static constexpr char TIME[] = "TIME";
        static constexpr char UPDATE[] = "UPDATE";
        static constexpr char UPDATE_DUE[] = "UPDATE_DUE";
        static constexpr char UPDATE_OUC[] = "UPDATE_OUC";
        static constexpr char USB[] = "USB";
        static constexpr char USERIAM_FP_HAL[] = "USERIAM_FP_HAL";
        static constexpr char USERIAM_FWK[] = "USERIAM_FWK";
        static constexpr char USERIAM_PIN[] = "USERIAM_PIN";
        static constexpr char WEARABLE_HARDWARE[] = "WEARABLEHW";
        static constexpr char WEARABLE[] = "WEARABLE";
        static constexpr char WEBVIEW[] = "WEBVIEW";
        static constexpr char WINDOW_MANAGER[] = "WINDOWMANAGER";
        static constexpr char WORK_SCHEDULER[] = "WORKSCHEDULER";
        static constexpr char OTHERS[] = "OTHERS";
        static constexpr char VIR_SERVICE[] = "VIR_SERVICE";
    };

    enum EventType {
        FAULT     = 1,    // system fault event
        STATISTIC = 2,    // system statistic event
        SECURITY  = 3,    // system security event
        BEHAVIOR  = 4     // system behavior event
    };

public:
    template<typename... Types>
    static int Write(const char* func, int64_t line, const std::string &domain,
        const std::string &eventName, EventType type, Types... keyValues)
    {
        ControlParam param {
#ifdef HISYSEVENT_PERIOD
            .period = HISYSEVENT_PERIOD,
#else
            .period = HISYSEVENT_DEFAULT_PERIOD,
#endif
#ifdef HISYSEVENT_THRESHOLD
            .threshold = HISYSEVENT_THRESHOLD,
#else
            .threshold = HISYSEVENT_DEFAULT_THRESHOLD,
#endif
        };
        if (controller.CheckLimitWritingEvent(param, domain.c_str(), eventName.c_str(), func, line)) {
            return ERR_WRITE_IN_HIGH_FREQ;
        }
        return InnerWriter::InnerWrite(domain, eventName, type, keyValues...);
    }

    template<const char* domain, typename... Types, std::enable_if_t<!isMasked<domain>>* = nullptr>
    static int Write(const char* func, int64_t line, const std::string& eventName,
        EventType type, Types... keyValues)
    {
        ControlParam param {
#ifdef HISYSEVENT_PERIOD
            .period = HISYSEVENT_PERIOD,
#else
            .period = HISYSEVENT_DEFAULT_PERIOD,
#endif
#ifdef HISYSEVENT_THRESHOLD
            .threshold = HISYSEVENT_THRESHOLD,
#else
            .threshold = HISYSEVENT_DEFAULT_THRESHOLD,
#endif
        };
        if (controller.CheckLimitWritingEvent(param, domain, eventName.c_str(), func, line)) {
            return ERR_WRITE_IN_HIGH_FREQ;
        }
        return InnerWriter::InnerWrite(std::string(domain), eventName, type, keyValues...);
    }

    template<const char* domain, typename... Types, std::enable_if_t<isMasked<domain>>* = nullptr>
    inline static constexpr int Write(const char*, int64_t, const std::string&, EventType, Types...)
    {
        // do nothing
        return ERR_DOMAIN_MASKED;
    }

private:
    static WriteController controller;
};

/**
 * @brief Macro interface for writing system event.
 * @param domain      event domain.
 * @param eventName   event name.
 * @param type        event type.
 * @return 0 means success,
 *     greater than 0 also means success but with some data ignored,
 *     less than 0 means failure.
 */
#define HiSysEventWrite(domain, eventName, type, ...) \
({ \
    int hiSysEventWriteRet2023___ = OHOS::HiviewDFX::ERR_DOMAIN_MASKED; \
    if constexpr (!OHOS::HiviewDFX::isMasked<domain>) { \
        hiSysEventWriteRet2023___ = OHOS::HiviewDFX::HiSysEvent::Write<domain>(__FUNCTION__, __LINE__, \
            eventName, type, ##__VA_ARGS__); \
    } \
    hiSysEventWriteRet2023___; \
})
} // namespace HiviewDFX
} // namespace OHOS

#endif // __cplusplus
#endif // HI_SYS_EVENT_H
