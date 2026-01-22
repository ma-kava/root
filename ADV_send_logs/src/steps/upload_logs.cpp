#include "steps/upload_logs.h"

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

    httplib::Result res = cli.Post(path_, message, "text/plain");
    
    if (!res) {
        const auto err = res.error();
        // const auto err_str = std::to_string(static_cast<int>(err));

        std::string preflight_str = "Preflight TLS failure: ";
        std::string transport_str = "Transport failure (retryable): ";

        switch (err) 
        {
            // Preflight (identity + trust)
            case httplib::Error::SSLConnection:
                cb(false, preflight_str + "SSLConnection");
                break;
            case httplib::Error::SSLServerHostnameVerification:
                cb(false, preflight_str + "SSLServerHostnameVerification");
                break;
            case httplib::Error::SSLLoadingCerts:
                cb(false, preflight_str + "SSLLoadingCerts");
                break;
            case httplib::Error::SSLServerVerification:
                cb(false, preflight_str + "SSLServerVerification");
                break;
                
            // Transport
            case httplib::Error::Connection: // a failure to establish or maintain a connection
                cb(false, transport_str + "Connection");
                break;
            case httplib::Error::ConnectionTimeout:
                cb(false, transport_str + "ConnectionTimeout");
                break;
            case httplib::Error::Read:
                cb(false, transport_str + "Read");
                break;
            case httplib::Error::Write:
                cb(false, transport_str + "Write");
                break;

            default:
                cb(false, "Unknown network failure: " + httplib::to_string(err));
        }
        return;
    }

    if (res) { // server response
        if (res->status >= 200 && res->status < 300) {
            cb(true, "HTTP Success: " + std::to_string(res->status));
        } else if (res->status >= 400 && res->status < 500) { // client error (no retry)
            cb(false, "HTTP Client failure: " + std::to_string(res->status));
        } else if (res->status >= 500) {                      // server error (retry)
            cb(false, "HTTP Server failure: " + std::to_string(res->status));
        }
    } 

    return;
}
