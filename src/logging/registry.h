//
// Created by Aman Mehara on 23/02/26.
//

#ifndef PRAPANCHA_LOGGING_REGISTRY_H
#define PRAPANCHA_LOGGING_REGISTRY_H

#include "console_sink.h"
#include "log_level.h"
#include "log_sink.h"
#include "logger.h"

namespace mehara::prapancha::logging {

    struct Loggers {

        static Logger<LogSinks<ConsoleSink>> &Main() {
            static LogSinks<ConsoleSink> sinks{ConsoleSink{}};
            static Logger instance(LogLevel::Info, "App", sinks);
            return instance;
        }
    };

} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGING_REGISTRY_H
