#pragma once


#include "common.hxx"
#include "error.hxx"


namespace mloader {
    struct Database;

    struct DatabaseRegistry {
        ctor DatabaseRegistry() = default;
        dtor ~DatabaseRegistry() = default;

        Database& activate(Database& db) noexcept {
            m_database = &db;
            return db;
        }

        Database& deactivate(Database& db) {
            if (m_database != &db) {
                throw RuntimeError("Tried to deactivate database that's not active.");
            }
            m_database = nullptr;
            return db;
        }

        void deactivate() noexcept {
            m_database = nullptr;
        }

        Database* database() noexcept {
            return m_database;
        }

        const Database* database() cx {
            return m_database;

        }

        static DatabaseRegistry& get() {
            return m_instance;
        }

    protected:
        Database* m_database = nullptr;
        static DatabaseRegistry m_instance;
    };
}
