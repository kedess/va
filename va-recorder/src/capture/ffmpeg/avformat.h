#pragma once
#include <format>
#include <memory>
#include <vector>
extern "C" {
#include <libavformat/avformat.h>
}

namespace va {
    void release_context(AVFormatContext *ctx);
    void release_output_context(AVFormatContext *ctx);

    inline std::unique_ptr<AVFormatContext, void (*)(AVFormatContext *ctx)> va_avformat_alloc_context() {
        auto ctx = avformat_alloc_context();
        if (ctx == nullptr) {
            return std::unique_ptr<AVFormatContext, void (*)(AVFormatContext * ctx)>(nullptr, release_context);
        }
        return std::unique_ptr<AVFormatContext, void (*)(AVFormatContext * ctx)>(ctx, release_context);
    }

    std::unique_ptr<AVFormatContext, void (*)(AVFormatContext *ctx)>
    va_avformat_alloc_output_context(const char *filename, std::vector<const AVCodecParameters *> &params_list);

    inline std::unique_ptr<AVFormatContext, void (*)(AVFormatContext *ctx)> va_avformat_null_alloc_output_context() {
        return std::unique_ptr<AVFormatContext, void (*)(AVFormatContext * ctx)>(nullptr, release_output_context);
    }
} // namespace va