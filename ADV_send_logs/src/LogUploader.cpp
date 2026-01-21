#include "LogUploader.h"

#include "httplib.h"

#include <string>
#include <cstdint>
#include <functional>

LogUploader::LogUploader(const std::string& serverUrl, uint16_t port, const std::string& path) {
    serverUrl_ = serverUrl;
    port_ = port;
    path_ = path;
}

void LogUploader::upload(const std::string& filePath, Callback cb) {
    // Tady bude ta logika:
    // 1. Načíst zipPath do std::string nebo vector<char>
    // 2. Vytvořit httplib::SSLClient(serverUrl_)
    // 3. Vypnout ověřování certifikátu (pro začátek/self-signed)
    // 4. cli.Post(...)
    // 5. Zavolat callback
}

void LogUploader::sendMessage(const std::string& message, Callback cb) {
    // HTTP
    httplib::Client cli(serverUrl_, port_);

    auto res = cli.Post(path_, message, "text/plain");

    if (res) {
        if (res->status >= 200 && res->status < 300) {
            cb(true, res->body);
        } else {
            cb(false, "HTTP error: " + std::to_string(res->status));
        }
    } else {
        cb(false, "Request failed: " + std::to_string(static_cast<int>(res.error())));
    }

    return;
}
