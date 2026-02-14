//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_CONTROLLER_H
#define PRAPANCHA_CONTROLLER_H

#include <concepts>
#include <memory>
#include <string_view>

#include <drogon/drogon.h>

#include "codec.h"
#include "persistence.h"
#include "uuid.h"

namespace mehara::prapancha {
    template<typename T>
    concept Controller = requires(T t, const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
        { T::ControllerName } -> std::convertible_to<std::string_view>;
        { t.handle(request, std::move(callback)) } -> std::same_as<void>;
    };

    template<typename T>
    class BaseController : std::enable_shared_from_this<T> {
    public:
        void dispatch(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            static_assert(Controller<T>, "Controller concept not satisfied.");
            LOG_INFO << "Dispatching request to " << T::ControllerName << " handle.";
            static_cast<T *>(this)->handle(request, std::move(callback));
        }
    };

    class RootController : public BaseController<RootController> {
    public:
        static constexpr std::string_view ControllerName = "root";

        static void handle(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback);
    };

    template<typename T, Model M, PersistencePolicy<M> P, template<typename> typename C>
        requires Codec<C<M>, M> && Codec<C<std::vector<M>>, std::vector<M>>
    class ModelController : public BaseController<T> {
    protected:
        P _persistence;

    public:
        explicit ModelController(P persistence) : _persistence(std::move(persistence)) {}

        void handle(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            const auto method = request->method();
            auto derived = static_cast<T *>(this);

            switch (method) {
                case drogon::Get:
                    derived->on_get(request, std::move(callback));
                    break;
                case drogon::Post:
                    derived->on_create(request, std::move(callback));
                    break;
                case drogon::Put:
                    derived->on_update(request, std::move(callback));
                    break;
                case drogon::Delete:
                    derived->on_delete(request, std::move(callback));
                    break;
                default:
                    callback(drogon::HttpResponse::newNotFoundResponse());
                    break;
            }
        }

        void on_get(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            auto id_str = request->getParameter("id");
            if (id_str.empty()) {
                auto items = _persistence.all();
                callback(drogon::HttpResponse::newHttpJsonResponse(C<std::vector<M>>::encode(items)));
            } else {
                auto id_opt = UUID::from_hex(id_str);
                auto item = id_opt ? _persistence.load(*id_opt) : std::nullopt;
                item ? callback(drogon::HttpResponse::newHttpJsonResponse(C<M>::encode(*item)))
                     : callback(drogon::HttpResponse::newNotFoundResponse());
            }
        }

        void on_create(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            const auto body = request->getBody();
            if (body.empty()) {
                callback(drogon::HttpResponse::newHttpResponse(drogon::k400BadRequest, drogon::CT_TEXT_PLAIN));
                return;
            }
            auto model_opt = C<M>::decode(std::string(body));
            if (!model_opt) {
                callback(drogon::HttpResponse::newHttpResponse(drogon::k422UnprocessableEntity, drogon::CT_TEXT_PLAIN));
                return;
            }
            _persistence.save(*model_opt);
            callback(drogon::HttpResponse::newHttpResponse(drogon::k201Created, drogon::CT_TEXT_PLAIN));
        }

        void on_update(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            auto id_opt = UUID::from_hex(request->getParameter("id"));
            const auto body = request->getBody();
            if (!id_opt || body.empty()) {
                callback(drogon::HttpResponse::newHttpResponse(drogon::k400BadRequest, drogon::CT_TEXT_PLAIN));
                return;
            }
            auto update_opt = C<M>::decode(std::string(body));
            auto existing = _persistence.load(*id_opt);
            if (existing && update_opt) {
                _persistence.save(existing->patch(update_opt->state));
                callback(drogon::HttpResponse::newHttpResponse(drogon::k204NoContent, drogon::CT_TEXT_PLAIN));
            } else {
                callback(drogon::HttpResponse::newNotFoundResponse());
            }
        }

        void on_delete(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            const auto id_opt = UUID::from_hex(request->getParameter("id"));
            if (id_opt && _persistence.remove(*id_opt)) {
                callback(drogon::HttpResponse::newHttpResponse(drogon::k204NoContent, drogon::CT_TEXT_PLAIN));
            } else {
                callback(drogon::HttpResponse::newNotFoundResponse());
            }
        }
    };

    template<PersistencePolicy<Author> P, template<typename> typename C>
    class AuthorController : public ModelController<AuthorController<P, C>, Author, P, C> {
    public:
        static constexpr std::string_view ControllerName = "AuthorController";
        using ModelController<AuthorController, Author, P, C>::ModelController;
    };

    template<PersistencePolicy<Post> P, template<typename> typename C>
    class PostController : public ModelController<PostController<P, C>, Post, P, C> {
    public:
        static constexpr std::string_view ControllerName = "PostController";
        using ModelController<PostController, Post, P, C>::ModelController;
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_CONTROLLER_H
