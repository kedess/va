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
        void stop_app();
        void wait_stop_app();

    private:
        std::condition_variable is_stop_condvar_;
        std::mutex is_stop_mutex_;
        std::atomic_flag is_stop_ = ATOMIC_FLAG_INIT;
    };
} // namespace va