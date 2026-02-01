//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_CONTROLLER_H
#define PRAPANCHA_CONTROLLER_H

#include <concepts>
#include <memory>
#include <string_view>

#include <drogon/drogon.h>

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

} // namespace mehara::prapancha

#endif // PRAPANCHA_CONTROLLER_H
