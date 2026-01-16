#include "LogUploader.h"

#include "httplib.h"

#include <string>
#include <functional>

LogUploader::LogUploader(const std::string& serverUrl) {
}

void LogUploader::upload(const std::string& filePath, Callback cb) {
    // Tady bude ta logika:
    // 1. Načíst zipPath do std::string nebo vector<char>
    // 2. Vytvořit httplib::SSLClient(serverUrl_)
    // 3. Vypnout ověřování certifikátu (pro začátek/self-signed)
    // 4. cli.Post(...)
    // 5. Zavolat callback
}

