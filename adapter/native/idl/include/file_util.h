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

#ifndef OHOS_HIVIEWDFX_FILE_UTILS_H
#define OHOS_HIVIEWDFX_FILE_UTILS_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
namespace FileUtil {
bool IsFileExists(const std::string& file);
bool IsFile(const std::string& file);
bool IsDirectory(const std::string& dir);
bool RemoveFile(const std::string& file);
bool RemoveDirectory(const std::string& dir);
bool ForceCreateDirectory(const std::string& dir);
std::string GetFilePathByDir(const std::string& dir, const std::string& fileName);
bool IsLegalPath(const std::string& path);
} // namespace FileUtil
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_FILE_UTILS_H