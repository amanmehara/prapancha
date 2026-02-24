//
// Created by Aman Mehara on 22/02/26.
//

#ifndef PRAPANCHA_LOGGING_LOG_LEVEL_H
#define PRAPANCHA_LOGGING_LOG_LEVEL_H

namespace mehara::prapancha::logging {

    enum class LogLevel { Off = 0, Trace, Debug, Info, Warn, Error, Critical };

    struct LogLevelTraits {
        std::string_view name;
        std::string_view color;
        static constexpr std::string_view reset = "\033[0m";
    };

    [[nodiscard]] constexpr LogLevelTraits get_traits(const LogLevel level) noexcept {
        using enum LogLevel;
        switch (level) {
            case Trace:
                return {"TRACE", "\033[2m"};
            case Debug:
                return {"DEBUG", "\033[36m"};
            case Info:
                return {"INFO", LogLevelTraits::reset};
            case Warn:
                return {"WARN", "\033[33m"};
            case Error:
                return {"ERROR", "\033[31m"};
            case Critical:
                return {"CRITICAL", "\033[1;31m"};
            default:
                return {"OFF", LogLevelTraits::reset};
        }
    }

} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGING_LOG_LEVEL_H
