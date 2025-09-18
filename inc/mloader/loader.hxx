#pragma once


#include "common.hxx"
#include "fs/path.hxx"


namespace mloader {


    struct ConfigLoader {
        ctor ConfigLoader() = default;

        void add_location(const Path& path) {
            m_locations.emplace_back(path);
        }

        void scan() {
            for (auto& root : m_locations) {
                // go over every location

                for (auto entry : root.walk()) {
                    // go over the fs tree

                    auto path = entry.path;

                    for (auto& files: entry.files) {
                        // go over files in directory

                        auto file = path / files;

                        if (!is_config(file)) {
                            continue;
                        }

                        m_files.emplace_back(file); // add yml config
                    }
                }
            }
        }

        void dump() const {
            str s;
            for (auto& path : m_files) {
                s = path.string();
                printf("%s", s.c_str());
            }
        }

        static bool is_config(const Path& path) {
            // check if it's a yml config

            auto sx = path.suffix();
            if (sx == ".yml") return true;
            if (sx == ".yaml") return true;
            return false;
        }

    protected:
        vec<Path> m_files;
        vec<Path> m_locations;
    };

}
