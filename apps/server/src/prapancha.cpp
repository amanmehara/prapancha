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
#include <prapancha/server/http.h>
#include <prapancha/server/listener.h>
#include <prapancha/server/logger_registry.h>
#include <prapancha/server/routes.h>

namespace mehara::prapancha {

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
