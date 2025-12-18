# Dependencies.cmake

if(USE_SYSTEM_CURL)
    find_package(CURL)
    if(CURL_FOUND)
        message(STATUS "Using system-installed libcurl: ${CURL_VERSION}")
        set(CURL_TARGET CURL::libcurl)
    else()
        message(WARNING "System libcurl not found, falling back to vendored version")
        set(USE_SYSTEM_CURL OFF)
    endif()
endif()

if(NOT USE_SYSTEM_CURL)
    # Přidáme jen to, co je potřeba, z 3rdparty/curl
    # např. jen statickou knihovnu curl (bez testů, docs, tools)
    add_subdirectory(3rdparty/curl EXCLUDE_FROM_ALL)
    set(CURL_TARGET curl)  # podle jména targetu v CMakeLists.txt 3rdparty/curl
endif()
