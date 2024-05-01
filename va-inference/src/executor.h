#pragma once
#include "settings.h"
#include <thread>
#include <vector>

namespace va {
    class Executor {
    public:
        Executor(const Settings &settings, std::stop_token stoken);
        void inference(std::stop_token stoken);
        void send(std::vector<char> &buffer, std::string &&path);
        ~Executor() {
            for (auto &thread : threads_) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }

    private:
        std::vector<std::thread> threads_;
        const std::string inference_uri_;
        const int16_t inference_port_;
    };
} // namespace va