#include "state.h"

namespace va {
    void StateApp::stop_app() {
        std::unique_lock<std::mutex> lock(is_stop_mutex_);
        is_stop_.test_and_set(std::memory_order_acquire);
        is_stop_condvar_.notify_one();
    }
    void StateApp::wait_stop_app() {
        std::unique_lock<std::mutex> lock(is_stop_mutex_);
        while (!is_stop_.test()) {
            is_stop_condvar_.wait(lock);
        }
    }
} // namespace va