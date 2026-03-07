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
#include <prapancha/server/logger_registry.h>
#include <prapancha/server/router.h>

namespace mehara::prapancha {

    using StringRequest = boost::beast::http::request<boost::beast::http::string_body>;
    using StringResponse = boost::beast::http::response<boost::beast::http::string_body>;
    using ResponseSender = std::function<void(StringResponse)>;

    struct AppHandlers {
        static void root_bridge(StringRequest&& req, ResponseSender&& send) {
            static auto instance = std::make_shared<RootController>();
            instance->dispatch(std::move(req), std::move(send));
        }

        static void status_handler(StringRequest&& req, ResponseSender&& send) {
            StringResponse res{boost::beast::http::status::ok, req.version()};
            res.set(boost::beast::http::field::content_type, "text/html; charset=utf-8");
            res.body() = "प्रपञ्च — Prapancha: अनवरत। Alea iacta est!";
            res.prepare_payload();
            send(std::move(res));
        }
    };

    using AppRouter = StaticRouter<
        Route<"/", Method::get, AppHandlers::root_bridge>,
        Route<"/api/v1/status", Method::get, AppHandlers::status_handler>
    >;

    template<typename Router>
    class Session : public std::enable_shared_from_this<Session<Router>> {
        boost::asio::ip::tcp::socket socket_;
        boost::beast::flat_buffer buffer_;
        StringRequest req_;

    public:
        explicit Session(boost::asio::ip::tcp::socket&& socket) : socket_(std::move(socket)) {}
        void run() { do_read(); }

    private:
        void do_read() {
            req_ = {};
            boost::beast::http::async_read(socket_, buffer_, req_,
                boost::asio::bind_executor(socket_.get_executor(),
                    [self = this->shared_from_this()](auto ec, auto bytes) {
                        self->on_read(ec, bytes);
                    }));
        }

        void on_read(boost::beast::error_code ec, std::size_t) {
            if (ec) return;

            auto send = [self = this->shared_from_this()](StringResponse&& res) {
                auto sp = std::make_shared<StringResponse>(std::move(res));
                boost::beast::http::async_write(self->socket_, *sp,
                    [self, sp](boost::beast::error_code ec, std::size_t) {
                        if (!ec) {
                            boost::system::error_code ignore_ec;
                            self->socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ignore_ec);
                        }
                    });
            };

            Router::dispatch(std::move(req_), std::move(send));
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

        auto endpoint = boost::asio::ip::tcp::endpoint{
            boost::asio::ip::make_address(config.network.host),
            static_cast<unsigned short>(config.network.port)
        };

        std::make_shared<Listener<AppRouter>>(io_context, endpoint)->run();

        std::promise<int> shutdown_promised;
        auto shutdown_future = shutdown_promised.get_future();
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

        signals.async_wait([&](const boost::system::error_code &ec, int signal_number) {
            if (ec == boost::asio::error::operation_aborted) return;
            Loggers::App().log_info("प्रपञ्च — Prapancha: Signal {} received.", signal_number);
            io_context.stop();
            static std::atomic<bool> signaled{false};
            if (!signaled.exchange(true)) {
                shutdown_promised.set_value(signal_number);
            }
        });

        Loggers::App().log_info("प्रपञ्च — Prapancha: Starting on http://{}:{} ({} executors).",
            config.network.host, config.network.port, thread_count);

        std::vector<std::thread> executors;
        executors.reserve(thread_count);
        for (auto i = 0; i < thread_count; ++i) {
            executors.emplace_back([&io_context] { io_context.run(); });
        }

        int signal_number = shutdown_future.get();
        Loggers::App().log_info("प्रपञ्च — Prapancha: Signal {} acknowledged.", signal_number);

        for (auto &thread: executors) {
            if (thread.joinable()) thread.join();
        }

        Loggers::App().log_info("प्रपञ्च — Prapancha: Stopping.");
    }

} // namespace mehara::prapancha
