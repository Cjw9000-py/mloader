#pragma once

#include <utility>

#include "common.hxx"
#include "fs/path.hxx"



struct ConfigLoader {
    ctor ConfigLoader() = default;

    void add_location(const Path& path) {
        m_locations.emplace_back(path);
    }

    void scan() {
        for (auto& root : m_locations) {
            for (auto entry : root.walk()) {
                auto path = entry.path;

                for (auto& files: entry.files) {
                    auto file = path / files;

                    if (!is_config(file)) {
                        continue;
                    }

                    m_files.emplace_back(file);
                }
            }
        }
    }

    static bool is_config(const Path& path) {
        auto sx = path.suffix();
        if (sx == ".yml") return true;
        if (sx == ".yaml") return true;
        return false;
    }

protected:
    vec<Path> m_files;
    vec<Path> m_locations;
};
