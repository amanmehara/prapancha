//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_SERVER_CONTROLLER_CONTROLLER_H_
#define PRAPANCHA_SERVER_CONTROLLER_CONTROLLER_H_

#include <concepts>
#include <memory>
#include <string_view>

#include <prapancha/server/codec/codec.h>
#include <prapancha/server/http.h>
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
        template<typename Sender>
        void dispatch(http::Request &&request, Sender &&sender) {
            static_assert(Controller<T>, "Controller concept not satisfied.");
            Loggers::App().log_info([&] {
                return std::format("Dispatch [{}] {} {} ({} bytes).", T::ControllerName,
                                   http::get_traits(request.method).name, request.target, request.body.size());
            });
            using namespace policy;
            using namespace boost::beast;
            using Traits = T::RequiredTraits;

            auto runner = [this, sender = std::forward<Sender>(sender)]<size_t I>(this auto &&self, auto &&ctx) {
                if constexpr (I == std::tuple_size_v<Traits>) {
                    static_cast<T *>(this)->handle(std::forward<decltype(ctx)>(ctx), std::move(sender));
                } else {
                    using NextTrait = std::tuple_element_t<I, Traits>;
                    auto res = PolicyFor<NextTrait>::execute(std::forward<decltype(ctx)>(ctx));
                    if (!res) {
                        sender(http::Response{res.error(), {}, "Policy Violation!"});
                        return;
                    }
                    std::forward<decltype(self)>(self).template operator()<I + 1>(std::move(*res));
                }
            };
            runner.template operator()<0>(std::move(request));
        }
    };

    class RootController : public BaseController<RootController> {
    public:
        static constexpr std::string_view ControllerName = "root";
        using RequiredTraits = std::tuple<policy::WithRequest>;

        void handle(auto &&ctx, auto &&sender) {
            http::Response res{http::Status::ok};
            res.set_header("Content-Type", "text/html; charset=utf-8");
            res.body = "प्रपञ्च — Prapancha!";
            sender(std::move(res));
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_CONTROLLER_CONTROLLER_H_
