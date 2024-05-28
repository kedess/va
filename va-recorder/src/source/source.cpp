#include "source.h"
#include <format>
#include <fstream>

namespace va {
    std::vector<Source> load_sources_from_file(const char *filepath) {
        std::vector<Source> sources;
        auto fis = std::ifstream(filepath);
        if (!fis) {
            auto err_msg = std::format("{}: {}", filepath, std::strerror(errno));
            throw std::runtime_error(err_msg);
        }
        std::string buffer(std::istreambuf_iterator<char>{fis}, {});
        rapidjson::Document doc;
        doc.Parse(buffer.c_str());
        if (doc.IsNull()) {
            auto err_msg = std::format("invalid json format file {}", filepath);
            throw std::runtime_error(err_msg);
        }

        if (doc.IsArray()) {
            for (decltype(doc.Size()) idx = 0, size = doc.Size(); idx < size; ++idx) {
                sources.push_back(Source::from_json(doc[idx].GetObject()));
            }
        }
        return sources;
    }
} // namespace va