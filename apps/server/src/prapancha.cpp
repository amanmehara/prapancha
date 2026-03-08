//
// Created by Aman Mehara on 01/02/26.
//

#include <prapancha/server/prapancha.h>

#include <future>

#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <prapancha/server/configuration.h>
#include <prapancha/server/controller/controller.h>
#include <prapancha/server/http.h>
#include <prapancha/server/logger_registry.h>
#include <prapancha/server/router.h>

namespace mehara::prapancha {

    static http::Request
    translate_request(boost::beast::http::request<boost::beast::http::vector_body<uint8_t>> &b_req) {
        http::Request p_req;
        p_req.target = std::string(b_req.target());
        p_req.version = b_req.version();

        switch (b_req.method()) {
            case boost::beast::http::verb::get:
                p_req.method = http::Method::Get;
                break;
            case boost::beast::http::verb::post:
                p_req.method = http::Method::Post;
                break;
            default:
                p_req.method = http::Method::Unknown;
        }

        for (auto const &field: b_req) {
            p_req.headers.push_back({std::string(field.name_string()), std::string(field.value())});
        }

        p_req.body = std::move(b_req.body());
        return p_req;
    }

    using ResponseSender = std::function<void(http::Response)>;

    struct AppHandlers {
        static void root_bridge(http::Request &&req, ResponseSender &&send) {
            static auto instance = std::make_shared<RootController>();
            instance->dispatch(std::move(req), std::forward<ResponseSender>(send));
        }

        static void status_handler(http::Request &&req, ResponseSender &&send) {
            http::Response res{http::Status::ok};
            res.set_header("Content-Type", "text/html; charset=utf-8");
            res.body = "प्रपञ्च — Prapancha: अनवरत। Alea iacta est!";
            send(std::move(res));
        }
    };

    using AppRouter = StaticRouter<Route<"/", http::Method::Get, AppHandlers::root_bridge>,
                                   Route<"/api/v1/status", http::Method::Get, AppHandlers::status_handler>>;

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

            // 1. Translate Beast -> Prapancha Request
            auto pra_req = translate_request(req_);
            auto version = req_.version();

            auto send = [self = this->shared_from_this(), version](http::Response &&pra_res) {
                auto b_res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();

                b_res->result(static_cast<boost::beast::http::status>(pra_res.status));
                b_res->version(version);

                for (const auto &h: pra_res.headers) {
                    b_res->set(h.name, h.value);
                }

                b_res->body() = std::move(pra_res.body);
                b_res->prepare_payload();

                boost::beast::http::async_write(
                        self->stream_, *b_res, [self, b_res](boost::beast::error_code ec, std::size_t) {
                            if (!ec) {
                                boost::system::error_code ignored_ec;
                                self->stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send,
                                                                ignored_ec);
                            }
                        });
            };

            Router::dispatch(std::move(pra_req), std::move(send));
        }
    };

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

    void run(int argc, char *argv[]) {
        configuration::initialize(configuration::from_cli(argc, argv));
        const auto &config = *configuration::Active;

        int thread_count = config.network.thread_count;
        if (thread_count == 0) {
            thread_count = std::max<int>(1, std::thread::hardware_concurrency());
        }

        boost::asio::io_context io_context{thread_count};

        auto endpoint = boost::asio::ip::tcp::endpoint{boost::asio::ip::make_address(config.network.host),
                                                       static_cast<unsigned short>(config.network.port)};

        std::make_shared<Listener<AppRouter>>(io_context, endpoint)->run();

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
            if (thread.joinable())
                thread.join();
        }

        Loggers::App().log_info("प्रपञ्च — Prapancha: Stopping.");
    }

} // namespace mehara::prapancha
