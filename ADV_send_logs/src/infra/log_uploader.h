#pragma once

#include <string>
#include <cstdint>
#include <functional>
#include <httplib.h>

using Callback = std::function<void(bool success, const std::string& msg)>;

class LogUploader {
public:

    LogUploader(const std::string& serverUrl, uint16_t port, const std::string& path, time_t timeout);
    httplib::Result sendMessage(const std::string& message, Callback cb = nullptr);
    httplib::Result sendBinary(std::vector<char> buffer);

private:
    std::string serverUrl_;
    uint16_t port_;
    std::string path_;
    u_int8_t timeout_;
};
