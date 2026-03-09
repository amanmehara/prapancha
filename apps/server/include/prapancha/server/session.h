//
// Created by Aman Mehara on 09/03/26.
//

#ifndef PRAPANCHA_SERVER_SESSION_H_
#define PRAPANCHA_SERVER_SESSION_H_

#include <memory>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <prapancha/server/beast_adapter.h>
#include <prapancha/server/http.h>
#include <prapancha/server/logger_registry.h>

namespace mehara::prapancha {

    template<typename Router>
    class Session : public std::enable_shared_from_this<Session<Router>> {
        boost::beast::tcp_stream stream_;
        boost::beast::flat_buffer buffer_;
        boost::beast::http::request<boost::beast::http::vector_body<uint8_t>> req_;

    public:
        explicit Session(boost::asio::ip::tcp::socket &&socket) : stream_(std::move(socket)) {}

        void run() {
            boost::beast::http::async_read(
                    stream_, buffer_, req_,
                    boost::beast::bind_front_handler(&Session::on_read, this->shared_from_this()));
        }

    private:
        void on_read(boost::beast::error_code ec, std::size_t) {
            if (ec)
                return;

            if (req_.version() != 11) {
                Loggers::App().log_info("प्रपञ्च — Prapancha: Request version {} not supported.", req_.version());
                return;
            }
            auto request = http::from_beast(req_);
            auto send = [self = this->shared_from_this()](http::Response &&response) {
                auto beast_response = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(
                        http::to_beast(std::move(response)));

                boost::beast::http::async_write(self->stream_, *beast_response,
                                                [self, beast_response](boost::beast::error_code ec, std::size_t) {
                                                    if (!ec) {
                                                        boost::system::error_code ignored_ec;
                                                        self->stream_.socket().shutdown(
                                                                boost::asio::ip::tcp::socket::shutdown_send,
                                                                ignored_ec);
                                                    }
                                                });
            };

            Router::dispatch(std::move(request), std::move(send));
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_SESSION_H_
