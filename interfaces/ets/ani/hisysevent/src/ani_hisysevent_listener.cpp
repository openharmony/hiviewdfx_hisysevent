/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_hisysevent_listener.h"
#include "hilog/log.h"
#include "hisysevent_ani_util.h"

namespace OHOS {
namespace HiviewDFX {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "ANI_HISYSEVENT_LISTENER"

namespace {
constexpr size_t ON_EVENT_PARAM_COUNT = 1;
}

AniHiSysEventListener::AniHiSysEventListener(CallbackContextAni *context)
{
    callbackContextAni = context;
    aniCallbackManager = std::make_shared<AniCallbackManager>();
}

AniHiSysEventListener::~AniHiSysEventListener()
{
    if (aniCallbackManager != nullptr) {
        aniCallbackManager->Release();
    }
    ani_env *env = nullptr;
    ani_options aniArgs {0, nullptr};
    if (ANI_OK == callbackContextAni->vm->GetEnv(ANI_VERSION_1, &env) && env != nullptr) {
        env->GlobalReference_Delete(callbackContextAni->ref);
    } else if (ANI_OK == callbackContextAni->vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env) && env != nullptr) {
        env->GlobalReference_Delete(callbackContextAni->ref);
        callbackContextAni->vm->DetachCurrentThread();
    } else {
        HILOG_WARN(LOG_CORE, "fail to delete the global reference.");
    }
    delete callbackContextAni;
}

void AniHiSysEventListener::OnEvent(const std::string& domain, const std::string& eventName, const int eventType,
    const std::string& eventDetail)
{
    aniCallbackManager->Add(callbackContextAni,
        [this, domain, eventName, eventType, eventDetail] (ani_vm *vm, ani_ref ref, pid_t threadId) {
            ani_env *env = HiSysEventAniUtil::AttachAniEnv(vm);
            ani_object sysEventInfo = nullptr;
            ani_class cls {};
            if (ANI_OK != env->FindClass(CLASS_NAME_SYSEVENTINFOANI, &cls)) {
                HILOG_ERROR(LOG_CORE, "find Class %{public}s failed", CLASS_NAME_SYSEVENTINFOANI);
                return;
            }
            ani_method ctor {};
            if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
                HILOG_ERROR(LOG_CORE, "get method %{public}s <ctor> failed", CLASS_NAME_SYSEVENTINFOANI);
                return;
            }
            if (ANI_OK != env->Object_New(cls, ctor, &sysEventInfo)) {
                HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_SYSEVENTINFOANI);
                return;
            }
            HiSysEventAniUtil::CreateHiSysEventInfoJsObject(env, eventDetail, sysEventInfo);
            ani_ref argv[ON_EVENT_PARAM_COUNT] = {sysEventInfo};
            ani_class watcherCls {};
            if (ANI_OK != env->FindClass(CLASS_NAME_WATCHERANI, &watcherCls)) {
                HILOG_ERROR(LOG_CORE, "find class failed.");
                return;
            }
            ani_ref onEvent {};
            if (ANI_OK != env->Object_GetPropertyByName_Ref(static_cast<ani_object>(ref), "onEvent", &onEvent)) {
                HILOG_ERROR(LOG_CORE, "get Object_GetPropertyByName_Ref failed.");
                return;
            }
            ani_ref result = nullptr;
            if (ANI_OK != env->FunctionalObject_Call(static_cast<ani_fn_object>(onEvent), ON_EVENT_PARAM_COUNT,
                argv, &result)) {
                HILOG_ERROR(LOG_CORE, "get FunctionalObject_Call onEvent failed.");
                return;
            }
            HiSysEventAniUtil::DetachAniEnv(vm);
        });
}

void AniHiSysEventListener::OnServiceDied()
{
    aniCallbackManager->Add(callbackContextAni,
        [this] (ani_vm *vm, ani_ref ref, pid_t threadId) {
            ani_env *env = HiSysEventAniUtil::AttachAniEnv(vm);
            ani_class watcherCls {};
            if (ANI_OK != env->FindClass(CLASS_NAME_WATCHERANI, &watcherCls)) {
                return;
            }
            ani_ref onServiceDied {};
            if (ANI_OK != env->Object_GetPropertyByName_Ref(static_cast<ani_object>(ref), "onServiceDied",
                &onServiceDied)) {
                return;
            }
            ani_ref result = nullptr;
            ani_ref argv[] = {};
            if (ANI_OK != env->FunctionalObject_Call(static_cast<ani_fn_object>(onServiceDied), 1, argv, &result)) {
                HILOG_ERROR(LOG_CORE, "get FunctionalObject_Call onServiceDied failed.");
                return;
            }
            HiSysEventAniUtil::DetachAniEnv(vm);
        });
}
} // HiviewDFX
} // OHOS
