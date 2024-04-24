#include "utils.h"
#include "pico/picohttpparser.h"
#include "playlist.h"
#include "settings.h"
#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <variant>

namespace fs = std::filesystem;

extern va::Settings settings;
extern std::map<std::string, va::PlayList> playlists;
extern std::shared_mutex mutex_playlists;

const char *HEADER_PLAYLIST = "#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-MEDIA-SEQUENCE:0\n#EXT-X-TARGETDURATION:5\n";
const char *FOOTER_PLAYLIST = "#EXT-X-ENDLIST";

namespace {
    enum class Tag { Archive, Live };
    struct ParamsRequestArchive {
        std::string stream_id;
        std::string year;
        std::string month;
        std::string day;
        std::string hour;
    };
    struct ParamsRequestLive {
        std::string stream_id;
    };
    struct Stat {
        double duration;
        bool is_valid = false;
    };
    inline std::pair<Tag, std::variant<ParamsRequestArchive, ParamsRequestLive>> parse_uri(const std::string &uri) {
        auto elems = va::utils::split(uri, '/');
        if (elems.size() == 7) {
            return {Tag::Archive, ParamsRequestArchive{elems[1], elems[2], elems[3], elems[4], elems[5]}};
        }
        if (elems.size() == 3) {
            return {Tag::Live, ParamsRequestLive{elems[1]}};
        }
        throw std::runtime_error("could not parse uri, number of parameters is missing");
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
    namespace utils {
        std::vector<std::string> split(const std::string &s, char delim) {
            std::vector<std::string> elems;
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delim)) {
                elems.push_back(item);
            }
            return elems;
        }
    } // namespace utils
    std::optional<std::string> make_playlist(const std::string &uri) {
        auto [tag, variant] = parse_uri(uri);
        if (tag == Tag::Archive) {
            std::string playlist(HEADER_PLAYLIST);
            auto params = std::get<ParamsRequestArchive>(variant);
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
            for (auto filename : intersection) {
                auto path_stat =
                    std::format("{}/{}/{}/{}/{}/{}/{}.stat", settings.prefix_archive_path(), params.stream_id,
                                params.year, params.month, params.day, params.hour, filename.c_str());
                auto stat = parse_stat(path_stat);
                if (stat.is_valid) {
                    auto path = std::format("{}.ts", filename.c_str());
                    auto item = std::format("#EXTINF:{}\n{}\n", stat.duration, path);
                    playlist.append(item);
                }
            }
            playlist.append(FOOTER_PLAYLIST);
            return playlist;
        } else if (tag == Tag::Live) {
            auto params = std::get<ParamsRequestLive>(variant);
            std::shared_lock lock(mutex_playlists);
            std::string playlist(
                std::format("#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-MEDIA-SEQUENCE:{}\n#EXT-X-TARGETDURATION:5\n",
                            playlists[params.stream_id].index));
            auto it = playlists.find(params.stream_id);
            if (it == playlists.end()) {
                return std::nullopt;
            }
            auto &queue_ref = playlists[params.stream_id].queue;
            for (auto path : queue_ref) {
                auto path_stat = std::format("{}/{}", settings.prefix_archive_path(), path);
                path_stat = path_stat.replace(path_stat.size() - 2, params.stream_id.size(), "stat");
                std::cout << path_stat << std::endl;
                auto stat = parse_stat(path_stat);
                if (stat.is_valid) {
                    auto path_without_prefix = path.replace(0, params.stream_id.size() + 1, "");
                    auto item = std::format("#EXTINF:{}\n{}\n", stat.duration, path);
                    playlist.append(item);
                }
            }
            return playlist;
        } else {
            return std::nullopt;
        }
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