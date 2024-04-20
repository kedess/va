#pragma once

#include "source/source.h"
#include <chrono>
#include <cstring>
#include <ctime>
#include <format>

namespace va {
    namespace utils {
        std::string make_endpoint(const Source &source);
        std::string format_datetime_to_path(time_t seconds);
    } // namespace utils
} // namespace va