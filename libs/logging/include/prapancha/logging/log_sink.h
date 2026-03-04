//
// Created by Aman Mehara on 22/02/26.
//

#ifndef PRAPANCHA_LOGGING_LOG_SINK_H
#define PRAPANCHA_LOGGING_LOG_SINK_H

#include <concepts>
#include <memory>
#include <string_view>
#include <tuple>

#include <prapancha/logging/log_level.h>

namespace mehara::prapancha::logging {

    template<typename Derived>
    class LogSink {
    public:
        void write(LogLevel level, std::string_view msg) const noexcept {
            static_cast<const Derived *>(this)->write(level, msg);
        }
    };

    template<typename T>
    concept IsLogSink = std::derived_from<T, LogSink<T>> &&
                        requires(const T &sink, LogLevel level, std::string_view msg) { sink.write(level, msg); };

    template<IsLogSink... Sinks>
    class LogSinks {
        std::tuple<std::unique_ptr<Sinks>...> sinks;

    public:
        explicit LogSinks(std::unique_ptr<Sinks>... args) : sinks(std::move(args)...) {}

        void dispatch(LogLevel level, std::string_view msg) const {
            std::apply([&](const auto &...s) { (s->write(level, msg), ...); }, sinks);
        }
    };

    template<typename T>
    concept IsLogSinks = requires {
        []<typename... Sinks>(const LogSinks<Sinks...> &) {}(std::declval<const std::remove_cvref_t<T> &>());
    };

    template<typename... Sinks>
    LogSinks(Sinks &&...) -> LogSinks<std::remove_cvref_t<Sinks>...>;

} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGING_LOG_SINK_H
