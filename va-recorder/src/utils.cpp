#include "utils.h"

namespace {
    constexpr size_t buffer_len = std::size("yyyy/mm/dd/hh");
}

namespace va {
    namespace utils {
        std::string make_endpoint(const Source &source) {
            if (!source.password().has_value() && !source.username().has_value()) {
                return source.url();
            }
            if (!source.password().has_value() || !source.username().has_value()) {
                throw std::runtime_error("'username' or 'password' are empty. Both "
                                         "must be specified if one is specified");
                return source.url();
            }
            auto &username = source.username().value();
            auto &password = source.password().value();
            auto &url = source.url();
            auto pos = url.find("://");
            if (pos == std::string::npos) {
                auto err_msg = std::format("could not make endpoint for video "
                                           "source({}), incorrect url address",
                                           source.id());
                throw std::runtime_error(err_msg);
            }
            return std::format("{}{}:{}@{}", url.substr(0, pos + 3), username, password, url.substr(pos + 3));
        }
        std::string format_datetime_to_path(time_t seconds) {
            char buffer[buffer_len];
            std::strftime(buffer, buffer_len, "%Y/%m/%d/%H", std::gmtime(&seconds));
            return std::string(buffer);
        }
    } // namespace utils
} // namespace va