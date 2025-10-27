#pragma once

#include "mtl/common.hxx"

#include "mloader/database/base.hxx"
#include "mloader/resource.hxx"

#include <algorithm>
#include <cstdio>

namespace mloader {

    struct DatabaseLoader {
        struct ConfigResource {
            Database* db = nullptr;
            Database::PurePath path;
            ResourceHandle resource;
        };

        ctor DatabaseLoader() = default;

        void add_database(Database& database) {
            m_databases.emplace_back(&database);
        }

        void clear_databases() {
            m_databases.clear();
        }

        void scan() {
            clear();
            for (Database* db : m_databases) {
                if (!db) {
                    continue;
                }
                if (!db->is_loaded()) {
                    db->load();
                }

                vec<Database::Entry> entries = db->list();
                for (const auto& entry : entries) {
                    Database* entry_db = entry.db ? entry.db : db;
                    if (!entry_db) {
                        continue;
                    }

                    bool file_like = entry.db ? entry.is_file()
                                              : entry_db->is_file(entry.path);
                    if (!file_like) {
                        continue;
                    }

                    if (!is_config(entry.path)) {
                        continue;
                    }

                    ConfigResource config;
                    config.db = entry_db;
                    config.path = entry.path;
                    config.resource = entry_db->resolve(entry.path);
                    m_configs.emplace_back(std::move(config));
                }
            }

            std::sort(m_configs.begin(), m_configs.end(),
                      [](const ConfigResource& lhs, const ConfigResource& rhs) {
                          return lhs.path.as_posix() < rhs.path.as_posix();
                      });
        }

        void dump() const {
            for (const auto& config : m_configs) {
                str display = config.path.as_posix();
                std::printf("%s\n", display.c_str());
            }
        }

        use const vec<ConfigResource>& configs() const {
            return m_configs;
        }

        void clear() {
            m_configs.clear();
        }

        static bool is_config(const Database::PurePath& path) {
            str suffix = path.suffix();
            if (suffix == ".yml" || suffix == ".yaml") {
                return true;
            }
            return false;
        }

    private:
        vec<ConfigResource> m_configs;
        vec<Database*> m_databases;
    };

} // namespace mloader
