#include "infra/log_uploader.h"

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <httplib.h>

// #define CPPHTTPLIB_OPENSSL_SUPPORT (defined in CMakeLists.txt)

LogUploader::LogUploader(const std::string& serverUrl, uint16_t port, const std::string& path, time_t timeout) {
    serverUrl_ = serverUrl;
    port_ = port;
    path_ = path;
    timeout_ = timeout;
}

// httplib uses 'blocking' socket I/O
httplib::Result LogUploader::sendMessage(const std::string& message, Callback cb) {
    // HTTPS
    httplib::Client cli(/*"https:" + */serverUrl_ + std::to_string(port_));
    
    cli.set_ca_cert_path("server.crt"); // client now trusts server.crt certificate
    cli.enable_server_certificate_verification(false);
    // cli.enable_server_hostname_verification(false);

    cli.set_connection_timeout(timeout_); // seconds
    cli.set_read_timeout(timeout_);
    cli.set_write_timeout(timeout_);

    httplib::Result res = cli.Post(path_, message, "text/plain");
    
    return res;
}

httplib::Result LogUploader::sendPlainData(const std::string& zipPath) {
    httplib::Client cli("https://" + serverUrl_ + ":" + std::to_string(port_));

    cli.set_ca_cert_path("server.crt");
    cli.enable_server_certificate_verification(false);
    // cli.enable_server_hostname_verification(false);

    cli.set_connection_timeout(timeout_);
    cli.set_read_timeout(timeout_);
    cli.set_write_timeout(timeout_);

    // std::ifstream file(zipPath, std::ios::binary);
    // if (!file.is_open()) {
    //     std::cerr << "Error opening file for reading." << std::endl;
    //     return httplib::Result(nullptr, httplib::Error::Unknown); 
    // }

    httplib::FormDataProviderItems items;
    httplib::FormDataProvider provider; // ensures the file is sent in chunks (saves RAM)
    
    std::string name = "pixet_logs_file"; // Form field name
    std::string filepath = zipPath;       // Local file path
    std::string filename = "logs.zip";    // Filename in the form data
    std::string content_type = "application/octet-stream";
    provider = httplib::make_file_provider(name, filepath, filename, content_type);
    
    items.push_back(provider);

    httplib::UploadFormDataItems fields = {
        {"user_id", "123", "", ""}
    };

    return cli.Post(path_, {}, fields, items); 
}
