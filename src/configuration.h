//
// Created by Aman Mehara on 15/02/26.
//

#ifndef PRAPANCHA_CONFIGURATION_H
#define PRAPANCHA_CONFIGURATION_H

#include <limits>
#include <string>
#include <string_view>

namespace mehara::prapancha::configuration {

    /// @brief Core configuration container for the Prapancha framework.
    ///
    /// This structure holds all settings related to network, persistence, and execution environment. Once manifested
    /// via the global Active pointer, it remains immutable for the lifetime of the server.
    struct Configuration {

        /// @brief Defines the operational environment of the system.
        enum class Environment {
            Development, ///< Internal mode with verbose logging and debugging features.
            Production ///< Optimized mode for global deployment and performance.
        };

        /// @brief Settings for the Prapancha forensics subsystem.
        struct Forensic {

            /// @brief Configuration for the historical event trail (Breadcrumbs).
            struct Breadcrumbs {
                static constexpr bool DefaultEnabled = false; ///< Default state for breadcrumb recording.
                static constexpr size_t DefaultLimit = 0; ///< Default capacity of the thread-local buffer.
                bool enabled = DefaultEnabled; ///< Activates the thread-local forensic trail.
                size_t per_thread_limit = DefaultLimit; ///< Max historical logs per thread.
            };

            /// @brief Configuration for runtime call stack capture.
            struct Stacktrace {
                static constexpr bool DefaultEnabled = false; ///< Default state for stack trace capture.
                static constexpr size_t DefaultDepth =
                        std::numeric_limits<size_t>::max(); ///< Captures the full available stack.
                static constexpr size_t DefaultSkip = 0; ///< Captures starting from the immediate caller.
                bool enabled = DefaultEnabled; ///< Activates call stack forensics.
                size_t depth = DefaultDepth; ///< Maximum number of frames to reconstruct.
                size_t skip = DefaultSkip; ///< Internal frames to skip from the top.
            };

            Breadcrumbs breadcrumbs; ///< Historical context settings.
            Stacktrace stacktrace; ///< Call stack capture settings.
        };

        /// @brief Settings related to the Prapancha logging subsystem.
        struct Logging {
            static constexpr std::string_view DefaultRootPath = "./logs"; ///< Default relative log storage path.
            static constexpr bool DefaultConsoleEnabled = true; ///< Default state for console-based logging.
            static constexpr bool DefaultFileEnabled = true; ///< Default state for file-based logging.
            static constexpr bool DefaultAsyncEnabled = false; ///< Default state for asynchronous logging.
            std::string root_path = std::string(DefaultRootPath); ///< Filesystem path for log files.
            bool console_enabled = DefaultConsoleEnabled; ///< Enables writing logs to the console.
            bool file_enabled = DefaultFileEnabled; ///< Enables writing logs to the filesystem.
            bool async_enabled = DefaultAsyncEnabled; ///< Enables asynchronous logging.
        };

        /// @brief Network-specific settings for the Prapancha listener.
        struct Network {
            static constexpr std::string_view DefaultHost = "127.0.0.1"; ///< Listens on all available interfaces.
            static constexpr uint16_t DefaultPort = 8080; ///< Fallback port if none is provided.
            static constexpr int AutoDetectThreads = 0; ///< Sentinel to trigger hardware concurrency detection.
            std::string host = std::string(DefaultHost); ///< Binding address for the server.
            uint16_t port = DefaultPort; ///< Listener port number.
            int thread_count = AutoDetectThreads; ///< Number of worker threads for the request pool.
        };

        /// @brief Settings related to the framework's filesystem storage layer.
        struct Persistence {
            static constexpr std::string_view DefaultRootPath = "./data"; ///< Default relative storage path.
            std::string root_path = std::string(DefaultRootPath); ///< Filesystem path for persistent data.
        };

        Environment environment = Environment::Development; ///< Current operational environment.
        Forensic forensic; ///> Instance of forensic settings.
        Logging logging; ///< Instance of logging settings.
        Network network; ///< Instance of network configuration settings.
        Persistence persistence; ///< Instance of persistence storage settings.

        /// @brief Checks if the system is running in Production mode.
        /// @return true if environment is set to Production.
        [[nodiscard]] bool is_production() const noexcept { return environment == Environment::Production; }

        /// @brief Checks if the system is running in Development mode.
        /// @return true if environment is set to Development.
        [[nodiscard]] bool is_development() const noexcept { return environment == Environment::Development; }
    };

    /// @brief Global immutable pointer to the active manifestation of the configuration.
    ///
    /// Components should read from this pointer to access system settings. It is initialized once during
    /// server startup and cannot be modified thereafter.
    extern const Configuration *Active;

    /// @brief Parses command line arguments into a Configuration object.
    /// @param argc Number of arguments from main.
    /// @param argv Array of argument strings from main.
    /// @return A populated Configuration object.
    [[nodiscard]] Configuration from_cli(int argc, const char *const argv[]);

    /// @brief Freezes the provided configuration and assigns it to the Active pointer.
    ///
    /// This function performs the "manifestation" of the world. It can only be successfully called once.
    /// Subsequent calls will be ignored.
    /// @param config The configuration object to move into global storage.
    void initialize(Configuration &&config) noexcept;

} // namespace mehara::prapancha::configuration

#endif // PRAPANCHA_CONFIGURATION_H
