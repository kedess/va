#pragma once

#include <string>

namespace va {
    class Settings final {
    public:
        const std::string &prefix_archive_path() const & {
            return prefix_archive_path_;
        }
        void prefix_archvie_path(const std::string &prefix_archive_path) {
            prefix_archive_path_ = prefix_archive_path;
        }

        void port(uint16_t port) {
            port_ = port;
        }
        uint16_t port() {
            return port_;
        }

    private:
        std::string prefix_archive_path_;
        uint16_t port_;
    };
} // namespace va