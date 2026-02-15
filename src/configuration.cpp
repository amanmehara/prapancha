//
// Created by Aman Mehara on 15/02/26.
//

#include "configuration.h"

#include <charconv>
#include <iostream>
#include <memory>
#include <vector>

namespace mehara::prapancha::configuration {

    // Internal storage hidden from other translation units
    static std::unique_ptr<Configuration> global_storage = nullptr;

    // The public read-only pointer
    const Configuration *Active = nullptr;

    void initialize(Configuration &&config) noexcept {
        if (Active != nullptr) {
            // Already initialized, prevent modification
            return;
        }

        // Move the temporary config into permanent heap storage
        global_storage = std::make_unique<Configuration>(std::move(config));

        // Point the immutable public pointer to our storage
        Active = global_storage.get();
    }

    Configuration from_cli(int argc, const char *const argv[]) {
        Configuration config;

        // args is a const vector of views: zero-copy and safe
        const std::vector<std::string_view> args(argv + 1, argv + argc);

        for (size_t i = 0; i < args.size(); ++i) {
            const std::string_view current_arg = args[i];
            if (current_arg == "--production") {
                config.environment = Configuration::Environment::Production;
            } else if (current_arg == "--development") {
                config.environment = Configuration::Environment::Development;
            } else if (current_arg == "--port" && (i + 1) < args.size()) {
                const std::string_view port_val = args[++i];
                auto [ptr, ec] =
                        std::from_chars(port_val.data(), port_val.data() + port_val.size(), config.network.port);
                if (ec != std::errc()) {
                    std::cerr << "Error: Invalid port value '" << port_val
                              << "'. Using default: " << Configuration::Network::DefaultPort << "\n";
                }
            } else if (current_arg == "--root_path" && (i + 1) < args.size()) {
                config.persistence.root_path = std::string(args[++i]);
            } else if (current_arg == "--help") {
                std::cout << "Prapancha Framework\n"
                          << "Usage: " << (argc > 0 ? argv[0] : "prapancha") << " [options]\n\n"
                          << "Options:\n"
                          << "  --production       Execute in a production environment\n"
                          << "  --development      Execute in a development environment\n"
                          << "  --port <number>    Set the network listener port\n"
                          << "  --root_path <path> Set the persistence storage root path\n"
                          << "  --help             Show help information\n";
                std::exit(0);
            }
        }

        return config;
    }

} // namespace mehara::prapancha::configuration
