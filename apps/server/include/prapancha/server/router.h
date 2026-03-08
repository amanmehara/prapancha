//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_SERVER_ROUTER_H_
#define PRAPANCHA_SERVER_ROUTER_H_

#include <algorithm>
#include <cstddef>
#include <string_view>

#include <prapancha/server/http.h>

namespace mehara::prapancha {

    template<std::size_t N>
    struct StaticPath {
        char data[N]{};

        constexpr StaticPath(const char (&str)[N]) {
            std::copy_n(str, N, data);
        }

        [[nodiscard]] constexpr std::string_view view() const noexcept {
            return {data, N > 0 ? N - 1 : 0};
        }
    };

    template<std::size_t N>
    StaticPath(const char (&)[N]) -> StaticPath<N>;

    template<StaticPath Path, http::Method Verb, auto HandlerFunc>
    struct Route {
        static constexpr std::string_view path = Path.view();
        static constexpr http::Method method = Verb;

        template<typename Req, typename Send>
        static void execute(Req&& req, Send&& send) {
            HandlerFunc(std::forward<Req>(req), std::forward<Send>(send));
        }
    };

    template<typename... Routes>
    struct StaticRouter {
        template<typename Req, typename Send>
        static void dispatch(Req&& req, Send&& send) {
            std::string_view target = req.target;
            if (auto pos = target.find('?'); pos != std::string_view::npos) {
                target = target.substr(0, pos);
            }

            const http::Method req_method = req.method;

            bool found = ((
                (target == Routes::path && req_method == Routes::method) ?
                (Routes::execute(std::forward<Req>(req), std::forward<Send>(send)), true) :
                false
            ) || ...);

            if (!found) {
                send_404(std::forward<Req>(req), std::forward<Send>(send));
            }
        }

    private:
        template<typename Req, typename Send>
        static void send_404(Req&& req, Send&& send) {
            http::Response res{http::Status::not_found};
            res.set_header("Content-Type", "text/html; charset=utf-8");
            res.body = "404 Not Found!";
            std::forward<Send>(send)(std::move(res));
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_ROUTER_H_
