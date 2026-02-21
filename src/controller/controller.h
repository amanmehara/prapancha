//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_CONTROLLER_H
#define PRAPANCHA_CONTROLLER_H

#include <concepts>
#include <memory>
#include <string_view>

#include <drogon/drogon.h>

#include "../codec.h"
#include "../persistence.h"
#include "../policy/mapping.h"
#include "../policy/service.h"
#include "../uuid.h"

namespace mehara::prapancha {

    template<typename T>
    concept Controller = requires(T t) {
        { T::ControllerName } -> std::convertible_to<std::string_view>;
        typename std::tuple_size<typename T::RequiredTraits>::type;
    };

    template<typename T>
    class BaseController : std::enable_shared_from_this<T> {
    public:
        void dispatch(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            static_assert(Controller<T>, "Controller concept not satisfied.");
            LOG_INFO << "Dispatch [" << T::ControllerName << "] " << request->getMethodString() << " "
                     << request->path() << " (" << request->bodyLength() << " bytes)";
            using namespace policy;
            using Traits = T::RequiredTraits;

            auto runner = [this, callback = std::move(callback)]<size_t I>(this auto &&self, auto &&context) {
                if constexpr (I == std::tuple_size_v<Traits>) {
                    static_cast<T *>(this)->handle(std::forward<decltype(context)>(context), std::move(callback));
                } else {
                    using NextTrait = std::tuple_element_t<I, Traits>;
                    auto res = PolicyFor<NextTrait>::execute(std::forward<decltype(context)>(context));
                    if (!res) {
                        callback(res.error());
                        return;
                    }
                    self.template operator()<I + 1>(std::move(res.value()));
                }
            };
            runner.template operator()<0>(request);
        }
    };

    class RootController : public BaseController<RootController> {
    public:
        static constexpr std::string_view ControllerName = "root";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(const auto &context, drogon::AdviceCallback &&callback)
            requires policy::HasRequest<decltype(context)>
        {
            const auto response = drogon::HttpResponse::newHttpResponse();
            response->setStatusCode(drogon::k200OK);
            response->setContentTypeCode(drogon::CT_TEXT_HTML);
            response->setBody("प्रपञ्च — Prapancha!");
            callback(response);
        }
    };


    template<typename T, Model M, Persistence<M> P, template<typename> typename C>
        requires Codec<C<M>, M> && Codec<C<std::vector<M>>, std::vector<M>>
    class ModelController : public BaseController<T> {
    protected:
        P _persistence;

    public:
        explicit ModelController(P persistence) : _persistence(std::move(persistence)) {}

        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(const auto &context, drogon::AdviceCallback &&callback)
            requires policy::HasRequest<decltype(context)> {
            auto request = context.request;
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

    template<Persistence<Author> P, template<typename> typename C>
    class AuthorController : public ModelController<AuthorController<P, C>, Author, P, C> {
    public:
        static constexpr std::string_view ControllerName = "AuthorController";
        using ModelController<AuthorController, Author, P, C>::ModelController;

        void on_create(const drogon::HttpRequestPtr &, drogon::AdviceCallback &&callback) {
            callback(drogon::HttpResponse::newHttpResponse(drogon::k405MethodNotAllowed, drogon::CT_TEXT_PLAIN));
        }

        void on_delete(const drogon::HttpRequestPtr &, drogon::AdviceCallback &&callback) {
            callback(drogon::HttpResponse::newHttpResponse(drogon::k405MethodNotAllowed, drogon::CT_TEXT_PLAIN));
        }
    };

    template<Persistence<Post> P, template<typename> typename C>
    class PostController : public ModelController<PostController<P, C>, Post, P, C> {
    public:
        static constexpr std::string_view ControllerName = "PostController";
        using ModelController<PostController, Post, P, C>::ModelController;
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_CONTROLLER_H
