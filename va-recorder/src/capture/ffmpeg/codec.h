#pragma once
#include <string>
#include <vector>
extern "C" {
#include <libavformat/avformat.h>
}

namespace va {
    std::vector<const AVCodecParameters *> stream_params(AVFormatContext *ctx, const std::string &id,
                                                         ssize_t &video_idx);
} // namespace va