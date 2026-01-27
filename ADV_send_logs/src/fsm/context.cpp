#include "context.h"

#include <filesystem>
#include <string>
#include "infra/log_uploader.h"
#include "infra/file_utils.h"

namespace fs = std::filesystem;

Context::Context(const std::string& serverUrl,
                 uint16_t port,
                 const std::string& path,
                 fs::path outputZip,
                 uint8_t timeout,
                 uint8_t n_reconnect)
    : zipPath(std::move(outputZip)),
      uploader(serverUrl, port, path, timeout),
      zipCleaner(zipPath),
      reconnect_count(0),
      n_reconnect_(n_reconnect)
{
}

bool Context::is_reconnect_count_limit_reached() {
    return reconnect_count >= n_reconnect_ ? true : false;
}

void Context::increment_reconnect_counter() {
    reconnect_count++;
}
