//
// Created by Aman Mehara on 14/03/26.
//

#ifndef PRAPANCHA_SERVER_CONTROLLER_IDENTITY_CONTROLLER_H_
#define PRAPANCHA_SERVER_CONTROLLER_IDENTITY_CONTROLLER_H_

#include <algorithm>
#include <string>
#include <string_view>
#include <tuple>

#include <prapancha/server/codec/binary_json_codec.h>
#include <prapancha/server/controller/base_controller.h>
#include <prapancha/server/logger_registry.h>
#include <prapancha/server/model.h>
#include <prapancha/server/persistence/persistence.h>

namespace mehara::prapancha {

    template<typename Persistence>
    class RegistrationController : public BaseController<RegistrationController<Persistence>> {
        using HashAlgorithmType = Persistence::ModelType::HashAlgorithmType;
        Persistence persistence_;

    public:
        explicit RegistrationController(Persistence persistence) : persistence_(std::move(persistence)) {}
        static constexpr std::string_view ControllerName = "registration";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(auto &&ctx, auto &&sender) {
            http::Response response;
            response.set_header("Content-Type", "text/html; charset=utf-8");
            auto json_object_opt = codec::BinaryCodec<boost::json::object>::decode(ctx.request.body);
            if (!json_object_opt) {
                response.status = http::Status::BadRequest;
                response.body = "प्रपञ्च — Prapancha: Invalid Input!";
                return sender(std::move(response));
            }
            auto *username_ptr = json_object_opt->if_contains("username");
            auto *password_ptr = json_object_opt->if_contains("password");
            if (!username_ptr || !password_ptr || !username_ptr->is_string() || !password_ptr->is_string()) {
                response.status = http::Status::UnprocessableEntity;
                response.body = "प्रपञ्च — Prapancha: Invalid Input!";
                return sender(std::move(response));
            }
            std::string username{username_ptr->as_string()};
            std::string password{password_ptr->as_string()};
            auto password_binding = security::Hasher::hash<HashAlgorithmType>(password);
            if (!password_binding) {
                auto &error = password_binding.error();
                Loggers::App().log_error([&] {
                    return error.description()
                            .transform([&](const auto &description) {
                                return std::format("Failed to generate PasswordBinding. ErrorCode({}): {}",
                                                   error.code(), description);
                            })
                            .value_or(std::format("Failed to generate PasswordBinding. ErrorCode({})", error.code()));
                });
                response.status = http::Status::InternalServerError;
                response.body = "प्रपञ्च — Prapancha: Internal Server Error!";
                return sender(std::move(response));
            }
            auto user_identity = UserIdentity<HashAlgorithmType>::create({username, *password_binding, false});
            persistence_.save(user_identity);
            Loggers::App().log_info([&] { return std::format("Registration Successful! Username={}", username); });
            response.status = {http::Status::Created};
            response.body = "प्रपञ्च — Prapancha: Registration Successful!";
            return sender(std::move(response));
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
