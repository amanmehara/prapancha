//
// Created by Aman Mehara on 22/02/26.
//

#ifndef PRAPANCHA_LOGGING_CONSOLE_SINK_H
#define PRAPANCHA_LOGGING_CONSOLE_SINK_H

#include <cstdio>
#include <print>
#include <string_view>

#include "log_level.h"
#include "log_sink.h"

namespace mehara::prapancha::logging {
    class ConsoleSink : public LogSink<ConsoleSink> {
    public:
        void write(LogLevel level, std::string_view msg) const { std::println(stdout, "{}", msg); }
    };
} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGING_CONSOLE_SINK_H
