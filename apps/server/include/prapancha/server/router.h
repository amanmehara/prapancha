//
// Created by Aman Mehara on 01/02/26.
//

#ifndef PRAPANCHA_SERVER_ROUTER_H_
#define PRAPANCHA_SERVER_ROUTER_H_

#include <functional>
#include <map>
#include <memory>

#include <boost/beast/http.hpp>

#include <prapancha/server/configuration.h>
#include <prapancha/server/controller/controller.h>

namespace mehara::prapancha {

    using Handler =
            std::function<void(boost::beast::http::request<boost::beast::http::string_body> &&,
                               std::function<void(boost::beast::http::response<boost::beast::http::string_body>)>)>;

    class Router {
    public:
        explicit Router(boost::asio::io_context &ioc, const configuration::Configuration *configuration);

        void register_routes();

        void dispatch(boost::beast::http::request<boost::beast::http::string_body> &&req,
                      std::function<void(boost::beast::http::response<boost::beast::http::string_body>)> &&send);

    private:
        template<typename T>
            requires std::is_base_of_v<BaseController<T>, T>
        Handler to_handler(const std::shared_ptr<T> &controller);

        std::string make_route_key(boost::beast::http::verb method, std::string_view target) const;

        boost::asio::io_context &ioc_;
        const configuration::Configuration *configuration_;
        std::map<std::string, Handler> routes_;
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_ROUTER_H_
