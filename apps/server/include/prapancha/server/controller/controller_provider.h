//
// Created by Aman Mehara on 08/03/26.
//

#ifndef PRAPANCHA_SERVER_CONTROLLER_CONTROLLER_PROVIDER_H_
#define PRAPANCHA_SERVER_CONTROLLER_CONTROLLER_PROVIDER_H_

#include <prapancha/server/controller/root_controller.h>
#include <prapancha/server/controller/status_controller.h>
#include <prapancha/server/controller/void_controller.h>

namespace mehara::prapancha {

    class ControllerProvider {
    public:
        static void root_controller(http::Request &&req, std::function<void(http::Response)> &&send) {
            static auto instance = std::make_shared<RootController>();
            instance->dispatch(std::move(req), std::forward<decltype(send)>(send));
        }

        static void status_controller(http::Request &&req, std::function<void(http::Response)> &&send) {
            static auto instance = std::make_shared<StatusController>();
            instance->dispatch(std::move(req), std::forward<decltype(send)>(send));
        }

        static void void_controller(http::Request &&req, std::function<void(http::Response)> &&send) {
            static auto instance = std::make_shared<VoidController>();
            instance->dispatch(std::move(req), std::forward<decltype(send)>(send));
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_CONTROLLER_CONTROLLER_PROVIDER_H_
