#include "mloader/scanner.hxx"

#include <algorithm>
#include <cstdio>

using namespace mloader;

DatabaseScanner::DatabaseScanner(Database& database)
    : m_database(&database) {}

Database& DatabaseScanner::database() {
    fassert(m_database != nullptr, "database scanner is not bound to a database");
    return *m_database;
}

const Database& DatabaseScanner::database() const {
    fassert(m_database != nullptr, "database scanner is not bound to a database");
    return *m_database;
}

DatabaseScanner& DatabaseScanner::scan() {
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

void DatabaseScanner::dump() const {
    std::printf("Scanned files:\n");
    for (const auto& cfg : m_configs) {
        const str posix = cfg.as_posix();
        std::printf("\t%s\n", posix.c_str());
    }
}

void DatabaseScanner::clear() {
    m_configs.clear();
}

const vec<Database::PurePath>& DatabaseScanner::configs() const {
    return m_configs;
}

bool DatabaseScanner::is_config(const Database::PurePath& path) {
    const str suffix = path.suffix();
    return suffix == ".yml" || suffix == ".yaml";
}
