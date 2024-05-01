#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace va {
    namespace utils {
        struct Metatada {
            double duration;
            bool is_valid = false;
        };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
        static Metatada parse_metadatat_file(const std::string &path) {
            std::ifstream in(path);
            std::string start_marker, end_marker;
            double duration;
            in >> start_marker;
            if (!in) {
                return {0, false};
            }
            in >> duration;
            if (!in) {
                return {0, false};
            }
            in >> end_marker;
            if (in && start_marker == "START_DATA" && end_marker == "END_DATA") {
                return {duration, true};
            }
            return {0, false};
        }
        static std::vector<std::string> split(const std::string &s, char delim) {
            std::vector<std::string> elems;
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delim)) {
                elems.push_back(item);
            }
            return elems;
        }
    } // namespace utils
#pragma GCC diagnostic pop
} // namespace va