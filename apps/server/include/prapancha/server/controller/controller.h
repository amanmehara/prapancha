//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_SERVER_CONTROLLER_CONTROLLER_H_
#define PRAPANCHA_SERVER_CONTROLLER_CONTROLLER_H_

#include <concepts>
#include <memory>
#include <string_view>

#include <boost/beast/http.hpp>

#include <prapancha/server/codec/codec.h>
#include <prapancha/server/logger_registry.h>
#include <prapancha/server/persistence/persistence.h>
#include <prapancha/server/policy/policy.h>
#include <prapancha/server/uuid.h>

namespace mehara::prapancha {

    template<typename T>
    concept Controller = requires(T t) {
        { T::ControllerName } -> std::convertible_to<std::string_view>;
        typename std::tuple_size<typename T::RequiredTraits>::type;
    };

    template<typename T>
    class BaseController : std::enable_shared_from_this<T> {
    public:

        using StringRequest = boost::beast::http::request<boost::beast::http::string_body>;
        using StringResponse = boost::beast::http::response<boost::beast::http::string_body>;
        using ResponseSender = std::function<void(StringResponse)>;

        void dispatch(StringRequest &&request, ResponseSender &&sender) {
            static_assert(Controller<T>, "Controller concept not satisfied.");
            Loggers::App().log_info([&] {
                return std::format("Dispatch [{}] {} {} ({} bytes).", T::ControllerName, request.method_string(),
                                   request.target(), request.body().size());
            });
            using namespace policy;
            using namespace boost::beast;
            using Traits = T::RequiredTraits;

            auto runner = [this, sender = std::move(sender), version = request.version()]<size_t I>(this auto &&self,
                                                                                                    auto &&ctx) {
                if constexpr (I == std::tuple_size_v<Traits>) {
                    static_cast<T *>(this)->handle(std::forward<decltype(ctx)>(ctx), std::move(sender));
                } else {
                    using NextTrait = std::tuple_element_t<I, Traits>;
                    auto res = PolicyFor<NextTrait>::execute(std::forward<decltype(ctx)>(ctx));
                    if (!res) {
                        http::response<http::string_body> error_res{res.error(), version};
                        error_res.prepare_payload();
                        sender(std::move(error_res));
                        return;
                    }
                    self.template operator()<I + 1>(std::move(*res));
                }
            };
            runner.template operator()<0>(std::move(request));
        }
    };

    class RootController : public BaseController<RootController> {
    public:
        static constexpr std::string_view ControllerName = "root";
        using RequiredTraits = std::tuple<policy::WithRequest<boost::beast::http::string_body>>;

        void handle(auto &&ctx, ResponseSender &&sender) {
            boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::ok,
                                                                              ctx.request.version()};
            res.set(boost::beast::http::field::content_type, "text/html; charset=utf-8");
            res.body() = "प्रपञ्च — Prapancha!";
            res.prepare_payload();
            sender(std::move(res));
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_CONTROLLER_CONTROLLER_H_
