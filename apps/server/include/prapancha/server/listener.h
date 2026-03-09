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
            acceptor_.async_accept(boost::asio::make_strand(ioc_),
                                   [self = this->shared_from_this()](auto ec, boost::asio::ip::tcp::socket socket) {
                                       if (!ec) {
                                           std::make_shared<Session<Router>>(std::move(socket))->run();
                                       }
                                       self->do_accept();
                                   });
        }
    };

} // namespace mehara::prapancha

#endif // PRAPANCHA_SERVER_LISTENER_H_
