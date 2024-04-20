#include "app.h"
#include <thread>

namespace va {
    void App::run_in_stream(const Source &source) {
        auto prefix_path{prefix_archive_path_};
        auto duration{duration_file_};
        std::jthread th([=](std::stop_token stoken) {
            Capture<FFMPEGCapture> capture(source);
            capture.run(stoken, prefix_path, duration);
        });
        threads_.push_back(std::move(th));
    }
} // namespace va