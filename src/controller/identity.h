//
// Created by Aman Mehara on 20/02/26.
//

#ifndef PRAPANCHA_CONTROLLER_IDENTITY_H
#define PRAPANCHA_CONTROLLER_IDENTITY_H

#include "controller.h"

#include "../policy/mapping.h"
#include "../security/hasher.h"

namespace mehara::prapancha {

    template<typename P>
    static std::optional<UserIdentity> find_user_by_username(P &persistence, const std::string_view username) {
        auto users = persistence.all();
        auto it = std::find_if(users.begin(), users.end(),
                               [&](const UserIdentity &u) { return u.state.username == username; });
        if (it == users.end()) {
            return std::nullopt;
        }
        return std::make_optional(std::move(*it));
    }

    template<Persistence<UserIdentity> P, template<typename> typename C>
    class RegistrationController : public BaseController<RegistrationController<P, C>> {
        P _persistence;

    public:
        explicit RegistrationController(P persistence) : _persistence(std::move(persistence)) {}

        static constexpr std::string_view ControllerName = "RegistrationController";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(const auto &context, drogon::AdviceCallback &&callback) {
            const auto json = context.request->getJsonObject();
            if (!json || !json->isMember("username") || !json->isMember("password")) {
                callback(drogon::HttpResponse::newHttpResponse(drogon::k400BadRequest, drogon::CT_TEXT_PLAIN));
                return;
            }
            const std::string username = (*json)["username"].asString();
            const std::string password = (*json)["password"].asString();
            if (find_user_by_username(_persistence, username)) {
                auto res = drogon::HttpResponse::newHttpResponse(drogon::k409Conflict, drogon::CT_TEXT_PLAIN);
                res->setBody("Username already exists.");
                callback(res);
                return;
            }
            auto binding = security::Hasher::generate(password);
            if (!binding) {
                callback(drogon::HttpResponse::newHttpResponse(drogon::k500InternalServerError, drogon::CT_TEXT_PLAIN));
                return;
            }
            auto new_user = UserIdentity::create({username, std::move(*binding), false});
            _persistence.save(new_user);
            auto response = drogon::HttpResponse::newHttpResponse(drogon::k201Created, drogon::CT_TEXT_PLAIN);
            response->setBody("REGISTERED!");
            callback(response);
        }
    };

    template<Persistence<UserIdentity> P, template<typename> typename C>
    class LoginController : public BaseController<LoginController<P, C>> {
        P _persistence;

    public:
        explicit LoginController(P persistence) : _persistence(std::move(persistence)) {}

        static constexpr std::string_view ControllerName = "LoginController";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(const auto &context, drogon::AdviceCallback &&callback) {
            const auto json = context.request->getJsonObject();
            if (!json) {
                callback(drogon::HttpResponse::newHttpResponse(drogon::k400BadRequest, drogon::CT_TEXT_PLAIN));
                return;
            }

            const std::string username = (*json)["username"].asString();
            const std::string password = (*json)["password"].asString();

            auto user = find_user_by_username(_persistence, username);

            if (user && security::Hasher::verify(password, user->state.password_binding)) {
                auto session = context.request->session();
                session->insert("identity", policy::WithIdentity{user->id, user->state.username, "member"});
                const auto response = drogon::HttpResponse::newHttpResponse(drogon::k200OK, drogon::CT_TEXT_PLAIN);
                response->setBody("LOGGED IN!");
                callback(response);
            } else {
                const auto response =
                        drogon::HttpResponse::newHttpResponse(drogon::k401Unauthorized, drogon::CT_TEXT_PLAIN);
                response->setBody("UNAUTHORIZED: Invalid username or password.");
                callback(response);
            }
        }
    };

    template<Persistence<UserIdentity> P, template<typename> typename C>
    class LogoutController : public BaseController<LogoutController<P, C>> {
        P _persistence;

    public:
        explicit LogoutController(P persistence) : _persistence(std::move(persistence)) {}

        static constexpr std::string_view ControllerName = "LogoutController";
        using RequiredTraits = std::tuple<policy::WithRequest, policy::WithIdentity>;

        void handle(const auto &context, drogon::AdviceCallback &&callback) {
            auto session = context.request->session();
            session->clear();
            const auto response = drogon::HttpResponse::newHttpResponse(drogon::k200OK, drogon::CT_TEXT_PLAIN);
            response->setBody("LOGGED OUT!");
            callback(response);
        }
    };

    template<Persistence<UserIdentity> P, template<typename> typename C>
    class DeregistrationController : public BaseController<DeregistrationController<P, C>> {
        P _persistence;

    public:
        explicit DeregistrationController(P persistence) : _persistence(std::move(persistence)) {}

        static constexpr std::string_view ControllerName = "DeregistrationController";
        using RequiredTraits = std::tuple<policy::WithRequest, policy::WithIdentity>;

        void handle(const auto &context, drogon::AdviceCallback &&callback) {
            if (_persistence.remove(context.id)) {
                const auto response = drogon::HttpResponse::newHttpResponse(drogon::k200OK, drogon::CT_TEXT_PLAIN);
                response->setBody("ACCOUNT DELETED.");
                callback(response);
            } else {
                callback(drogon::HttpResponse::newHttpResponse(drogon::k500InternalServerError, drogon::CT_TEXT_PLAIN));
            }
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_CONTROLLER_IDENTITY_H
