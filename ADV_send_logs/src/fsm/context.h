#pragma once

#include <filesystem>
#include <string>
#include "infra/log_uploader.h"
#include "infra/file_utils.h"

namespace fs = std::filesystem;

class Context {
public:
    Context(const std::string& serverUrl,
            uint16_t port,
            const std::string& path,
            fs::path outputZip,
            uint8_t timeout = 5,
            uint8_t n_reconnect = 3);

    bool is_reconnect_count_limit_reached();
    void increment_reconnect_counter();

    fs::path home;
    fs::path logsDir;
    fs::path zipPath;

    LogUploader uploader;

private:
    FileDeleter zipCleaner;
    std::string lastError;
    uint8_t reconnect_count;
    const uint8_t n_reconnect_;
};
