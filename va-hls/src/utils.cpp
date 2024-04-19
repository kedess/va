#include "utils.h"
#include "pico/picohttpparser.h"
#include "settings.h"
#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

extern va::Settings settings;

const char *HEADER_PLAYLIST =
    "#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-MEDIA-SEQUENCE:0\n#EXT-X-ALLOW-CACHE:YES\n#EXT-X-TARGETDURATION:10\n";
const char *FOOTER_PLAYLIST = "#EXT-X-ENDLIST";

namespace {
    struct ParamsRequest {
        std::string stream_id;
        std::string year;
        std::string month;
        std::string day;
        std::string hour;
    };
    struct Stat {
        double duration;
        bool is_valid = false;
    };
    std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }
    inline ParamsRequest parse_uri(const std::string &uri) {
        auto elems = split(uri, '/');
        if (elems.size() != 7) {
            throw std::runtime_error("could not parse uri, number of parameters is missing");
        }
        return {elems[1], elems[2], elems[3], elems[4], elems[5]};
    }
    Stat parse_stat(const std::string &path) {
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

} // namespace

namespace va {
    std::optional<std::string> make_playlist(const std::string &uri) {
        auto params = parse_uri(uri);
        auto path = std::format("{}/{}/{}/{}/{}/{}", settings.prefix_archive_path(), params.stream_id, params.year,
                                params.month, params.day, params.hour);
        std::set<fs::path> files, files_stat;
        for (const auto &entry : fs::directory_iterator(path)) {
            auto file = entry.path();
            if (file.extension() == ".ts") {
                files.emplace(file.stem());
            }
            if (file.extension() == ".stat") {
                files_stat.emplace(file.stem());
            }
        }
        std::vector<fs::path> intersection;
        /*
         * В playlist включаются только файлы для которых известна их продолжительность,
         * то есть присутствие файла filename.stat
         */
        std::set_intersection(files.begin(), files.end(), files_stat.begin(), files_stat.end(),
                              std::back_inserter(intersection));
        if (intersection.empty()) {
            return std::nullopt;
        }
        std::string playlist(HEADER_PLAYLIST);
        for (auto filename : intersection) {
            auto path_stat = std::format("{}/{}/{}/{}/{}/{}/{}.stat", settings.prefix_archive_path(), params.stream_id,
                                         params.year, params.month, params.day, params.hour, filename.c_str());
            auto stat = parse_stat(path_stat);
            if (stat.is_valid) {
                auto path = std::format("{}.ts", filename.c_str());
                auto item = std::format("#EXTINF:{}\n{}\n", stat.duration, path);
                playlist.append(item);
            }
        }
        playlist.append(FOOTER_PLAYLIST);
        return {playlist};
    }
    std::string fetch_uri(const std::string &req) {
        std::string uri;
        char *method, *path;
        int minor_version;
        struct phr_header headers[100];
        size_t method_len, path_len, num_headers;
        num_headers = sizeof(headers) / sizeof(headers[0]);
        int res =
            phr_parse_request(req.c_str(), req.size(), const_cast<const char **>(&method), &method_len,
                              const_cast<const char **>(&path), &path_len, &minor_version, headers, &num_headers, 0);
        if (-1 == res) {
            throw std::runtime_error("Could not parse request");
        }
        return {path, path + path_len};
    }
} // namespace va