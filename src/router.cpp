//
// Created by Aman Mehara on 01/02/26.
//

#include "router.h"

#include <boost/beast/http.hpp>

#include "codec.h"
#include "controller/controller.h"

namespace mehara::prapancha {

    Router::Router(boost::asio::io_context &ioc, const configuration::Configuration *configuration) :
        ioc_(ioc), configuration_(configuration) {
        register_routes();
    }

    template<typename T>
        requires std::is_base_of_v<BaseController<T>, T>
    Handler Router::to_handler(const std::shared_ptr<T> &controller) {
        using StringRequest = boost::beast::http::request<boost::beast::http::string_body>;
        using StringResponse = boost::beast::http::response<boost::beast::http::string_body>;
        using ResponseSender = std::function<void(StringResponse)>;
        return [controller](StringRequest &&req, ResponseSender &&send) {
            controller->dispatch(std::move(req), std::move(send));
        };
    }

    std::string Router::make_route_key(boost::beast::http::verb method, std::string_view target) const {
        std::string path(target);
        if (auto pos = path.find('?'); pos != std::string::npos) {
            path = path.substr(0, pos);
        }
        return std::string(boost::beast::http::to_string(method)) + " " + path;
    }

    void Router::dispatch(boost::beast::http::request<boost::beast::http::string_body> &&req,
                          std::function<void(boost::beast::http::response<boost::beast::http::string_body>)> &&send) {

        const std::string key = make_route_key(req.method(), req.target());
        if (auto it = routes_.find(key); it != routes_.end()) {
            it->second(std::move(req), std::move(send));
        } else {
            boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::not_found,
                                                                              req.version()};
            res.set(boost::beast::http::field::server, "Prapancha");
            res.prepare_payload();
            send(std::move(res));
        }
    }

    void Router::register_routes() {
        const auto root_controller = std::make_shared<RootController>();
        routes_["GET /"] = to_handler(root_controller);
    }

} // namespace mehara::prapancha
