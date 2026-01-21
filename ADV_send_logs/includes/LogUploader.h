#pragma once

#include <string>
#include <cstdint>
#include <functional>

class LogUploader {
public:
    using Callback = std::function<void(bool success, const std::string& msg)>;

    LogUploader(const std::string& serverUrl, uint16_t port, const std::string& path);
    void upload(const std::string& filePath, Callback cb = nullptr);
    void sendMessage(const std::string& message, Callback cb = nullptr);

private:
    std::string serverUrl_;
    uint16_t port_;
    std::string path_;
};
