#pragma once
#include <memory>
extern "C" {
#include <libavformat/avformat.h>
}

namespace va {
    void release_avpacket(AVPacket *pkt);

    inline std::unique_ptr<AVPacket, void (*)(AVPacket *pkt)> va_avpacket_alloc() {
        auto ptr = av_packet_alloc();
        if (ptr == nullptr) {
            throw std::runtime_error("failed to create AVPacket");
        }
        return std::unique_ptr<AVPacket, void (*)(AVPacket * pkt)>(ptr, release_avpacket);
    }
} // namespace va