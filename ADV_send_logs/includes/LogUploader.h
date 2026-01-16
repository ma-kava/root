#pragma once

#include <string>
#include <functional>

class LogUploader {
public:
    using Callback = std::function<void(bool success, const std::string& msg)>;

    LogUploader(const std::string& serverUrl);
    void upload(const std::string& filePath, Callback cb);

private:
    std::string serverUrl_;
};
