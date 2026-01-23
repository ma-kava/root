#include <filesystem>
#include <iostream>
// #include <curl/curl.h>
// #include <libzippp.h>
#include <sys/stat.h> // for file size
#include <vector>
#include "infra/file_utils.h"
#include "infra/log_uploader.h"

// using namespace libzippp;
namespace fs = std::filesystem;


int main() {
    // ------ seraching for logs file ------
    std::string home = get_home_path();
    
    std::vector<fs::path> searchPaths = {
        fs::path(home),
        fs::path(home) / ".config"
    };

    fs::path result = findFolder(searchPaths, "PixetPro/logs");
    // read files
    if (result.empty())
        return false;
        
    fs::path folder(result);
    
    
    // ------ creating archive ------
    std::cout << "zipping: " << folder << '\n';
    std::string sourceDir = folder.string();
    std::string outputZip = "diagnostics_upload.zip";
    zipDir(sourceDir, outputZip, false);


    // ------ communication with the server ------

    { // new scope for FileDeleter

        FileDeleter cleaner{outputZip};

        LogUploader uploader{"127.0.0.1", 5000, "/log"};

        uploader.sendMessage("Hello server", [](bool success, const std::string& msg) {
            if (success) {
                std::cout << "Upload OK: " << msg << std::endl;
            } else {
                std::cerr << "Upload FAILED: " << msg << std::endl;
            }
        });

    } // <- archive deleted

    return 0;
}
