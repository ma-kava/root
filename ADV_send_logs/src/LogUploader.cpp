#include "LogUploader.h"

#include "httplib.h"

#include <string>
#include <cstdint>
#include <functional>

// #define CPPHTTPLIB_OPENSSL_SUPPORT

LogUploader::LogUploader(const std::string& serverUrl, uint16_t port, const std::string& path) {
    serverUrl_ = serverUrl;
    port_ = port;
    path_ = path;
}

// httplib uses 'blocking' socket I/O (calling this fnc within another thread would be better)
void LogUploader::upload(const std::string& filePath, Callback cb) {
    // Tady bude ta logika:
    // 1. Načíst zipPath do std::string nebo vector<char>
    // 2. Vytvořit httplib::SSLClient(serverUrl_)
    // 3. Vypnout ověřování certifikátu (pro začátek/self-signed)
    // 4. cli.Post(...)
    // 5. Zavolat callback
}

void LogUploader::sendMessage(const std::string& message, Callback cb) {
    // HTTPS
    httplib::SSLClient cli(/*"https:" + */serverUrl_, port_);
    
    cli.set_ca_cert_path("server.crt"); // client now trusts server.crt certificate
    cli.enable_server_certificate_verification(false);
    // cli.enable_server_hostname_verification(false);

    auto res = cli.Post(path_, message, "text/plain");
    
    if (!res) {
        cb(false, "Request failed: " + std::to_string(static_cast<int>(res.error())));
        return;
    }

    if (res) {
        if (res->status >= 200 && res->status < 300) {
            cb(true, res->body);
        } else {
            cb(false, "HTTP error: " + std::to_string(res->status));
        }
    } 

    return;
}
