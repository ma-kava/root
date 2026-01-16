cmake_minimum_required(VERSION 3.12)
project(LogUploaderStandalone CXX)

# Nastavení C++ standardu (libzippp i httplib vyžadují moderní C++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# 1. find OpenSSL
# ---------------------------------------------------------

# path to vendored OpenSSL
set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/openssl3.0")
set(OPENSSL_USE_STATIC_LIBS ON) 

find_package(OpenSSL REQUIRED)

message(STATUS "OpenSSL Found: ${OPENSSL_FOUND}")
message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")

# ---------------------------------------------------------
# 2. libzippp
# ---------------------------------------------------------

add_subdirectory(3rdparty/libzippp)

# ---------------------------------------------------------
# 3. app
# ---------------------------------------------------------

add_executable(LogUploaderApp
    src/main.cpp
    # src/LogUploader.cpp
)

# ---------------------------------------------------------
# 4. includes
# ---------------------------------------------------------

target_include_directories(LogUploaderApp PRIVATE
    include                     # for LogUploader.h and httplib.h
    3rdparty/libzippp/src       # for libzippp.h
)

# ---------------------------------------------------------
# 5. link and definition
# ---------------------------------------------------------

# Enabling SSL support for httplib
target_compile_definitions(LogUploaderApp PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

target_link_libraries(LogUploaderApp PRIVATE
    # 1. OpenSSL
    OpenSSL::SSL
    OpenSSL::Crypto
    
    # 2. libzippp
    libzippp
)

if(WIN32)
    target_link_libraries(LogUploaderApp PRIVATE ws2_32 crypt32)
endif()