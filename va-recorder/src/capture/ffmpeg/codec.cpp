#include "codec.h"
#include <boost/log/trivial.hpp>
#include <format>

namespace va {
    const AVCodecParameters *stream_video_params(AVFormatContext *ctx, const std::string &id, int &video_idx) {
        for (size_t idx = 0; idx < ctx->nb_streams; ++idx) {
            auto codec_params = ctx->streams[idx]->codecpar;
            auto media_type = codec_params->codec_type;
            if (media_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
                video_idx = static_cast<int>(idx);
                auto info_msg = std::format("video stream {}, width {}, height {}, codec {}", id, codec_params->width,
                                            codec_params->height, avcodec_get_name(codec_params->codec_id));
                BOOST_LOG_TRIVIAL(info) << info_msg;
                return codec_params;
            }
        }
        return nullptr;
    }
} // namespace va