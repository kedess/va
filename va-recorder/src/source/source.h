#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "../../../rapidjson/document.h"
#include "../../../rapidjson/filereadstream.h"
#include "../../../rapidjson/rapidjson.h"
#include "../../../rapidjson/reader.h"
#pragma GCC diagnostic pop
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace va {
    class Source final {
    public:
        static Source from_json(const rapidjson::Value &doc) {
            if (!doc.IsObject())
                throw std::runtime_error("source parsing error, document is not an object");
            if (!doc.HasMember("id") || !doc.HasMember("url")) {
                throw std::runtime_error("source parsing error, not found fields 'id' or 'url'");
            }
            if (!doc["id"].IsString() || !doc["url"].IsString()) {
                throw std::runtime_error("source parsing error, fields 'id' or "
                                         "'url' must be string");
            }
            const char *id = doc["id"].GetString();
            const char *url = doc["url"].GetString();
            if (strlen(id) == 0 || strlen(url) == 0) {
                throw std::runtime_error("source parsing error, empty fields 'id' or 'url'");
            }
            if (!doc.HasMember("username") || !doc.HasMember("password")) {
                return {id, url};
            }
            if (!doc["username"].IsString() || !doc["password"].IsString()) {
                throw std::runtime_error("source parsing error, fields 'username' or "
                                         "'password' must be string");
            }
            const char *username = doc["username"].GetString();
            const char *password = doc["password"].GetString();
            if (strlen(username) == 0 || strlen(password) == 0) {
                throw std::runtime_error("source parsing error, empty fields "
                                         "'username' or 'password'");
            }
            return {id, url, username, password};
        }
        Source(const char *id, const char *url, const char *username = nullptr, const char *password = nullptr)
            : id_(id), url_(url), username_(username ? std::optional<std::string>(username) : std::nullopt),
              password_(password ? std::optional<std::string>(password) : std::nullopt) {
        }
        Source(const Source &) = default;
        Source &operator=(const Source &) = default;
        Source(Source &&) = default;
        Source &operator=(Source &&) = default;
        const std::string &id() const & {
            return id_;
        }
        const std::string &url() const & {
            return url_;
        }
        const std::optional<std::string> &username() const & {
            return username_;
        }
        const std::optional<std::string> &password() const & {
            return password_;
        }

    private:
        std::string id_;
        std::string url_;
        std::optional<std::string> username_ = std::nullopt;
        std::optional<std::string> password_ = std::nullopt;
    };

    std::vector<Source> load_sources_from_file(const char *filepath);
} // namespace va