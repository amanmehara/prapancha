//
// Created by Aman Mehara on 25/02/26.
//

#ifndef PRAPANCHA_LOGGING_ASYNC_SINK_H
#define PRAPANCHA_LOGGING_ASYNC_SINK_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <prapancha/logging/log_level.h>
#include <prapancha/logging/log_sink.h>

namespace mehara::prapancha::logging {

    template<typename T>
    class AsyncSink : public LogSink<AsyncSink<T>> {
    public:
        template<typename... Args>
        explicit AsyncSink(const bool enabled, Args &&...args) noexcept :
            enabled_(enabled), sink_(std::forward<Args>(args)...) {
            if (enabled_) {
                worker_ = std::jthread([this](const std::stop_token &st) { process_queue(st); });
            }
        }

        AsyncSink(const AsyncSink &) = delete;
        AsyncSink &operator=(const AsyncSink &) = delete;
        AsyncSink(AsyncSink &&) = delete;
        AsyncSink &operator=(AsyncSink &&) = delete;

        void write(LogLevel level, std::string_view msg) const noexcept {
            if (!enabled_) {
                sink_.write(level, msg);
                return;
            }
            {
                std::lock_guard lock(queue_mutex_);
                queue_.emplace(std::string(msg), level);
                in_flight_count_.fetch_add(1, std::memory_order_relaxed);
            }
            cv_.notify_one();
        }

        size_t in_flight_count() const noexcept { return in_flight_count_.load(std::memory_order_relaxed); }

    private:
        void process_queue(std::stop_token st) {
            while (true) {
                std::unique_lock lock(queue_mutex_);
                cv_.wait(lock, [this, &st] { return !queue_.empty() || st.stop_requested(); });
                if (st.stop_requested() && queue_.empty()) {
                    break;
                }
                if (!queue_.empty()) {
                    auto [msg, level] = std::move(queue_.front());
                    queue_.pop();
                    lock.unlock();
                    sink_.write(level, msg);
                    in_flight_count_.fetch_sub(1, std::memory_order_relaxed);
                }
            }
        }

        bool enabled_;
        T sink_;
        mutable std::queue<std::pair<std::string, LogLevel>> queue_;
        mutable std::mutex queue_mutex_;
        mutable std::condition_variable cv_;
        mutable std::atomic<size_t> in_flight_count_{0};
        std::jthread worker_;
    };

} // namespace mehara::prapancha::logging

#endif // PRAPANCHA_LOGGING_ASYNC_SINK_H
