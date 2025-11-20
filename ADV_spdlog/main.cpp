#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <memory>

#include "http_sink.h"

int main() {
    auto sink = std::make_shared<http_sink<std::mutex>>("http://127.0.0.1:5000/log");

    auto logger = std::make_shared<spdlog::logger>("http_logger", spdlog::sinks_init_list{sink});
    spdlog::register_logger(logger);

    // Log messages — each will trigger HTTP POST
    logger->info("Hello from http_sink!");
    logger->error("This is an error log");

    return 0;
}
