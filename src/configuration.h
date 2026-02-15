//
// Created by Aman Mehara on 15/02/26.
//

#ifndef PRAPANCHA_CONFIGURATION_H
#define PRAPANCHA_CONFIGURATION_H

#include <string>
#include <string_view>

namespace mehara::prapancha::configuration {

    struct Configuration {
        enum class Environment { Development, Production };

        struct Network {
            static constexpr uint16_t DefaultPort = 8080;
            static constexpr int AutoDetectThreads = 0;
            std::string host = "0.0.0.0";
            uint16_t port = DefaultPort;
            int thread_count = AutoDetectThreads;
        };

        struct Persistence {
            static constexpr std::string_view DefaultRootPath = "./data";
            std::string root_path = std::string(DefaultRootPath);
        };

        Network network;
        Persistence persistence;
        Environment environment = Environment::Development;

        [[nodiscard]] bool is_production() const noexcept { return environment == Environment::Production; }

        [[nodiscard]] bool is_development() const noexcept { return environment == Environment::Development; }
    };

    /**
     * Global immutable pointer to the active configuration.
     * Components can read from this, but cannot modify the values.
     */
    extern const Configuration *Active;

    /**
     * Parses command line arguments into a temporary Configuration object.
     */
    [[nodiscard]] Configuration from_cli(int argc, const char *const argv[]);

    /**
     * Freezes the provided configuration and assigns it to the Active pointer.
     * This should be called exactly once during server startup.
     */
    void initialize(Configuration &&config) noexcept;

} // namespace mehara::prapancha::configuration

#endif // PRAPANCHA_CONFIGURATION_H
