#pragma once
#include "capture/capture.h"
#include "capture/ffmpeg/backend.h"
#include "source/source.h"
#include "version.h"
#include <boost/log/trivial.hpp>
#include <boost/version.hpp>
#include <thread>

namespace va {
    class App final {
    public:
        App() {
            BOOST_LOG_TRIVIAL(info) << "application " << PROJECT_NAME << " has been successfully launched "
                                    << " ver." << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH;
            BOOST_LOG_TRIVIAL(info) << "boost version " << BOOST_VERSION / 100000 << "." << BOOST_VERSION / 100 % 1000
                                    << "." << BOOST_VERSION % 100;
            BOOST_LOG_TRIVIAL(info) << "ffmpeg version " << av_version_info();
        }
        ~App() {
            for (auto &thread : threads_) {
                thread.request_stop();
            }
            for (auto &thread : threads_) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            BOOST_LOG_TRIVIAL(info) << "application has been successfully stopped";
        }
        App(const App &) = delete;
        App &operator=(const App &) = delete;
        App(App &&) = delete;
        App &operator=(App &&) = delete;
        void run_in_stream(const Source &source) {
            auto prefix_path{prefix_archive_path_};
            auto duration{duration_video_file_};
            std::jthread th([=](std::stop_token stoken) {
                Capture<FFMPEGCapture> capture(source);
                capture.run(stoken, prefix_path, duration);
            });
            threads_.push_back(std::move(th));
        }

        const std::string &prefix_archive_path() const & {
            return prefix_archive_path_;
        }
        void prefix_archvie_path(const std::string &prefix_archive_path) {
            prefix_archive_path_ = prefix_archive_path;
        }
        const std::string &source_file() const & {
            return source_file_;
        }
        void source_file(const std::string &source_file) {
            source_file_ = source_file;
        }
        int64_t duration_video_file() const {
            return duration_video_file_;
        }
        void duration_video_file(int64_t duration_video_file) {
            duration_video_file_ = duration_video_file;
        }

    private:
        std::vector<std::jthread> threads_;
        std::string prefix_archive_path_;
        std::string source_file_;
        int64_t duration_video_file_ = 30;
    };
} // namespace va