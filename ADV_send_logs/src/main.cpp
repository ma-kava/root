#include <filesystem>
#include <iostream>
// #include <curl/curl.h>
// #include <libzippp.h>
#include <sys/stat.h> // for file size
#include <vector>
#include "file_utils.h"

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
    if (result.empty())
        return false;
    fs::path folder(result);
    std::cout << "traversing: " << folder << '\n';


    // ------ creating archive ------
    std::string sourceDir = folder.string();
    std::string outputZip = "diagnostics_upload.zip";
    zipDir(sourceDir, outputZip);


    // ------ libcurl stuff ------
    // CURLcode res;
    // curl_off_t speed_upload, total_time;
    // FILE * fd;
    // struct stat file_info;

    // { // new scope for FileDeleter

    //     FileDeleter cleaner{outputZip};
        
    //     if (stat(outputZip.c_str(), &file_info) != 0) {
    //         std::cerr << "Error: file size cannot be found" << std::endl;
    //         return false;
    //     }
    //     curl_off_t fsize = (curl_off_t)file_info.st_size;
        
    //     fd = fopen(outputZip.c_str(), "rb");
    //     if (!fd) {
    //         std::cerr << "Error: file cannot be opened for reading." << std::endl;
    //         return false;
    //     }
        
    //     res = curl_global_init(CURL_GLOBAL_DEFAULT);
    //     if (res) return false;
        
    //     CURL *curl = curl_easy_init();
    //     if (curl) {
    //         curl_easy_setopt(curl, CURLOPT_URL, "127.0.0.1:5000/log");
            
    //         curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            
    //         curl_easy_setopt(curl, CURLOPT_READDATA, fd);
    //         curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, fsize);
            
    //         // create custom headers list and remove "Expect" header
    //         struct curl_slist *headerlist = NULL;
    //         headerlist = curl_slist_append(headerlist, "Expect:"); 
    //         curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
            
    //         curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            
    //         res = curl_easy_perform(curl);
            
    //         if (res != CURLE_OK) {
    //             fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    //         } else {
    //             curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed_upload);
    //             curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_time);
                
    //             if (total_time > 0) {
    //                 fprintf(stderr, "Speed: %" CURL_FORMAT_CURL_OFF_T " bytes/sec\n", speed_upload);
    //             }
    //         }
            
    //         curl_slist_free_all(headerlist); // Uvolnit hlavičky
    //         curl_easy_cleanup(curl);
    //     }
        
    //     fclose(fd);
    //     curl_global_cleanup();

    // } // <- archive deleted

    // return (res == CURLE_OK);

    return 0;
}
