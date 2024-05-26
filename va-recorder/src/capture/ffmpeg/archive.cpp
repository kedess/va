#include "archive.h"
#include "../../utils.h"
#include "libswscale/swscale.h"

namespace va {
    void Archive::send_pkt(AVPacket *pkt, AVFormatContext *ctx_in) {
        if (!time_base_) {
            auto idx = pkt->stream_index;
            auto in_stream = ctx_in->streams[idx];
            time_base_ = in_stream->time_base;
        }
        if (pkt->pts != AV_NOPTS_VALUE) {
            current_pts_pkt_ = pkt->pts;
            if (start_pts_pkt_ == AV_NOPTS_VALUE) {
                start_pts_pkt_ = pkt->pts;
            }
        }
        auto is_key = pkt->flags & AV_PKT_FLAG_KEY;
        if (is_key && ctx_) {
            auto tp = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(tp - time_point_).count() > duration_file_) {
                av_interleaved_write_frame(ctx_.get(), nullptr);
                ctx_.reset(nullptr);
                BOOST_LOG_TRIVIAL(debug) << "file " << current_file_ << " recording has finished";
                auto time_base = time_base_.value();
                size_t extension_pos{current_file_.rfind(".ts")};
                auto tmp = std::string(current_file_.begin(), current_file_.end());
                fs::path path{tmp.replace(extension_pos, 3, ".meta")};
                std::ofstream ofs(path);
                ofs << "START_DATA" << std::endl;
                ofs << static_cast<double>((current_pts_pkt_ - start_pts_pkt_)) * time_base.num / time_base.den
                    << std::endl;
                ofs << codec_id() << std::endl;
                ofs << "END_DATA" << std::endl;
                start_pts_pkt_ = AV_NOPTS_VALUE;
            }
        }
        if (!ctx_) {
            time_point_ = std::chrono::steady_clock::now();
            auto secs =
                std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                    .count();
            auto intermediate_path = std::format("{}/{}/", prefix_path_, utils::format_datetime_to_path(secs));
            auto filename = std::format("{}/{}.ts", intermediate_path, secs);
            fs::create_directories(intermediate_path);
            ctx_ = va_avformat_alloc_output_context(filename.c_str(), video_params_);
            current_file_ = filename;
        }
        if (ctx_) {
            auto idx = pkt->stream_index;
            auto in_stream = ctx_in->streams[idx];
            auto out_stream = ctx_->streams[idx];
            pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
            pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
            pkt->duration = av_rescale_q(pkt->duration, in_stream->time_base, out_stream->time_base);
            pkt->pos = -1;
            av_interleaved_write_frame(ctx_.get(), pkt);
        }
    }
} // namespace va