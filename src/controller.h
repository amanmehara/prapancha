//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_CONTROLLER_H
#define PRAPANCHA_CONTROLLER_H

#include <concepts>
#include <memory>
#include <string_view>

#include <drogon/drogon.h>

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

    template<typename T, Model M, PersistencePolicy<M> P>
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
                callback(drogon::HttpResponse::newHttpJsonResponse(serialize_vector(items)));
            } else {
                auto item = _persistence.load(UUID::from_hex(id_str));
                item ? callback(drogon::HttpResponse::newHttpJsonResponse(item->to_json()))
                     : callback(drogon::HttpResponse::newNotFoundResponse());
            }
        }

        void on_create(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            auto json = request->getJsonObject();
            if (!json) {
                callback(drogon::HttpResponse::newHttpResponse(drogon::k400BadRequest, drogon::CT_TEXT_PLAIN));
                return;
            }
            M model = M::from_json(*json);
            _persistence.save(model);
            callback(drogon::HttpResponse::newHttpResponse(drogon::k201Created, drogon::CT_TEXT_PLAIN));
        }

        void on_delete(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            const auto id_str = request->getParameter("id");
            if (!id_str.empty() && _persistence.remove(UUID::from_hex(id_str))) {
                callback(drogon::HttpResponse::newHttpResponse());
            } else {
                callback(drogon::HttpResponse::newNotFoundResponse());
            }
        }

        void on_update(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback) {
            const auto id_str = request->getParameter("id");
            auto json = request->getJsonObject();
            if (id_str.empty() || !json) {
                callback(drogon::HttpResponse::newHttpResponse(drogon::k400BadRequest, drogon::CT_TEXT_PLAIN));
                return;
            }

            auto existing = _persistence.load(UUID::from_hex(id_str));
            if (existing) {
                existing->update_from_json(*json);
                _persistence.save(*existing);
                callback(drogon::HttpResponse::newHttpResponse());
            } else {
                callback(drogon::HttpResponse::newNotFoundResponse());
            }
        }
    };

    class RootController : public BaseController<RootController> {
    public:
        static constexpr std::string_view ControllerName = "root";

        static void handle(const drogon::HttpRequestPtr &request, drogon::AdviceCallback &&callback);
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_CONTROLLER_H
