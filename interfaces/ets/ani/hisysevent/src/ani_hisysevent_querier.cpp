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

#include "ani_hisysevent_querier.h"
#include "hilog/log.h"
#include "hisysevent_ani_util.h"
namespace OHOS {
namespace HiviewDFX {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D08

#undef LOG_TAG
#define LOG_TAG "ANI_HISYSEVENT_QUERIER"

namespace {
constexpr size_t ON_QUERY_PARAM_COUNT = 1;
constexpr size_t ON_QUERY_COMPLTE_COUNT = 3;
}

AniHiSysEventQuerier::AniHiSysEventQuerier(CallbackContextAni *context, ON_COMPLETE_FUNC handler)
{
    callbackContextAni = context;
    onCompleteHandler = handler;
    aniCallbackManager = std::make_shared<AniCallbackManager>();
}

AniHiSysEventQuerier::~AniHiSysEventQuerier()
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

void AniHiSysEventQuerier::OnQuery(const std::vector<std::string>& sysEvents,
    const std::vector<int64_t>& seq)
{
    aniCallbackManager->Add(callbackContextAni,
        [this, sysEvents, seq] (ani_vm *vm, ani_ref ref, pid_t threadId) {
            ani_env *env = HiSysEventAniUtil::AttachAniEnv(vm);
            if (env == nullptr) {
                return;
            }
            ani_array sysEventInfoJsArray = nullptr;
            ani_class cls {};
            if (ANI_OK != env->FindClass(CLASS_NAME_SYSEVENTINFOANI, &cls)) {
                HILOG_ERROR(LOG_CORE, "find Class %{public}s failed", CLASS_NAME_SYSEVENTINFOANI);
                return;
            }
            if (ANI_OK != env->Array_New(sysEvents.size(), nullptr, &sysEventInfoJsArray)) {
                HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_SYSEVENTINFOANI);
                return;
            }
            HiSysEventAniUtil::CreateJsSysEventInfoArray(env, sysEvents, sysEventInfoJsArray);
            ani_ref argv[ON_QUERY_PARAM_COUNT] = {sysEventInfoJsArray};
            ani_class querierCls {};
            if (ANI_OK != env->FindClass(CLASS_NAME_QUERIERANI, &querierCls)) {
                return;
            }
            ani_ref onQuery {};
            if (ANI_OK != env->Object_GetPropertyByName_Ref(static_cast<ani_object>(ref), "onQuery", &onQuery)) {
                HILOG_ERROR(LOG_CORE, "get onQuery failed.");
                return;
            }
            ani_ref result = nullptr;
            if (ANI_OK != env->FunctionalObject_Call(static_cast<ani_fn_object>(onQuery), ON_QUERY_PARAM_COUNT,
                argv, &result)) {
                HILOG_ERROR(LOG_CORE, "failed to call OnQuery function.");
                return;
            }
            HiSysEventAniUtil::DetachAniEnv(vm);
        });
}

void AniHiSysEventQuerier::OnComplete(int32_t reason, int32_t total, int64_t seq)
{
    aniCallbackManager->Add(callbackContextAni,
        [this, reason, total, seq] (ani_vm *vm, ani_ref ref, pid_t threadId) {
            ani_env *env = HiSysEventAniUtil::AttachAniEnv(vm);
            if (env == nullptr) {
                return;
            }
            ani_object reasonJsParam = nullptr;
            reasonJsParam = HiSysEventAniUtil::CreateDoubleInt32(env, reason);
            ani_object totalJsParam = nullptr;
            totalJsParam = HiSysEventAniUtil::CreateDoubleInt32(env, total);
            ani_object seqJsParm = nullptr;
            seqJsParm = HiSysEventAniUtil::CreateDoubleInt64(env, seq);
            ani_ref argv[ON_QUERY_COMPLTE_COUNT] = {reasonJsParam, totalJsParam, seqJsParm};
            ani_class querierCls {};
            if (ANI_OK != env->FindClass(CLASS_NAME_QUERIERANI, &querierCls)) {
                return;
            }
            ani_ref onComplete {};
            if (ANI_OK != env->Object_GetPropertyByName_Ref(static_cast<ani_object>(ref), "onComplete", &onComplete)) {
                HILOG_ERROR(LOG_CORE, "get onQuery failed.");
                return;
            }
            ani_ref result = nullptr;
            if (ANI_OK != env->FunctionalObject_Call(static_cast<ani_fn_object>(onComplete), ON_QUERY_COMPLTE_COUNT,
                argv, &result)) {
                HILOG_ERROR(LOG_CORE, "failed to call OnComplete function.");
                return;
            }
            HiSysEventAniUtil::DetachAniEnv(vm);
        }, [this] (ani_vm *vm, pid_t threadId) {
            if (this->onCompleteHandler != nullptr && this->callbackContextAni != nullptr) {
                this->onCompleteHandler(vm, this->callbackContextAni->ref);
            }
        });
}
} // HiviewDFX
} // OHOS
