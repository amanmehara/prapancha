//
// Created by Aman Mehara on 22/02/26.
//

#ifndef PRAPANCHA_LOGGING_LOG_SINK_H
#define PRAPANCHA_LOGGING_LOG_SINK_H

#include <concepts>
#include <source_location>
#include <string_view>
#include <tuple>

#include "log_level.h"

namespace mehara::prapancha::logging {

    template<typename Derived>
    class LogSink {
    public:
        void write(LogLevel level, std::string_view msg, const std::source_location &loc) {
            static_cast<Derived *>(this)->write(level, msg, loc);
        }

    protected:
        LogSink() = default;
        ~LogSink() = default;
    };

    template<typename T>
    concept IsLogSink =
            std::derived_from<T, LogSink<T>> && requires(const T &sink, LogLevel level, std::string_view msg,
                                                         std::source_location loc) { sink.write(level, msg, loc); };

    template<IsLogSink... Sinks>
    class LogSinks {
        std::tuple<Sinks...> sinks;

    public:
        explicit LogSinks(Sinks &&...args) : sinks(std::forward<Sinks>(args)...) {}

        void dispatch(LogLevel level, std::string_view msg, const std::source_location& loc) {
            std::apply([&](auto &...s) {
                (s.write(level, msg, loc), ...);
            }, sinks);
        }
    };

    template<typename T>
    concept IsLogSinks = requires {
        []<typename... Sinks>(const LogSinks<Sinks...> &) {}(std::declval<const std::remove_cvref_t<T> &>());
    };

} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGING_LOG_SINK_H
