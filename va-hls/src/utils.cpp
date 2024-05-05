#include "utils.h"
#include "../../common/utils.h"
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

} // namespace

namespace va {
    // namespace utils
    std::optional<std::string> make_playlist(const std::string &uri) {
        auto [tag, variant] = parse_uri(uri);
        if (tag == Tag::Archive) {
            std::string playlist(HEADER_PLAYLIST);
            auto params = std::get<ParamsRequestArchive>(variant);
            auto path = std::format("{}/{}/{}/{}/{}/{}", va::Settings::instance().prefix_archive_path(),
                                    params.stream_id, params.year, params.month, params.day, params.hour);

            std::set<fs::path> files, files_meta;
            for (const auto &entry : fs::directory_iterator(path)) {
                auto file = entry.path();
                if (file.extension() == ".ts") {
                    files.emplace(file.stem());
                }
                if (file.extension() == ".meta") {
                    files_meta.emplace(file.stem());
                }
            }

            std::vector<fs::path> intersection;
            /*
             * В playlist включаются только файлы для которых известна их продолжительность,
             * то есть присутствие файла filename.meta
             */
            std::set_intersection(files.begin(), files.end(), files_meta.begin(), files_meta.end(),
                                  std::back_inserter(intersection));
            if (intersection.empty()) {
                return std::nullopt;
            }
            for (auto filename : intersection) {
                auto path_meta =
                    std::format("{}/{}/{}/{}/{}/{}/{}.meta", va::Settings::instance().prefix_archive_path(),
                                params.stream_id, params.year, params.month, params.day, params.hour, filename.c_str());
                auto meta = va::utils::parse_metadatat_file(path_meta);
                if (meta.is_valid) {
                    auto path = std::format("{}.ts", filename.c_str());
                    auto item = std::format("#EXTINF:{}\n{}\n", meta.duration, path);
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
                auto path_meta = std::format("{}/{}", va::Settings::instance().prefix_archive_path(), path);
                path_meta = path_meta.replace(path_meta.size() - 2, params.stream_id.size(), "meta");
                auto meta = va::utils::parse_metadatat_file(path_meta);
                if (meta.is_valid) {
                    auto tmp = std::string(path.begin(), path.end());
                    auto path_without_prefix = tmp.replace(0, params.stream_id.size() + 1, "");
                    auto item = std::format("#EXTINF:{}\n{}\n", meta.duration, path_without_prefix);
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