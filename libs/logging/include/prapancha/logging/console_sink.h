//
// Created by Aman Mehara on 22/02/26.
//

#ifndef PRAPANCHA_LOGGING_CONSOLE_SINK_H
#define PRAPANCHA_LOGGING_CONSOLE_SINK_H

#include <cstdio>
#include <print>
#include <string_view>

#include <prapancha/logging/log_level.h>
#include <prapancha/logging/log_sink.h>

namespace mehara::prapancha::logging {
    class ConsoleSink : public LogSink<ConsoleSink> {
    public:
        explicit ConsoleSink(bool enabled = true) noexcept : enabled_(enabled) {}

        void write(LogLevel level, std::string_view msg) const noexcept {
            if (!enabled_) {
                return;
            }
            const auto &color = get_traits(level).color;
            std::println(stdout, "{}{}{}", color, msg, LogLevelTraits::reset);
        }

    private:
        bool enabled_;
    };
} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGING_CONSOLE_SINK_H
