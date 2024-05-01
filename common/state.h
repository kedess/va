#pragma once

#include <atomic>
#include <boost/log/trivial.hpp>
#include <boost/version.hpp>
#include <condition_variable>
#include <mutex>

namespace va {
    class StateApp final {
    public:
        StateApp() {
        }
        StateApp(const StateApp &) = delete;
        StateApp &operator=(const StateApp &) = delete;
        StateApp(StateApp &&) = delete;
        StateApp &operator=(StateApp &&) = delete;
        void stop_app() {
            std::unique_lock<std::mutex> lock(is_stop_mutex_);
            is_stop_.test_and_set(std::memory_order_acquire);
            is_stop_condvar_.notify_one();
        }
        void wait_stop_app() {
            std::unique_lock<std::mutex> lock(is_stop_mutex_);
            while (!is_stop_.test_and_set()) {
                is_stop_condvar_.wait(lock);
            }
        }

    private:
        std::condition_variable is_stop_condvar_;
        std::mutex is_stop_mutex_;
        std::atomic_flag is_stop_ = ATOMIC_FLAG_INIT;
    };
} // namespace va