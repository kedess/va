#pragma once

#include "../source/source.h"
#include <thread>

/*
 * Граф состояний объекта Capture
 * CREATED -----> PENDING -----> RUNNING -----> PENDING
 *                  |
 *                  |
 *                  v
 *                ERROR
 */

namespace va {
    enum class StreamStateEnum { Created, Pending, Running, Error };

    class CaptureBackend {
    public:
        explicit CaptureBackend(const Source &source) : source_(source) {
        }
        virtual ~CaptureBackend() = default;

    public:
        StreamStateEnum state_ = StreamStateEnum::Created;
        Source source_;
    };

    template <typename T> class Capture final {
    public:
        explicit Capture(const Source &source) : backend_(source) {
        }
        Capture(const Capture &) = delete;
        Capture &operator=(const Capture &) = delete;
        Capture(Capture &&) = delete;
        Capture &operator=(Capture &&) = delete;
        void run(std::stop_token stoken, const std::string &prefix_path, int64_t duration) {
            backend_.run(stoken, prefix_path, duration);
        }

    private:
        T backend_;
    };
} // namespace va