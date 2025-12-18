cmake_minimum_required(VERSION 3.23)
project(PixetLog_HTTPTransfer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_SYSTEM_CURL "Use system-installed libcurl if available" ON)

# ======================
# Dependencies
# ======================
include(cmake/Dependencies.cmake)

# libzippp – vendored
add_subdirectory(3rdparty/libzippp)

# ======================
# Target
# ======================
add_executable(sender src/main.cpp)

# ======================
# Link libraries
# ======================
target_link_libraries(sender
    PRIVATE ${CURL_TARGET}
    PRIVATE libzippp
)

target_include_directories(sender
    PRIVATE 3rdparty/libzippp/src
)
