//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_ROUTER_H
#define PRAPANCHA_ROUTER_H

#include <memory>

#include "drogon/HttpAppFramework.h"

#include "configuration.h"

namespace mehara::prapancha {

    /// @brief Orchestrates endpoint registration and handler mapping for the framework.
    class Router {
    public:
        /// @brief Constructs a Router and binds it to a specific application and configuration.
        ///
        /// Upon construction, the Router immediately registers all framework routes to the app.
        /// @param app Reference to the Drogon application framework instance.
        /// @param configuration Pointer to the manifested system configuration.
        explicit Router(drogon::HttpAppFramework &app,
                        const configuration::Configuration *configuration = configuration::Active);

    private:
        /// @brief Wraps a controller member function into a Drogon-compatible handler.
        template<typename T>
        auto to_handler(const std::shared_ptr<T> &controller);

        /// @brief Internal execution of route registration.
        void register_routes();

        drogon::HttpAppFramework &app_; ///< Reference to the underlying HTTP framework.
        const configuration::Configuration *configuration_; ///< Governing configuration for path resolution.
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_ROUTER_H
