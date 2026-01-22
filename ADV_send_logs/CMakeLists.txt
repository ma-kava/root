cmake_minimum_required(VERSION 3.12)
project(LogUploaderStandalone CXX)

# Nastavení C++ standardu
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# 1. Source files definition
# ---------------------------------------------------------
set(SOURCE_FILES
    src/main.cpp
    src/steps/cleanup.cpp
    src/steps/locate_logs.cpp
    src/steps/upload_logs.cpp
    src/steps/zip_logs.cpp
    src/file_utils.cpp
)

# ---------------------------------------------------------
# 2. OpenSSL (vendored)
# ---------------------------------------------------------
set(MY_OPENSSL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/openssl3.0")

set(OPENSSL_ROOT_DIR "${MY_OPENSSL_ROOT}" CACHE PATH "OpenSSL root dir")
set(OPENSSL_USE_STATIC_LIBS ON CACHE BOOL "Use static OpenSSL")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OPENSSL_LIB_DIR "${MY_OPENSSL_ROOT}/linux/lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OPENSSL_LIB_DIR "${MY_OPENSSL_ROOT}/macos/lib")
endif()

# setting the cache is important for `find_package(OpenSSL)` in libzippp
set(OPENSSL_INCLUDE_DIR
    "${MY_OPENSSL_ROOT}/include"
    CACHE PATH "OpenSSL include dir"
)

set(OPENSSL_SSL_LIBRARY
    "${OPENSSL_LIB_DIR}/libssl.a"
    CACHE FILEPATH "OpenSSL SSL library"
)

set(OPENSSL_CRYPTO_LIBRARY
    "${OPENSSL_LIB_DIR}/libcrypto.a"
    CACHE FILEPATH "OpenSSL Crypto library"
)

find_package(OpenSSL REQUIRED)

message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL Libs: ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}")

# ---------------------------------------------------------
# 3. Libzippp
# ---------------------------------------------------------

add_subdirectory(3rdparty/libzippp)

# ---------------------------------------------------------
# 4. Target
# ---------------------------------------------------------

add_executable(main ${SOURCE_FILES})

# ---------------------------------------------------------
# 5. Includes
# ---------------------------------------------------------

target_include_directories(main PRIVATE
    includes                     # (LogUploader.h, httplib.h,...)
    includes/steps
    ${OPENSSL_INCLUDE_DIR}       # OpenSSL headers
    3rdparty/libzippp/src        # libzippp headers
)

# ---------------------------------------------------------
# 6. Linking a Definition
# ---------------------------------------------------------

# Turn on SSL for httplib (important to define before including httplib)
target_compile_definitions(main PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

target_link_libraries(main PRIVATE
    OpenSSL::SSL        # httplib needs these to support SSL
    OpenSSL::Crypto
    
    libzippp
)

if(WIN32)
    target_link_libraries(main PRIVATE ws2_32 crypt32)
elseif(UNIX AND NOT APPLE)
    # Na Linuxu statické OpenSSL často potřebuje vlákna a dl
    target_link_libraries(main PRIVATE pthread dl)
endif()