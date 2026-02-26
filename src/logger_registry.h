//
// Created by Aman Mehara on 23/02/26.
//

#ifndef PRAPANCHA_LOGGER_REGISTRY_H
#define PRAPANCHA_LOGGER_REGISTRY_H

#include "configuration.h"
#include "logging/async_sink.h"
#include "logging/console_sink.h"
#include "logging/file_sink.h"
#include "logging/log_level.h"
#include "logging/log_sink.h"
#include "logging/logger.h"

namespace mehara::prapancha {

    struct Loggers {
    private:
        static logging::Logger<logging::LogSinks<logging::ConsoleSink, logging::AsyncSink<logging::FileSink>>> &Main() {
            using namespace mehara::prapancha::logging;
            static constexpr std::string_view category = "App";
            static constexpr configuration::Configuration::Logging default_logging{};
            static const auto path = std::filesystem::absolute(default_logging.root_path) / category / "log";
            static LogSinks<ConsoleSink, AsyncSink<FileSink>> sinks(
                    std::make_unique<ConsoleSink>(default_logging.console_enabled),
                    std::make_unique<AsyncSink<FileSink>>(default_logging.async_enabled, path,
                                                          default_logging.file_enabled));
            static Logger instance(LogLevel::Info, std::string(category), sinks);
            return instance;
        }

    public:
        static inline auto &app = Main();
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_LOGGER_REGISTRY_H
