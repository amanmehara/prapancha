//
// Created by Aman Mehara on 23/02/26.
//

#ifndef PRAPANCHA_SERVER_LOGGER_REGISTRY_H
#define PRAPANCHA_SERVER_LOGGER_REGISTRY_H

#include <prapancha/logging/async_sink.h>
#include <prapancha/logging/configuration.h>
#include <prapancha/logging/console_sink.h>
#include <prapancha/logging/file_sink.h>
#include <prapancha/logging/log_level.h>
#include <prapancha/logging/log_sink.h>
#include <prapancha/logging/logger.h>

namespace mehara::prapancha {

    struct Loggers {
        static auto &App() {
            using namespace mehara::prapancha::logging;
            static constexpr std::string_view category = "App";
            static constexpr Configuration::Logging default_logging{};
            static const auto path = std::filesystem::absolute(default_logging.root_path) / category / "log";
            static LogSinks<ConsoleSink, AsyncSink<FileSink>> sinks(
                    std::make_unique<ConsoleSink>(default_logging.console_enabled),
                    std::make_unique<AsyncSink<FileSink>>(default_logging.async_enabled, path,
                                                          default_logging.file_enabled));
            static Logger instance(LogLevel::Info, std::string(category), sinks);
            return instance;
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_LOGGER_REGISTRY_H
