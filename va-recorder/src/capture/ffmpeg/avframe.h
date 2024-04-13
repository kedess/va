#pragma once
#include <memory>
extern "C" {
#include <libavformat/avformat.h>
}

namespace va {
    void release_avframe(AVFrame *frame);

    inline std::unique_ptr<AVFrame, void (*)(AVFrame *pkt)> va_avframe_alloc() {
        auto ptr = av_frame_alloc();
        if (ptr == nullptr) {
            throw std::runtime_error("failed to create AVFrame");
        }
        return std::unique_ptr<AVFrame, void (*)(AVFrame * pkt)>(ptr, release_avframe);
    }
} // namespace va