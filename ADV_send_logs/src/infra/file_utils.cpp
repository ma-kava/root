#include "file_utils.h"

#include <filesystem>
#include <fstream>
#include <iostream>
// #include <sys/stat.h> // for file size
#include <miniz.h>
#include <algorithm>

namespace fs = std::filesystem;

FileDeleter::~FileDeleter() {
    if (!p.empty() && fs::exists(p)) {
        fs::remove(p);
    }
}

bool is_zip_ready(const fs::path& dir) {
    if (!fs::exists(dir) || !fs::is_directory(dir))
        return false;
    return true;
}

fs::path get_home_path() {
#ifdef _WIN32
    std::string user_profile = std::getenv("USERPROFILE");
    return user_profile ? fs::path(user_profile) : fs::path("");
#else
    std::string home = std::getenv("HOME");
    return fs::exists(home) ? fs::path(home) : fs::path("");
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

bool zipDir(fs::path& sourceDir, fs::path& outputZip, bool verbose) {
    if (!fs::exists(sourceDir) || !fs::is_directory(sourceDir)) {
        if (verbose) std::cerr << "Source directory does not exist or is not a directory: " << sourceDir << std::endl;
        return false;
    }

    mz_zip_archive zip_archive;
    mz_zip_zero_struct(&zip_archive);

    if (!mz_zip_writer_init_file(&zip_archive, outputZip.string().c_str(), 0)) {
        if (verbose) std::cerr << "Failed to initialize zip writer for: " << outputZip << std::endl;
        return false;
    }

    bool success = true;
    for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
        if (entry.is_regular_file()) {
            std::string fullPath = fs::absolute(entry.path()).string();
            std::string nameInZip = fs::relative(entry.path(), sourceDir).string();
            
            // Normalize path separators to forward slash for ZIP compatibility
            std::replace(nameInZip.begin(), nameInZip.end(), '\\', '/');

            if (!mz_zip_writer_add_file(&zip_archive, nameInZip.c_str(), fullPath.c_str(), NULL, 0, MZ_DEFAULT_COMPRESSION)) {
                if (verbose) std::cerr << "Failed to add file: " << nameInZip << " (Path: " << fullPath << ")" << std::endl;
                success = false;
            } else if (verbose) {
                std::cout << "Added: " << nameInZip << std::endl;
            }
        }
    }

    if (!mz_zip_writer_finalize_archive(&zip_archive)) {
        if (verbose) std::cerr << "Failed to finalize archive" << std::endl;
        success = false;
    }

    if (!mz_zip_writer_end(&zip_archive)) {
        if (verbose) std::cerr << "Failed to end zip writer" << std::endl;
        success = false;
    }

    return success;
}

bool env_or_default(const char* variable, std::string& set_val, std::string default_val) {
    const char* received = std::getenv(variable);
    set_val = received ? std::string(received) : default_val;
    return received != nullptr ? true : false;
}
