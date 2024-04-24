#pragma once

#include <deque>
#include <string>

namespace va {
    struct PlayList {
        size_t index = 0;
        std::deque<std::string> queue;
    };
} // namespace va