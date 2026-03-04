//
// Created by Aman Mehara on 25/02/26.
//

#ifndef PRAPANCHA_LOGGING_FILE_SINK_H_
#define PRAPANCHA_LOGGING_FILE_SINK_H_

#include <cstdio>
#include <filesystem>
#include <print>
#include <string_view>

#include <prapancha/logging/log_level.h>
#include <prapancha/logging/log_sink.h>

namespace mehara::prapancha::logging {

    class FileSink : public LogSink<FileSink> {
    public:
        static constexpr std::string_view mode_append = "a";

        explicit FileSink(const std::filesystem::path &path, const bool enabled = true) noexcept :
            enabled_(enabled), file_(nullptr) {
            if (!enabled_) {
                return;
            }
            if (const auto parent = path.parent_path(); !parent.empty()) {
                std::error_code ec;
                std::filesystem::create_directories(parent, ec);
            }
            file_ = std::fopen(path.string().c_str(), mode_append.data());
        }

        ~FileSink() {
            if (file_) {
                std::fclose(file_);
            }
        }

        void write(LogLevel level, std::string_view msg) const noexcept {
            if (enabled_ && file_) {
                std::println(file_, "{}", msg);
            }
        }

    private:
        bool enabled_;
        std::FILE *file_;
    };

} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGING_FILE_SINK_H_
