//
// Created by Aman Mehara on 15/02/26.
//

#include "configuration.h"

#include <charconv>
#include <iostream>
#include <memory>
#include <vector>

namespace mehara::prapancha::configuration {

    /// @brief Internal storage for the configuration instance, managed via unique_ptr for automatic cleanup.
    static std::unique_ptr<Configuration> global_storage = nullptr;

    /// @brief Global read-only access point for the manifested configuration.
    const Configuration *Active = nullptr;

    /// @brief Initializes the global configuration state. This function is idempotent and only initializes on first
    /// call.
    ///
    /// Subsequent calls are ignored to preserve the immutability of the active configuration throughout the lifecycle.
    void initialize(Configuration &&config) noexcept {
        if (Active != nullptr) {
            return;
        }

        global_storage = std::make_unique<Configuration>(std::move(config));
        Active = global_storage.get();
    }

    /// @brief Parses command line arguments into a Configuration structure to override default framework settings.
    ///
    /// Iterates through arguments; numeric values that fail validation revert to the defaults specified in the struct.
    Configuration from_cli(int argc, const char *const argv[]) {
        Configuration config;
        const std::vector<std::string_view> args(argv + 1, argv + argc);
        for (size_t i = 0; i < args.size(); ++i) {
            const std::string_view current_arg = args[i];
            if (current_arg == "--production") {
                config.environment = Configuration::Environment::Production;
            } else if (current_arg == "--development") {
                config.environment = Configuration::Environment::Development;
            } else if (current_arg == "--port" && (i + 1) < args.size()) {
                const std::string_view val = args[++i];
                auto [ptr, ec] = std::from_chars(val.data(), val.data() + val.size(), config.network.port);
                if (ec != std::errc()) {
                    std::cerr << "Warning: Invalid port '" << val
                              << "'. Using default: " << Configuration::Network::DefaultPort << "\n";
                }
            } else if (current_arg == "--thread_count" && (i + 1) < args.size()) {
                const std::string_view val = args[++i];
                auto [ptr, ec] = std::from_chars(val.data(), val.data() + val.size(), config.network.thread_count);
                if (ec != std::errc()) {
                    std::cerr << "Warning: Invalid thread_count '" << val
                              << "'. Using default: " << Configuration::Network::AutoDetectThreads << "\n";
                }
            } else if (current_arg == "--root_path" && (i + 1) < args.size()) {
                config.persistence.root_path = std::string(args[++i]);
            } else if (current_arg == "--help") {
                std::cout << "Prapancha Framework\n"
                          << "Usage: " << (argc > 0 ? argv[0] : "prapancha") << " [options]\n\n"
                          << "Options:\n"
                          << "  --production         Execute in a production environment\n"
                          << "  --development        Execute in a development environment\n"
                          << "  --port <number>      Set the network listener port\n"
                          << "  --thread_count <n>   Set number of worker threads (0 for auto)\n"
                          << "  --root_path <path>   Set the persistence storage root path\n"
                          << "  --help               Show help information\n";
                std::exit(0);
            }
        }
        return config;
    }

} // namespace mehara::prapancha::configuration
