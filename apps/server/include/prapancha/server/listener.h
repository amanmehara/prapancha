//
// Created by Aman Mehara on 09/03/26.
//

#ifndef PRAPANCHA_SERVER_LISTENER_H_
#define PRAPANCHA_SERVER_LISTENER_H_

#include <memory>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>

#include <prapancha/server/session.h>

namespace mehara::prapancha {

    template<typename Router>
    class Listener : public std::enable_shared_from_this<Listener<Router>> {
        boost::asio::io_context &ioc_;
        boost::asio::ip::tcp::acceptor acceptor_;

    public:
        Listener(boost::asio::io_context &ioc, const boost::asio::ip::tcp::endpoint &endpoint) :
            ioc_(ioc), acceptor_(boost::asio::make_strand(ioc)) {
            boost::beast::error_code ec;
            acceptor_.open(endpoint.protocol(), ec);
            acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
            acceptor_.bind(endpoint, ec);
            acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        }

        void run() { do_accept(); }

    private:
        void do_accept() {
            acceptor_.async_accept(boost::asio::make_strand(ioc_), [self = this->shared_from_this()](
                                                                           auto ec,
                                                                           boost::asio::ip::tcp::socket socket) {
                if (!ec) {
                    boost::beast::error_code endpoint_ec;
                    const auto remote = socket.remote_endpoint(endpoint_ec);
                    if (!endpoint_ec) {
                        Loggers::App().log_info("Connection from {}:{}", remote.address().to_string(), remote.port());
                    } else {
                        Loggers::App().log_warn("प्रपञ्च — Prapancha: Connection accepted but endpoint unreachable: {}",
                                                endpoint_ec.message());
                    }
                    std::make_shared<Session<Router>>(std::move(socket))->run();
                } else if (ec != boost::asio::error::operation_aborted) {
                    Loggers::App().log_error("प्रपञ्च — Prapancha: Accept failed [{}]: {}", ec.value(), ec.message());
                }
                if (ec != boost::asio::error::operation_aborted) {
                    self->do_accept();
                }
            });
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_LISTENER_H_
