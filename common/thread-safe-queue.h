#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace va {

    template <typename T> class ThreadSafeQueue {
    public:
        ThreadSafeQueue() {
        }
        ThreadSafeQueue(const ThreadSafeQueue &other) : std::lock_guard<std::mutex>(other.mutex), queue_(other.queue_) {
        }

        ThreadSafeQueue &operator=(const ThreadSafeQueue &other) = delete;

        void push(T new_value) {
            std::lock_guard<std::mutex> lock{mutex_};
            queue_.push(new_value);
            cond_.notify_one();
        }

        void wait_and_pop(T &value) {
            std::unique_lock<std::mutex> lock{mutex_};
            cond_.wait(lock, [this] { return !queue_.empty(); });
            value = queue_.front();
            queue_.pop();
        }

        std::shared_ptr<T> wait_and_pop() {
            std::unique_lock<std::mutex> lock{mutex_};
            cond_.wait(lock, [this] { return !queue_.empty(); });
            std::shared_ptr<T> res{std::make_shared<T>(queue_.front())};
            queue_.pop();
            return res;
        }

        bool try_pop(T &value) {
            std::lock_guard<std::mutex> lock{mutex_};
            if (queue_.empty())
                return false;
            value = queue_.front();
            queue_.pop();
            return true;
        }

        std::shared_ptr<T> try_pop() {
            std::lock_guard<std::mutex> lock{mutex_};
            if (queue_.empty())
                return std::shared_ptr<T>{};
            std::shared_ptr<T> res{std::make_shared<T>(queue_.front())};
            queue_.pop();
            return res;
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock{mutex_};
            return queue_.empty();
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock{mutex_};
            return queue_.size();
        }

    private:
        mutable std::mutex mutex_;
        std::condition_variable cond_;
        std::queue<T> queue_;
    };

} // namespace va