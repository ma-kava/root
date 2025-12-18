#pragma once
#include <spdlog/sinks/base_sink.h>
#include <curl/curl.h>
#include <string>

template<typename Mutex>
class http_sink : public spdlog::sinks::base_sink<Mutex> {
public:
    explicit http_sink(std::string url) : url_(std::move(url)) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~http_sink() override {
        curl_global_cleanup();
    }

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override {
        spdlog::memory_buf_t formatted;
        this->formatter_->format(msg, formatted);

        CURL *curl = curl_easy_init();
        if (!curl) return;

        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, formatted.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, formatted.size());

        // optional: prevent output to stdout
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    void flush_() override {}

private:
    std::string url_;
};
