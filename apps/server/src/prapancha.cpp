//
// Created by Aman Mehara on 01/02/26.
//

#include "../include/prapancha/server/prapancha.h"

#include <future>

#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <prapancha/server/configuration.h>
#include <prapancha/server/logger_registry.h>
#include <prapancha/server/router.h>

namespace mehara::prapancha {

    class Session : public std::enable_shared_from_this<Session> {
        boost::asio::ip::tcp::socket socket_;
        std::shared_ptr<Router> router_;
        boost::beast::flat_buffer buffer_;
        boost::beast::http::request<boost::beast::http::string_body> req_;

    public:
        Session(boost::asio::ip::tcp::socket &&socket, std::shared_ptr<Router> router) :
            socket_(std::move(socket)), router_(std::move(router)) {}

        void run() { do_read(); }

    private:
        void do_read() {
            req_ = {};
            boost::beast::http::async_read(socket_, buffer_, req_,
                                           boost::asio::bind_executor(socket_.get_executor(),
                                                                      [self = shared_from_this()](auto ec, auto bytes) {
                                                                          self->on_read(ec, bytes);
                                                                      }));
        }

        void on_read(boost::beast::error_code ec, std::size_t) {
            if (ec) {
                return;
            }
            router_->dispatch(std::move(req_), [self = shared_from_this()](auto &&res) {
                auto sp = std::make_shared<std::decay_t<decltype(res)>>(std::forward<decltype(res)>(res));
                boost::beast::http::async_write(
                        self->socket_, *sp, [self, sp](boost::beast::error_code ec, std::size_t) {
                            if (!ec) {
                                self->socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
                            }
                        });
            });
        }
    };

    class Listener : public std::enable_shared_from_this<Listener> {
        boost::asio::io_context &ioc_;
        boost::asio::ip::tcp::acceptor acceptor_;
        std::shared_ptr<Router> router_;

    public:
        Listener(boost::asio::io_context &ioc, const boost::asio::ip::tcp::endpoint &endpoint,
                 std::shared_ptr<Router> router) :
            ioc_(ioc), acceptor_(boost::asio::make_strand(ioc)), router_(std::move(router)) {
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
                                   [self = shared_from_this()](auto ec, boost::asio::ip::tcp::socket socket) {
                                       if (!ec)
                                           std::make_shared<Session>(std::move(socket), self->router_)->run();
                                       self->do_accept();
                                   });
        }
    };

    void run(int argc, char *argv[]) {
        configuration::initialize(configuration::from_cli(argc, argv));
        const auto &config = *configuration::Active;
        int thread_count = config.network.thread_count;
        if (thread_count == 0) {
            thread_count = std::thread::hardware_concurrency();
            if (thread_count == 0) {
                thread_count = 1;
            }
        }
        boost::asio::io_context io_context{thread_count};
        auto router = std::make_shared<Router>(io_context, &config);
        router->register_routes();
        auto endpoint = boost::asio::ip::tcp::endpoint{boost::asio::ip::make_address(config.network.host),
                                                       static_cast<unsigned short>(config.network.port)};
        std::make_shared<Listener>(io_context, endpoint, router)->run();
        std::promise<int> shutdown_promised;
        auto shutdown_future = shutdown_promised.get_future();
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](const boost::system::error_code &ec, int signal_number) {
            if (ec == boost::asio::error::operation_aborted)
                return;
            Loggers::App().log_info("प्रपञ्च — Prapancha: Signal {} received.", signal_number);
            io_context.stop();
            static std::atomic<bool> signaled{false};
            if (!signaled.exchange(true)) {
                shutdown_promised.set_value(signal_number);
            }
        });
        Loggers::App().log_info("प्रपञ्च — Prapancha: Starting on http://{}:{} ({} executors).", config.network.host,
                                config.network.port, thread_count);
        std::vector<std::thread> executors;
        executors.reserve(thread_count);
        for (auto i = 0; i < thread_count; ++i) {
            executors.emplace_back([&io_context] { io_context.run(); });
        }
        int signal_number = shutdown_future.get();
        Loggers::App().log_info("प्रपञ्च — Prapancha: Signal {} acknowledged.", signal_number);
        for (auto &thread: executors) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        Loggers::App().log_info("प्रपञ्च — Prapancha: Stopping.");
    }

} // namespace mehara::prapancha
