//
// Created by Aman Mehara on 22/02/26.
//

#ifndef PRAPANCHA_LOGGER_H
#define PRAPANCHA_LOGGER_H

#include <concepts>
#include <deque>
#include <format>
#include <functional>
#include <string>

#include "log_level.h"
#include "log_sink.h"

namespace mehara::prapancha::logging {

    template<IsLogSinks Sinks>
    class Logger {
    public:
        Logger(const LogLevel lvl, std::string cat, Sinks &s) : min_level(lvl), category(std::move(cat)), sinks(s) {}

        template<typename... Args>
            requires(std::formattable<Args, char> && ...)
        void log_trace(std::format_string<Args...> fmt, Args &&...args) {
            log(LogLevel::Trace, fmt, std::forward<Args>(args)...);
        }
        template<typename... Args>
            requires(std::formattable<Args, char> && ...)
        void log_debug(std::format_string<Args...> fmt, Args &&...args) {
            log(LogLevel::Debug, fmt, std::forward<Args>(args)...);
        }
        template<typename... Args>
            requires(std::formattable<Args, char> && ...)
        void log_info(std::format_string<Args...> fmt, Args &&...args) {
            log(LogLevel::Info, fmt, std::forward<Args>(args)...);
        }
        template<typename... Args>
            requires(std::formattable<Args, char> && ...)
        void log_warn(std::format_string<Args...> fmt, Args &&...args) {
            log(LogLevel::Warn, fmt, std::forward<Args>(args)...);
        }
        template<typename... Args>
            requires(std::formattable<Args, char> && ...)
        void log_error(std::format_string<Args...> fmt, Args &&...args) {
            log(LogLevel::Error, fmt, std::forward<Args>(args)...);
        }
        template<typename... Args>
            requires(std::formattable<Args, char> && ...)
        void log_critical(std::format_string<Args...> fmt, Args &&...args) {
            log(LogLevel::Critical, fmt, std::forward<Args>(args)...);
        }

        template<typename Callable>
            requires std::invocable<Callable> && std::convertible_to<std::invoke_result_t<Callable>, std::string_view>
        void log_trace(Callable &&callable) {
            if (should_log(LogLevel::Trace)) {
                dispatch(LogLevel::Trace, std::invoke(std::forward<Callable>(callable)));
            }
        }
        template<typename Callable>
            requires std::invocable<Callable> && std::convertible_to<std::invoke_result_t<Callable>, std::string_view>
        void log_debug(Callable &&callable) {
            if (should_log(LogLevel::Debug)) {
                dispatch(LogLevel::Debug, std::invoke(std::forward<Callable>(callable)));
            }
        }
        template<typename Callable>
            requires std::invocable<Callable> && std::convertible_to<std::invoke_result_t<Callable>, std::string_view>
        void log_info(Callable &&callable) {
            if (should_log(LogLevel::Info)) {
                dispatch(LogLevel::Info, std::invoke(std::forward<Callable>(callable)));
            }
        }
        template<typename Callable>
            requires std::invocable<Callable> && std::convertible_to<std::invoke_result_t<Callable>, std::string_view>
        void log_warn(Callable &&callable) {
            if (should_log(LogLevel::Warn)) {
                dispatch(LogLevel::Warn, std::invoke(std::forward<Callable>(callable)));
            }
        }
        template<typename Callable>
            requires std::invocable<Callable> && std::convertible_to<std::invoke_result_t<Callable>, std::string_view>
        void log_error(Callable &&callable) {
            if (should_log(LogLevel::Error)) {
                dispatch(LogLevel::Error, std::invoke(std::forward<Callable>(callable)));
            }
        }
        template<typename Callable>
            requires std::invocable<Callable> && std::convertible_to<std::invoke_result_t<Callable>, std::string_view>
        void log_critical(Callable &&callable) {
            if (should_log(LogLevel::Critical)) {
                dispatch(LogLevel::Critical, std::invoke(std::forward<Callable>(callable)));
            }
        }

    private:
        LogLevel min_level;
        std::string category;
        Sinks &sinks;
        bool backtrace_enabled = true;

        struct ThreadContext {
            std::string log_buffer;
            std::deque<std::string> history;
            static constexpr size_t max_history = 50;
        };

        static inline thread_local ThreadContext ctx;

        [[nodiscard]] bool should_log(const LogLevel level) const noexcept {
            return (min_level != LogLevel::Off) && (level >= min_level || backtrace_enabled);
        }

        template<typename... Args>
        void log(const LogLevel level, std::format_string<Args...> fmt, Args &&...args) {
            if (should_log(level)) {
                dispatch(level, assemble(fmt, std::forward<Args>(args)...));
            }
        }

        template<typename... Args>
        [[nodiscard]] std::string_view assemble(std::format_string<Args...> fmt, Args &&...args) {
            ctx.log_buffer.clear();
            std::format_to(std::back_inserter(ctx.log_buffer), fmt, std::forward<Args>(args)...);
            return ctx.log_buffer;
        }

        void dispatch(LogLevel level, std::string_view msg) {
            if (level >= min_level) {
                if (level >= LogLevel::Error) {
                    flush();
                }
                sinks.dispatch(level, msg);
            } else if (backtrace_enabled) {
                ctx.history.emplace_back(msg);
                if (ctx.history.size() > ThreadContext::max_history)
                    ctx.history.pop_front();
            }
        }

        void flush() {
            if (ctx.history.empty())
                return;
            sinks.dispatch(LogLevel::Info, ">>> [HISTORY START] <<<");
            for (const auto &line: ctx.history) {
                sinks.dispatch(LogLevel::Trace, line);
            }
            sinks.dispatch(LogLevel::Info, ">>> [HISTORY END] <<<");
            ctx.history.clear();
        }
    };

} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGER_H
