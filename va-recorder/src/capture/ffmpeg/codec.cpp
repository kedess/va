#include "codec.h"
#include <boost/log/trivial.hpp>
#include <format>

namespace va {
    std::vector<const AVCodecParameters *> stream_params(AVFormatContext *ctx, const std::string &id) {
        std::vector<const AVCodecParameters *> params_list;
        for (size_t idx = 0; idx < ctx->nb_streams; ++idx) {
            auto codec_params = ctx->streams[idx]->codecpar;
            auto media_type = codec_params->codec_type;
            if (media_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
                auto info_msg = std::format("video stream {}, width {}, height {}, codec {}", id, codec_params->width,
                                            codec_params->height, avcodec_get_name(codec_params->codec_id));
                BOOST_LOG_TRIVIAL(info) << info_msg;
                params_list.push_back(codec_params);
            }
            if (media_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
                auto info_msg = std::format("audio stream {}, codec {}", id, avcodec_get_name(codec_params->codec_id));
                BOOST_LOG_TRIVIAL(info) << info_msg;
                params_list.push_back(codec_params);
            }
        }
        return params_list;
    }
} // namespace va