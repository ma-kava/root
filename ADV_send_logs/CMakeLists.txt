cmake_minimum_required(VERSION 3.12)
project(LogUploaderStandalone CXX)

# ---------------------------------------------------------
# 0. Language standard
# ---------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# 1. Source files (module-oriented)
# ---------------------------------------------------------
set(SOURCE_FILES
    src/app/main.cpp

    src/fsm/fsm.cpp

    src/steps/environment.cpp
    src/steps/packaging.cpp
    src/steps/transport.cpp
    src/steps/cleanup.cpp
    src/steps/network_setup.cpp

    src/policy/retry_policy.cpp

    src/infra/file_utils.cpp
    src/infra/log_uploader.cpp
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
# 4. Executable target
# ---------------------------------------------------------
add_executable(log_uploader ${SOURCE_FILES})

# ---------------------------------------------------------
# 5. Include directories
# ---------------------------------------------------------
target_include_directories(log_uploader PRIVATE
    src
    3rdparty/httplib
    ${OPENSSL_INCLUDE_DIR}
    3rdparty/libzippp/src
)

# ---------------------------------------------------------
# 6. Compile definitions
# ---------------------------------------------------------
target_compile_definitions(log_uploader PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
)

# ---------------------------------------------------------
# 7. Link libraries
# ---------------------------------------------------------
target_link_libraries(log_uploader PRIVATE
    OpenSSL::SSL        # httplib needs these to support SSL
    OpenSSL::Crypto
    
    libzippp
)

if(WIN32)
    target_link_libraries(log_uploader PRIVATE ws2_32 crypt32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(log_uploader PRIVATE pthread dl)
endif()
