#include <string>
#include "infra/log_uploader.h"
#include "infra/file_utils.h"
#include <functional>
#include <httplib.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <cassert>

namespace fs = std::filesystem;

// Paths for the test
const std::string TEST_DIR = "test_payload_dir";
const std::string TEST_FILE = "test_payload.txt";
const std::string ZIP_FILE = "test_payload.zip";
const std::string SERVER_CERT = "server.crt";
const std::string SERVER_KEY = "server.key";

void setupTestFiles() {
    if (fs::exists(TEST_DIR)) fs::remove_all(TEST_DIR);
    fs::create_directory(TEST_DIR);
    std::ofstream ofs(fs::path(TEST_DIR) / TEST_FILE);
    ofs << "This is a test log file content.";
    ofs.close();
    
    // Ensure we have a clean state for zip
    if (fs::exists(ZIP_FILE)) fs::remove(ZIP_FILE);
}

void cleanupTestFiles() {
    if (fs::exists(TEST_DIR)) fs::remove_all(TEST_DIR);
    if (fs::exists(ZIP_FILE)) fs::remove(ZIP_FILE);
}

int main() {
    std::cout << "Starting LogUploader Test..." << std::endl;

    // 1. Setup Files
    setupTestFiles();

    // 2. Zip Directory
    fs::path source = fs::absolute(TEST_DIR);
    fs::path zipDest = fs::absolute(ZIP_FILE);
    std::cout << "Zipping " << source << " to " << zipDest << "..." << std::endl;
    if (!zipDir(source, zipDest, true)) {
        std::cerr << "Failed to zip directory!" << std::endl;
        return 1;
    }
    std::cout << "Zip created successfully." << std::endl;

    // 3. read file size
    std::ifstream zipIn(zipDest, std::ios::binary);
    zipIn.seekg(0, std::ios::end);
    size_t zipFileSize = static_cast<size_t>(zipIn.tellg());
    zipIn.seekg(0, std::ios::beg);

    // 4. Setup SSL Server
    // Note: This requires server.crt and server.key to be present in the working directory
    if (!fs::exists(SERVER_CERT) || !fs::exists(SERVER_KEY)) {
        std::cerr << "Error: server.crt or server.key not found in current directory. Cannot run SSL server." << std::endl;
        return 1;
    }

    httplib::SSLServer svr(SERVER_CERT.c_str(), SERVER_KEY.c_str());
    if (!svr.is_valid()) {
        std::cerr << "Failed to create SSL Server." << std::endl;
        return 1;
    }

    std::atomic<bool> uploadReceived{false};

    svr.Post("/endpoint", 
        [&](const httplib::Request& req, httplib::Response& res) {
            std::cout << "Server received POST /endpoint" << std::endl;
            // Note: we check for "pixet_logs_file" since that's what the client seems to send
            if (req.form.has_file("pixet_logs_file")) {
                const auto& file = req.form.get_file("pixet_logs_file");
                std::cout << "Received file: " << file.filename << ", content_type: " << file.content_type << ", size: " << file.content.length() << std::endl;
                
                if (file.filename == "logs.zip" && file.content.length() == zipFileSize) {
                    uploadReceived = true;
                    res.status = 200;
                    res.set_content("Upload OK", "text/plain");
                } else {
                    std::cerr << "File mismatch! Name: " << file.filename << ", Size: " << file.content.length() << " (Expected size: " << zipFileSize << ")" << std::endl;
                    res.status = 400;
                }
            } else {
                std::cerr << "No file named 'pixet_logs_file' in request!" << std::endl;
                res.status = 400;
            }
        }
    );

    int port = 9999;
    std::thread serverThread([&]() {
        svr.listen("localhost", port);
    });
    
    // Give server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 5. Run LogUploader
    // Helper to find 'main' is not needed as we link against the library parts
    std::cout << "Sending binary..." << std::endl;
    // LogUploader(url, port, path, timeout)
    // Note: LogUploader takes "serverUrl" which behaves like hostname in the client constructor
    // but the implementation does: httplib::SSLClient cli(serverUrl_, port_);
    // so we pass "localhost"p' in re
    LogUploader uploader("localhost", port, "/endpoint", 5); 
    const std::string zipPath = zipDest.string();
    auto res = uploader.sendPlainData(zipPath);

    if (res && res->status == 200) {
        std::cout << "Uploader returned success (200)." << std::endl;
    } else {
        std::cerr << "Uploader failed or returned non-200. Error: " << (res ? std::to_string(res->status) : "Connection failed") << std::endl;
        std::cout << "Error: " << httplib::to_string(res.error()) << std::endl;

        // const auto err = res.error();
        // print_httplib_error(err);
    }

    // 6. Verify and Cleanup
    svr.stop();
    serverThread.join();
    
    std::string s = fs::path(ZIP_FILE).string();
    int _max_tries = 50;
    int tries = 0;
    bool success = false;

    while (tries < _max_tries) {
        try {
            cleanupTestFiles();
            success = true;
            break;
        } catch (const std::exception& e) {
            tries++;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    if (!success) {
        std::cerr << "Failed to cleanup zip test file after " << _max_tries 
                  << " tries." << std::endl;
        return 1;
    }
        
    if (uploadReceived) {
        std::cout << "TEST PASSED: Server received correctly sized payload." << std::endl;
        return 0;
    } else {
        std::cout << "TEST FAILED: Server did not receive correct payload." << std::endl;
        return 1;
    }
}
