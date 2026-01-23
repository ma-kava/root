#pragma once

#include <filesystem>
#include <string>
#include "infra/log_uploader.h"

struct Context {
    std::filesystem::path logsDir;
    std::filesystem::path zipPath;

    std::string lastError;

    LogUploader uploader;
};
