#include "archive.h"
#include "../../utils.h"
#include <chrono>

namespace va {
    void Archive::send_pkt(AVPacket *pkt, AVFormatContext *ctx_in) {
        if (pkt->pts != AV_NOPTS_VALUE) {
            current_pts_pkt_ = pkt->pts;
        }
        auto is_key = pkt->flags & AV_PKT_FLAG_KEY;
        if (is_key && ctx_) {
            auto tp = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(tp - time_point_).count() > duration_file_) {
                ctx_ = nullptr;
                BOOST_LOG_TRIVIAL(debug) << "file " << current_file_ << " recording has finished";
                if (start_pts_pkt_ != AV_NOPTS_VALUE && current_pts_pkt_ != AV_NOPTS_VALUE && time_base_) {
                    auto time_base = time_base_.value();
                    size_t start{current_file_.rfind(".ts")};
                    fs::path path{current_file_.replace(start, 3, ".stat")};
                    std::ofstream ofs(path);
                    ofs << "START_DATA" << std::endl;
                    ofs << static_cast<double>((current_pts_pkt_ - start_pts_pkt_)) * time_base.num / time_base.den
                        << std::endl;
                    ofs << "END_DATA";

                } else {
                    // TODO: Предполагаю длительность файла равна duration_file_
                    size_t start{current_file_.rfind(".ts")};
                    fs::path path{current_file_.replace(start, 3, ".stat")};
                    std::ofstream ofs(path);
                    ofs << static_cast<double>(duration_file_);
                }
                if (pkt->pts != AV_NOPTS_VALUE) {
                    start_pts_pkt_ = pkt->pts;
                }
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
            ctx_ = va_avformat_alloc_output_context(filename.c_str(), params_list_);
            current_file_ = filename;
        }
        auto idx = pkt->stream_index;
        auto in_stream = ctx_in->streams[idx];
        auto out_stream = ctx_->streams[idx];
        if (!time_base_) {
            time_base_ = in_stream->time_base;
        }
        if (start_pts_pkt_ == AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE) {
            start_pts_pkt_ = pkt->pts;
        }
        pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
        pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
        pkt->duration = av_rescale_q(pkt->duration, in_stream->time_base, out_stream->time_base);
        pkt->pos = -1;
        av_interleaved_write_frame(ctx_.get(), pkt);
    }
} // namespace va