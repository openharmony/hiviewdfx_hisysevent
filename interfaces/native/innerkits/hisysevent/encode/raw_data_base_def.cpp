/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "raw_data_base_def.h"

#include <vector>

namespace OHOS {
namespace HiviewDFX {
namespace Encoded {
namespace {
constexpr unsigned int DEFAULT_TZ_POS = 14; // default "+0000"

static std::vector<long> ALL_TIME_ZONES {
    3600, 7200, 10800, 11880, 14400, 18000, 21600,
    25200, 28800, 32400, 33480, 36000, 39600, 43200,
    0, -3600, -7200, -10800, -11880, -14400, 15480,
    -18000, -19080, -19620, -21600, -22680, -25200, -28800,
    -30420, -32400, -33480, -36000, -37080, -39600, -43200,
    -44820, -46800, -50400
};
}

int ParseTimeZone(long tz)
{
    int ret = DEFAULT_TZ_POS;
    for (auto iter = ALL_TIME_ZONES.begin(); iter < ALL_TIME_ZONES.end(); ++iter) {
        if (*iter == tz) {
            ret = (iter - ALL_TIME_ZONES.begin());
            break;
        }
    }
    return ret;
}
} // namespace Encoded
} // namespace HiviewDFX
} // namespace OHOS