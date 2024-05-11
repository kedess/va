#pragma once
#include <string>
#include <vector>
extern "C" {
#include <libavformat/avformat.h>
}

namespace va {
    const AVCodecParameters *stream_video_params(AVFormatContext *ctx, const std::string &id, int &video_idx);
} // namespace va