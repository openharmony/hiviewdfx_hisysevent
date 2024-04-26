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
#include "file_util.h"

#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace OHOS {
namespace HiviewDFX {
namespace FileUtil {
namespace {
const char PATH_DELIMITER = '/';
}
bool IsFileExists(const std::string& file)
{
    return access(file.c_str(), F_OK) == 0;
}

bool IsFile(const std::string& file)
{
    struct stat statBuf {};
    return lstat(file.c_str(), &statBuf) == 0 ? S_ISREG(statBuf.st_mode) : false;
}

bool IsDirectory(const std::string& dir)
{
    struct stat statBuf {};
    return lstat(dir.c_str(), &statBuf) == 0 ? S_ISDIR(statBuf.st_mode) : false;
}

bool RemoveFile(const std::string& file)
{
    return !IsFileExists(file) || (remove(file.c_str()) == 0);
}

bool RemoveDirectory(const std::string& dir)
{
    return !IsFileExists(dir) || (rmdir(dir.c_str()) == 0);
}

bool ForceCreateDirectory(const std::string& dir)
{
    std::string::size_type index = 0;
    do {
        std::string subPath;
        index = dir.find('/', index + 1); // (index + 1) means the next char traversed
        if (index == std::string::npos) {
            subPath = dir;
        } else {
            subPath = dir.substr(0, index);
        }

        if (!IsFileExists(subPath) && mkdir(subPath.c_str(), S_IRWXU) != 0) {
            return false;
        }
    } while (index != std::string::npos);
    return IsFileExists(dir);
}

std::string GetFilePathByDir(const std::string& dir, const std::string& fileName)
{
    if (dir.empty()) {
        return fileName;
    }
    std::string filePath = dir;
    if (filePath.back() != '/') {
        filePath.push_back(PATH_DELIMITER);
    }
    filePath.append(fileName);
    return filePath;
}

bool IsLegalPath(const std::string& path)
{
    if (path.find("./") != std::string::npos ||
        path.find("../") != std::string::npos) {
        return false;
    }
    return true;
}
} // namespace FileUtil
} // namespace HiviewDFX
} // namespace OHOS
