
#include "infra/log_uploader.h"
#include "infra/file_utils.h"
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

    // 3. Read Zip to Buffer
    std::ifstream zipIn(zipDest, std::ios::binary);
    std::vector<char> zipBuffer((std::istreambuf_iterator<char>(zipIn)), std::istreambuf_iterator<char>());
    std::cout << "Read " << zipBuffer.size() << " bytes from zip file." << std::endl;

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

    svr.Post("/endpoint", [&](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Server received POST /endpoint" << std::endl;
        if (req.form.has_file("file3")) {
            const auto& file = req.form.get_file("file3");
            std::cout << "Received file: " << file.filename << ", content_type: " << file.content_type << ", size: " << file.content.size() << std::endl;
            if (file.filename == "logs.zip" && file.content.size() == zipBuffer.size()) {
                 uploadReceived = true;
                 res.status = 200;
                 res.set_content("Upload OK", "text/plain");
            } else {
                std::cerr << "File mismatch!" << std::endl;
                res.status = 400;
            }
        } else {
            std::cerr << "No file named 'file3' in request!" << std::endl;
            res.status = 400;
        }
    });

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
    // so we pass "localhost"
    LogUploader uploader("localhost", port, "/endpoint", 5); 
    auto res = uploader.sendBinary(zipBuffer);

    if (res && res->status == 200) {
        std::cout << "Uploader returned success (200)." << std::endl;
    } else {
        std::cerr << "Uploader failed or returned non-200. Error: " << (res ? std::to_string(res->status) : "Connection failed") << std::endl;
    }

    // 6. Verify and Cleanup
    svr.stop();
    serverThread.join();
    cleanupTestFiles();

    if (uploadReceived) {
        std::cout << "TEST PASSED: Server received correctly sized payload." << std::endl;
        return 0;
    } else {
        std::cout << "TEST FAILED: Server did not receive correct payload." << std::endl;
        return 1;
    }
}
