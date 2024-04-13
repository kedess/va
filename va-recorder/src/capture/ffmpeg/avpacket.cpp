#include "avpacket.h"

#include <iostream>

namespace va {
    void release_avpacket(AVPacket *pkt) {
        av_packet_unref(pkt);
        av_packet_free(&pkt);
    }
} // namespace va