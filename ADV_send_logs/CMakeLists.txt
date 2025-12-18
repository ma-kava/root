cmake_minimum_required(VERSION 3.23)

project(PixetLog_HTTPTransfer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

# ======================
# Dependencies
# ======================

# libcurl – system dependency
find_package(CURL REQUIRED)

# libzippp – vendored
add_subdirectory(3rdparty/libzippp)

# ======================
# Target
# ======================

add_executable(sender
    src/main.cpp
)

# ======================
# Link
# ======================

target_link_libraries(sender
    PRIVATE CURL::libcurl
    PRIVATE libzippp
)

# libzippp headers
target_include_directories(sender
    PRIVATE 3rdparty/libzippp/src
)
