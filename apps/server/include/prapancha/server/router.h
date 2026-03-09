//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_SERVER_ROUTER_H_
#define PRAPANCHA_SERVER_ROUTER_H_

#include <algorithm>
#include <cstddef>
#include <string_view>

#include <prapancha/server/controller/controller_provider.h>
#include <prapancha/server/http.h>

namespace mehara::prapancha {

    template<std::size_t N>
    struct Path {
        char data[N]{};

        constexpr Path(const char (&str)[N]) { std::copy_n(str, N, data); }

        [[nodiscard]] constexpr std::string_view view() const noexcept { return {data, N > 0 ? N - 1 : 0}; }
    };

    template<std::size_t N>
    Path(const char (&)[N]) -> Path<N>;

    template<Path Path, http::Method Verb, auto Handler>
        requires requires(http::Request &request) { Handler(std::move(request), [](http::Response &&) {}); }
    struct Route {
        static constexpr std::string_view path = Path.view();
        static constexpr http::Method method = Verb;

        template<typename Request, typename Responder>
            requires std::derived_from<std::decay_t<Request>, http::Request> &&
                     std::invocable<Responder, http::Response &&> &&
                     std::same_as<std::invoke_result_t<Responder, http::Response &&>, void>
        static void execute(Request &&request, Responder &&responder) {
            Handler(std::forward<Request>(request), std::forward<Responder>(responder));
        }
    };

    template<typename... Routes>
    struct Router {
        template<typename Req, typename Send>
        static void dispatch(Req &&req, Send &&send) {
            std::string_view target = req.target;
            if (auto pos = target.find('?'); pos != std::string_view::npos) {
                target = target.substr(0, pos);
            }

            const http::Method method = req.method;
            const bool found = (((target == Routes::path && method == Routes::method) &&
                                 (Routes::execute(std::forward<Req>(req), std::forward<Send>(send)), true)) ||
                                ...);
            if (!found) {
                ControllerProvider::void_controller(std::forward<Req>(req), std::forward<Send>(send));
            }
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_ROUTER_H_
