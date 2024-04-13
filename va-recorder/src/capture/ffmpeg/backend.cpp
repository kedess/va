#include "backend.h"
#include "../../utils.h"
#include "archive.h"
#include "avformat.h"
#include "avpacket.h"
#include "codec.h"

static void sleep_and_test(std::stop_token stoken, int duration) {
    while (duration > 0) {
        if (stoken.stop_requested()) {
            break;
        }
        --duration;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

namespace va {
    void FFMPEGCapture::run(std::stop_token stoken, const std::string &prefix_path, int64_t duration) {
        std::string endpoint;
        try {
            endpoint = utils::make_endpoint(source_);
        } catch (std::exception &ex) {
            BOOST_LOG_TRIVIAL(error) << "thread will be stop, " << ex.what();
            return;
        }
        auto prefix_archive_path = std::format("{}/{}", prefix_path, source_.id());
        while (!stoken.stop_requested()) {
            update_time_point();
            auto info_msg = std::format("trying to connect to video source ({})", source_.id());
            BOOST_LOG_TRIVIAL(info) << info_msg;
            auto ctx = va_avformat_alloc_context();
            if (!ctx) {
                auto err_msg = std::format("failed to create AVFormatContext [{}]", source_.id());
                BOOST_LOG_TRIVIAL(error) << err_msg;
                sleep_and_test(stoken, 5);
                continue;
            }
            /*
             * Функция следит за временем задержки получения пакетов из
             * потока, используя переменную time_point_
             */
            AVIOInterruptCB icb = {check_latency_read_packet, static_cast<void *>(this)};
            ctx->interrupt_callback = icb;
            ctx->interrupt_callback.opaque = this;

            auto ptr = ctx.get();
            if (avformat_open_input(&ptr, endpoint.c_str(), nullptr, nullptr) < 0) {
                /*
                 * Освобождаем uniqe_ptr fmt_ctx, так как в случаи сбоя
                 * функции avformat_open_input, она сама вызывает
                 * зачистку данных по этому указателю fmt_ctx
                 */
                ctx.release();
                auto err_msg = std::format("unable to connect to the video source ({})", source_.id());
                BOOST_LOG_TRIVIAL(error) << err_msg;
                sleep_and_test(stoken, 10);
                continue;
            }
            if (avformat_find_stream_info(ctx.get(), nullptr) < 0) {
                auto err_msg = std::format("not found info for stream ({})", source_.id());
                BOOST_LOG_TRIVIAL(error) << err_msg;
                sleep_and_test(stoken, 5);
                continue;
            }
            auto params_list = stream_params(ctx.get(), source_.id());
            if (params_list.empty()) {
                auto err_msg = std::format("not found audio and video channels for stream ({})", source_.id());
                BOOST_LOG_TRIVIAL(error) << err_msg;
                sleep_and_test(stoken, 5);
                continue;
            }
            try {
                Archive archive(prefix_archive_path, duration, params_list);
                const auto pkt = va::va_avpacket_alloc();
                while (!stoken.stop_requested() && av_read_frame(ctx.get(), pkt.get()) == 0) {
                    archive.send_pkt(pkt.get(), ctx.get());
                    update_time_point();
                    av_packet_unref(pkt.get());
                }
            } catch (std::exception &ex) {
                auto err_msg = std::format("failed to create output AVFormatContext ({}), ", source_.id());
                BOOST_LOG_TRIVIAL(error) << err_msg << ex.what();
                sleep_and_test(stoken, 5);
            }
        }
    }
} // namespace va