//
// Created by Aman Mehara on 27/02/26.
//

#ifndef PRAPANCHA_LOGGING_CONFIGURATION_H
#define PRAPANCHA_LOGGING_CONFIGURATION_H

#include <cstddef>
#include <limits>
#include <string>
#include <string_view>

namespace mehara::prapancha::logging {

    struct Configuration {

        struct Forensic {

            struct Breadcrumbs {
                static constexpr bool DefaultEnabled = false;
                static constexpr std::size_t DefaultLimit = 0;
                bool enabled = DefaultEnabled;
                std::size_t per_thread_limit = DefaultLimit;
            };

            struct Stacktrace {
                static constexpr bool DefaultEnabled = false;
                static constexpr std::size_t DefaultDepth = std::numeric_limits<std::size_t>::max();
                static constexpr std::size_t DefaultSkip = 0;
                bool enabled = DefaultEnabled;
                std::size_t depth = DefaultDepth;
                std::size_t skip = DefaultSkip;
            };

            Breadcrumbs breadcrumbs;
            Stacktrace stacktrace;
        };

        struct Logging {
            static constexpr std::string_view DefaultRootPath = "./logs";
            static constexpr bool DefaultConsoleEnabled = true;
            static constexpr bool DefaultFileEnabled = true;
            static constexpr bool DefaultAsyncEnabled = false;
            std::string root_path = std::string(DefaultRootPath);
            bool console_enabled = DefaultConsoleEnabled;
            bool file_enabled = DefaultFileEnabled;
            bool async_enabled = DefaultAsyncEnabled;
        };

        Forensic forensic;
        Logging logging;
    };

} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGING_CONFIGURATION_H
