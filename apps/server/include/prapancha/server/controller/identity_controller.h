//
// Created by Aman Mehara on 14/03/26.
//

#ifndef PRAPANCHA_SERVER_CONTROLLER_IDENTITY_CONTROLLER_H_
#define PRAPANCHA_SERVER_CONTROLLER_IDENTITY_CONTROLLER_H_

#include <algorithm>
#include <string>
#include <string_view>
#include <tuple>

#include <prapancha/server/controller/base_controller.h>
#include <prapancha/server/logger_registry.h>
#include <prapancha/server/model.h>

namespace mehara::prapancha {

    template<typename Persistence>
    class RegistrationController : public BaseController<RegistrationController<Persistence>> {
        Persistence persistence_;

    public:
        explicit RegistrationController(Persistence persistence) : persistence_(std::move(persistence)) {}
        static constexpr std::string_view ControllerName = "registration";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(auto &&ctx, auto &&sender) {
            Loggers::App().log_warn([&] { return std::format("{}. 501 NotImplemented!", ControllerName); });
            http::Response res{http::Status::NotImplemented};
            res.set_header("Content-Type", "text/html; charset=utf-8");
            res.body = "प्रपञ्च — Prapancha: 501 NotImplemented!";
            sender(std::move(res));
        }
    };

    template<typename Persistence>
    class DeregistrationController : public BaseController<DeregistrationController<Persistence>> {
        Persistence persistence_;

    public:
        explicit DeregistrationController(Persistence persistence) : persistence_(std::move(persistence)) {}
        static constexpr std::string_view ControllerName = "deregistration";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(auto &&ctx, auto &&sender) {
            Loggers::App().log_warn([&] { return std::format("{}. 501 NotImplemented!", ControllerName); });
            http::Response res{http::Status::NotImplemented};
            res.set_header("Content-Type", "text/html; charset=utf-8");
            res.body = "प्रपञ्च — Prapancha: 501 NotImplemented!";
            sender(std::move(res));
        }
    };

    template<typename Persistence>
    class LoginController : public BaseController<LoginController<Persistence>> {
        Persistence persistence_;

    public:
        explicit LoginController(Persistence persistence) : persistence_(std::move(persistence)) {}
        static constexpr std::string_view ControllerName = "login";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(auto &&ctx, auto &&sender) {
            Loggers::App().log_warn([&] { return std::format("{}. 501 NotImplemented!", ControllerName); });
            http::Response res{http::Status::NotImplemented};
            res.set_header("Content-Type", "text/html; charset=utf-8");
            res.body = "प्रपञ्च — Prapancha: 501 NotImplemented!";
            sender(std::move(res));
        }
    };

    template<typename Persistence>
    class LogoutController : public BaseController<LogoutController<Persistence>> {
        Persistence persistence_;

    public:
        explicit LogoutController(Persistence persistence) : persistence_(std::move(persistence)) {}
        static constexpr std::string_view ControllerName = "logout";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(auto &&ctx, auto &&sender) {
            Loggers::App().log_warn([&] { return std::format("{}. 501 NotImplemented!", ControllerName); });
            http::Response res{http::Status::NotImplemented};
            res.set_header("Content-Type", "text/html; charset=utf-8");
            res.body = "प्रपञ्च — Prapancha: 501 NotImplemented!";
            sender(std::move(res));
        }
    };


} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_CONTROLLER_IDENTITY_CONTROLLER_H_
