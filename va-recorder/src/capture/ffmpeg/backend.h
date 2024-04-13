#pragma once

#include "../capture.h"
#include "avformat.h"
#include <boost/log/trivial.hpp>
#include <chrono>
#include <format>
#include <thread>

namespace {

    inline int check_latency_read_packet(void *obj);
} // namespace

namespace va {
    class FFMPEGCapture final : CaptureBackend {
    public:
        explicit FFMPEGCapture(const Source &source) : CaptureBackend(source) {
        }
        FFMPEGCapture(const FFMPEGCapture &) = delete;
        FFMPEGCapture &operator=(const FFMPEGCapture &) = delete;
        FFMPEGCapture(FFMPEGCapture &&) = delete;
        FFMPEGCapture &operator=(FFMPEGCapture &&) = delete;
        void run(std::stop_token stoken, const std::string &prefix_path, int64_t duration);
        std::chrono::steady_clock::time_point timePoint() const {
            return time_point_;
        }
        const Source &source() const & {
            return source_;
        }

    private:
        void update_time_point() {
            time_point_ = std::chrono::steady_clock::now();
        }
        std::chrono::steady_clock::time_point time_point_ = std::chrono::steady_clock::now();
    };
} // namespace va

namespace {
    int check_latency_read_packet(void *obj) {
        auto self = static_cast<va::FFMPEGCapture *>(obj);
        auto tp = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(tp - self->timePoint()).count() > 10) {
            auto info_msg = std::format("timeout video source ({})", self->source().id());
            BOOST_LOG_TRIVIAL(info) << info_msg;
            return 1;
        }
        return 0;
    }
} // namespace