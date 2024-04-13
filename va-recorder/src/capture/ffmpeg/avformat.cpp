#include "avformat.h"

namespace va {
    void release_context(AVFormatContext *ctx) {
        avformat_close_input(&ctx);
        avformat_free_context(ctx);
    }
    void release_output_context(AVFormatContext *ctx) {
        if (ctx) {
            if (ctx->pb) {
                avio_close(ctx->pb);
            }
            avformat_free_context(ctx);
        }
    }
    std::unique_ptr<AVFormatContext, void (*)(AVFormatContext *ctx)>
    va_avformat_alloc_output_context(const char *filename, std::vector<const AVCodecParameters *> &params_list) {
        AVFormatContext *ctx = nullptr;
        if (avformat_alloc_output_context2(&ctx, nullptr, nullptr, filename) < 0) {
            throw std::runtime_error("failed to alocate output AVFormatContext");
        }
        for (auto codec_params : params_list) {
            auto out_stream = avformat_new_stream(ctx, nullptr);
            if (!out_stream) {
                throw std::runtime_error("failed allocating output stream");
            }
            if (avcodec_parameters_copy((*out_stream).codecpar, codec_params) < 0) {
                throw std::runtime_error("failed to copy codec parameters");
            }
        }
        if (avio_open(&ctx->pb, filename, AVIO_FLAG_WRITE) < 0) {
            auto err_msg = std::format("could not open output file {}", filename);
            throw std::runtime_error(err_msg);
        }
        if (avformat_write_header(ctx, nullptr) < 0) {
            auto err_msg = std::format("error occurred when opening output file {}", filename);
            throw std::runtime_error(err_msg);
        }
        return std::unique_ptr<AVFormatContext, void (*)(AVFormatContext * ctx)>(ctx, release_output_context);
    }
} // namespace va