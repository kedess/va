#pragma once

#include <string>

namespace va {
    class Settings final {
    public:
        const std::string &prefix_archive_path() const & {
            return prefix_archive_path_;
        }
        void prefix_archvie_path(const std::string &path) {
            prefix_archive_path_ = path;
        }
        const std::string &inference_server_uri() const & {
            return inference_server_address_;
        }
        void inference_server_address(const std::string &address) {
            inference_server_address_ = address;
        }
        int16_t inference_server_port() const {
            return inference_server_port_;
        }
        void inference_server_port(int16_t port) {
            inference_server_port_ = port;
        }

        size_t num_threads() const {
            return num_threads_;
        }
        void num_threads(size_t num_threads) {
            num_threads_ = num_threads;
        }

    private:
        std::string prefix_archive_path_;
        std::string inference_server_address_;
        int16_t inference_server_port_;
        size_t num_threads_;
    };
} // namespace va