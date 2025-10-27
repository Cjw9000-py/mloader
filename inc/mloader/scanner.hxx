#pragma once

#include "mtl/common.hxx"

#include "mloader/database/base.hxx"
#include "mloader/resource.hxx"

#include <algorithm>
#include <cstdio>

namespace mloader {

    struct DatabaseScanner {
        struct Config {
            Database* db = nullptr;
            Database::PurePath path;
            ResourceHandle resource;
        };

        ctor DatabaseScanner(Database& database)
            : m_database(&database) {}

        prop Database& database() {
            fassert(m_database != nullptr, "database scanner is not bound to a database");
            return *m_database;
        }

        prop const Database& database() const {
            fassert(m_database != nullptr, "database scanner is not bound to a database");
            return *m_database;
        }

        void scan() {
            clear();

            Database& db = database();
            if (!db.is_loaded()) {
                db.load();
            }

            vec<Database::Entry> entries = db.list();
            for (const auto& entry : entries) {
                Database* entry_db = entry.db ? entry.db : &db;
                if (!entry_db) {
                    continue;
                }

                const bool file_like = entry.db ? entry.is_file()
                                               : entry_db->is_file(entry.path);
                if (!file_like) {
                    continue;
                }

                if (!is_config(entry.path)) {
                    continue;
                }

                Config config;
                config.db = entry_db;
                config.path = entry.path;
                config.resource = entry_db->resolve(entry.path);
                m_configs.emplace_back(std::move(config));
            }

            std::sort(m_configs.begin(), m_configs.end(), [](const Config& lhs, const Config& rhs) {
                return lhs.path.as_posix() < rhs.path.as_posix();
            });
        }

        void dump() const {
            for (const auto& cfg : m_configs) {
                const str posix = cfg.path.as_posix();
                std::printf("%s\n", posix.c_str());
            }
        }

        prop const vec<Config>& configs() const {
            return m_configs;
        }

        void clear() {
            m_configs.clear();
        }

        static bool is_config(const Database::PurePath& path) {
            const str suffix = path.suffix();
            return suffix == ".yml" || suffix == ".yaml";
        }

    private:
        Database* m_database = nullptr;
        vec<Config> m_configs;
    };

} // namespace mloader
