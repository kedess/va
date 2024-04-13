#pragma once

#include "avformat.h"
#include <chrono>
#include <string>
extern "C" {
#include <libavformat/avformat.h>
}
namespace va {
    class Archive final {
    public:
        Archive(std::string &prefix_path, int64_t duration, std::vector<const AVCodecParameters *> params_list)
            : prefix_path_(prefix_path), duration_video_file_(duration), params_list_(params_list) {
        }
        ~Archive() {
            if (ctx_) {
                av_write_trailer(ctx_.get());
            }
        }
        Archive(const Archive &) = delete;
        Archive &operator=(const Archive &) = delete;
        Archive(Archive &&) = delete;
        Archive &operator=(Archive &&) = delete;
        void send_pkt(AVPacket *pkt, AVFormatContext *ctx_in);

    private:
        std::chrono::steady_clock::time_point time_point_;
        std::string prefix_path_;
        std::string current_file_;
        int64_t duration_video_file_;
        std::vector<const AVCodecParameters *> params_list_;
        std::unique_ptr<AVFormatContext, void (*)(AVFormatContext *ctx)> ctx_ = va_avformat_null_alloc_output_context();
    };
} // namespace va