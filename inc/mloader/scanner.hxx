#pragma once

#include "mtl/common.hxx"

#include "mloader/database/base.hxx"

#include <algorithm>
#include <cstdio>

namespace mloader {

    struct DatabaseScanner {
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

        DatabaseScanner& scan() {
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

                m_configs.emplace_back(entry.path);
            }

            std::sort(m_configs.begin(), m_configs.end(), [](const Database::PurePath& lhs, const Database::PurePath& rhs) {
                return lhs.as_posix() < rhs.as_posix();
            });

            return *this;
        }

        void dump() const {
            printf("Scanned files:\n");
            for (const auto& cfg : m_configs) {
                const str posix = cfg.as_posix();
                printf("\t%s\n", posix.c_str());
            }
        }

        void clear() {
            m_configs.clear();
        }

        prop const vec<Database::PurePath>& configs() const {
            return m_configs;
        }

        static bool is_config(const Database::PurePath& path) {
            const str suffix = path.suffix();
            return suffix == ".yml" || suffix == ".yaml";
        }

    private:
        Database* m_database = nullptr;
        vec<Database::PurePath> m_configs;
    };

} // namespace mloader
