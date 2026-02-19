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
    src/fsm/context.cpp

    src/steps/environment.cpp
    src/steps/network_setup.cpp
    src/steps/packaging.cpp

    src/policy/retry_policy.cpp

    src/infra/file_utils.cpp
    src/infra/log_uploader.cpp

    src/debug/log.hpp
)

# ---------------------------------------------------------
# 2. OpenSSL (vendored)
# ---------------------------------------------------------
set(MY_OPENSSL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/openssl3.0")
set(OPENSSL_INCLUDE_DIR "${MY_OPENSSL_ROOT}/include")

# Definujeme cesty ručně, abychom se vyhnuli detekci platformy
if(WIN32)
    set(OPENSSL_LIB_DIR "${MY_OPENSSL_ROOT}/win64")
    set(LIB_SSL_PATH "${OPENSSL_LIB_DIR}/libssl.lib")
    set(LIB_CRYPTO_PATH "${OPENSSL_LIB_DIR}/libcrypto.lib")
else()
    # Linux / Mac logika zůstává stejná...
    if(APPLE)
        set(OPENSSL_LIB_DIR "${MY_OPENSSL_ROOT}/macos/lib")
    else()
        set(OPENSSL_LIB_DIR "${MY_OPENSSL_ROOT}/linux/lib")
    endif()
    set(LIB_SSL_PATH "${OPENSSL_LIB_DIR}/libssl.a")
    set(LIB_CRYPTO_PATH "${OPENSSL_LIB_DIR}/libcrypto.a")
endif()

message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL Libs: ${LIB_SSL_PATH} ${LIB_CRYPTO_PATH}")

# --- ZMĚNA ZDE: Místo find_package vytvoříme IMPORTED cíle ručně ---

# Vytvoříme cíl OpenSSL::Crypto
if(NOT TARGET OpenSSL::Crypto)
    add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
    set_target_properties(OpenSSL::Crypto PROPERTIES
        IMPORTED_LOCATION "${LIB_CRYPTO_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
    )
endif()

# Vytvoříme cíl OpenSSL::SSL
if(NOT TARGET OpenSSL::SSL)
    add_library(OpenSSL::SSL UNKNOWN IMPORTED)
    set_target_properties(OpenSSL::SSL PROPERTIES
        IMPORTED_LOCATION "${LIB_SSL_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
    )
    # SSL závisí na Crypto
    target_link_libraries(OpenSSL::SSL INTERFACE OpenSSL::Crypto)
endif()

# Nastavíme proměnné, které by normálně nastavil find_package
# (pro případ, že je potřebuje libzippp nebo jiné podmoduly)
set(OPENSSL_FOUND ON CACHE BOOL "" FORCE)
set(OPENSSL_LIBRARIES OpenSSL::SSL OpenSSL::Crypto CACHE INTERNAL "")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_INCLUDE_DIR}" CACHE PATH "" FORCE)

# ---------------------------------------------------------
# 3. Libzippp and Miniz
# ---------------------------------------------------------
# add_subdirectory(3rdparty/libzippp)
add_subdirectory(3rdparty/miniz)

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
    # 3rdparty/libzippp/src
    3rdparty/miniz
)

# ---------------------------------------------------------
# 6. Compile definitions
# ---------------------------------------------------------
target_compile_definitions(log_uploader PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
)

target_compile_definitions(log_uploader PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
    $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0A00> # Tohle vynutí Windows 10 API
)
# ---------------------------------------------------------
# 7. Link libraries
# ---------------------------------------------------------
target_link_libraries(log_uploader PRIVATE
    OpenSSL::SSL        # httplib needs these to support SSL
    OpenSSL::Crypto
    
    # libzippp
    miniz
)

if(WIN32)
    target_link_libraries(log_uploader PRIVATE ws2_32 crypt32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(log_uploader PRIVATE pthread dl)
endif()

# ---------------------------------------------------------
# 8. Test Executable
# ---------------------------------------------------------
add_executable(test_log_uploader src/tests/test_log_uploader.cpp)

target_include_directories(test_log_uploader PRIVATE
    src
    3rdparty/httplib
    ${OPENSSL_INCLUDE_DIR}
    3rdparty/miniz
)

target_link_libraries(test_log_uploader PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    miniz
)

if(WIN32)
    target_link_libraries(test_log_uploader PRIVATE ws2_32 crypt32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_log_uploader PRIVATE pthread dl)
endif()

# We need to link the object files of the library classes we are testing
# Since we didn't create a library target for the core logic (just sources in log_uploader executable),
# we need to reference them again or create a library target. 
# For simplicity, I'll add the necessary source files here.
target_sources(test_log_uploader PRIVATE
    src/infra/log_uploader.cpp
    src/infra/file_utils.cpp
)

target_compile_definitions(test_log_uploader PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
)

target_compile_definitions(test_log_uploader PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
    $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0A00>
)
