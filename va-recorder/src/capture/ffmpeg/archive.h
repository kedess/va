#pragma once

#include "avformat.h"
#include <boost/log/trivial.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
}

namespace fs = std::filesystem;

namespace va {
    class Archive final {
    public:
        Archive(std::string &prefix_path, int64_t duration, const AVCodecParameters *video_params)
            : prefix_path_(prefix_path), duration_file_(duration), video_params_(video_params) {
        }
        ~Archive() {
            if (ctx_) {
                av_write_trailer(ctx_.get());
                BOOST_LOG_TRIVIAL(debug) << "file " << current_file_ << " recording has finished";
                auto time_base = time_base_.value();
                size_t extention_pos{current_file_.find(".ts")};
                auto tmp = std::string(current_file_.begin(), current_file_.end());
                fs::path path{tmp.replace(extention_pos, 3, ".meta")};
                std::ofstream ofs(path);
                ofs << "START_DATA" << std::endl;
                ofs << static_cast<double>((current_pts_pkt_ - start_pts_pkt_)) * time_base.num / time_base.den;
                ofs << codec_id() << std::endl;
                ofs << "END_DATA";
            }
        }
        Archive(const Archive &) = delete;
        Archive &operator=(const Archive &) = delete;
        Archive(Archive &&) = delete;
        Archive &operator=(Archive &&) = delete;
        void send_pkt(AVPacket *pkt, AVFormatContext *ctx_in);

        const char *codec_id() {
            return avcodec_get_name(video_params_->codec_id);
        }

    private:
        std::chrono::steady_clock::time_point time_point_;
        std::string prefix_path_;
        std::string current_file_;
        int64_t duration_file_;
        int64_t start_pts_pkt_ = AV_NOPTS_VALUE;
        int64_t current_pts_pkt_ = AV_NOPTS_VALUE;
        std::optional<AVRational> time_base_;
        const AVCodecParameters *video_params_;
        std::unique_ptr<AVFormatContext, void (*)(AVFormatContext *ctx)> ctx_ = va_avformat_null_alloc_output_context();
    };
} // namespace va