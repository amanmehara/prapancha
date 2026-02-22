//
// Created by Aman Mehara on 22/02/26.
//

#ifndef PRAPANCHA_LOGGING_FILE_SINK_H
#define PRAPANCHA_LOGGING_FILE_SINK_H

#include <string_view>

#include <trantor/utils/Logger.h>

#include "log_level.h"
#include "log_sink.h"

namespace mehara::prapancha::logging {
    class FileSink : public LogSink<FileSink> {
    public:
        void write(LogLevel level, std::string_view msg, const std::source_location &loc) const {
            const auto source_file = trantor::Logger::SourceFile(loc.file_name());
            const auto trantor_level = to_trantor_level(level);
            trantor::Logger(source_file, static_cast<int>(loc.line()), trantor_level).stream() << msg;
        }

    private:
        static constexpr trantor::Logger::LogLevel to_trantor_level(LogLevel level) noexcept {
            switch (level) {
                case LogLevel::Off:
                case LogLevel::Trace:
                    return trantor::Logger::kTrace;
                case LogLevel::Debug:
                    return trantor::Logger::kDebug;
                case LogLevel::Warn:
                    return trantor::Logger::kWarn;
                case LogLevel::Error:
                case LogLevel::Critical:
                    return trantor::Logger::kError;
                case LogLevel::Info:
                default:
                    return trantor::Logger::kInfo;
            }
        }
    };
} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGING_FILE_SINK_H
