#pragma once

#include <optional>
#include <string>
#include <vector>

namespace va {
    std::optional<std::string> make_playlist(const std::string &uri);
    std::string fetch_uri(const std::string &req);
} // namespace va