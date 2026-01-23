#include "file_utils.h"

// #include <cstdlib>
#include <filesystem>
#include <iostream>
#include <libzippp.h>
#include <sys/stat.h> // for file size

using namespace libzippp;
namespace fs = std::filesystem;

FileDeleter::~FileDeleter() { 
    if (fs::exists(p)) {
        fs::remove(p); 
        // std::cout << "Cleanup: deleted temp file " << p << std::endl;
    }
}

/*
Result<fs::path, FsError> get_home_path() {
#ifdef _WIN32
    if (const char* p = std::getenv("USERPROFILE")) {
        return { fs::path(p), std::nullopt };
    }
    return { std::nullopt, FsError::HomeNotSet };
#else
    if (const char* p = std::getenv("HOME")) {
        fs::path home(p);
        if (fs::exists(home)) {
            return { home, std::nullopt };
        }
        return { std::nullopt, FsError::PathNotFound };
    }
    return { std::nullopt, FsError::HomeNotSet };
#endif
}
*/

fs::path get_home_path() {
#ifdef _WIN32
    std::string user_profile = std::getenv("USERPROFILE");
    return user_profile ? fs::path(user_profile) : fs::path("C:\\");
#else
    std::string home = std::getenv("HOME");
    return fs::exists(home) ? fs::path(home) : fs::path("/tmp");
#endif
}

fs::path findFolder(const std::vector<fs::path>& locations, const std::string& folderName) {
    for (const auto& loc : locations) {
        fs::path candidate = loc / folderName;
        if (fs::exists(candidate) && fs::is_directory(candidate)) {
            return candidate;
        }
    }
    return "";
}

bool zipDir(std::string& sourceDir, std::string& outputZip, bool verbose) {
    // create and open zip 
    ZipArchive zf(outputZip);
    if (!zf.open(ZipArchive::New)) {
        std::cerr << "Error: archive cannot be created " << outputZip << std::endl;
        return false;
    }

    for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
        if (verbose)
            std::cout << entry.path().string() << '\n';
        
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();
            
            // realitve path only
            std::string nameInZip = fs::relative(entry.path(), sourceDir).string();

#ifdef _WIN64
            // zip standard requires '/' even on windows
            std::replace(nameInZip.begin(), nameInZip.end(), '\\', '/');
#endif

            if (!zf.addFile(nameInZip, filePath) && verbose) {
                std::cerr << "Warning: Failed to add " << nameInZip << std::endl;
            } else if (verbose) {
                std::cout << "Added: " << nameInZip << std::endl;
            }
        }
    }

    int result = zf.close();
    if (result != LIBZIPPP_OK) {
        std::cerr << "Error during archiving. Code: " << result << std::endl;
        return false;
    }

    return true;
}
