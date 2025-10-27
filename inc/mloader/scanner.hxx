#pragma once

#include "mtl/common.hxx"

#include "mloader/database/base.hxx"

namespace mloader {

    struct DatabaseScanner {
        explicit DatabaseScanner(Database& database);

        prop Database& database();
        prop const Database& database() const;

        DatabaseScanner& scan();

        void dump() const;
        void clear();

        prop const vec<Database::PurePath>& configs() const;

        static bool is_config(const Database::PurePath& path);

    private:
        Database* m_database = nullptr;
        vec<Database::PurePath> m_configs;
    };

} // namespace mloader
