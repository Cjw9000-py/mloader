#pragma once

#include "common.hxx"
#include "base.hxx"


namespace mloader {
    struct FilesystemDatabase : Database {
        ctor FilesystemDatabase() = default;
        ctor FilesystemDatabase(const Path& root);

        ~FilesystemDatabase() override = default;

        bool is_loaded() cx override;
        void load() override;
        void unload() override;

        vec<Path> list() override;
        Entry resolve(const Path &path) override;

    protected:
        Path m_root;
        vec<Entry> m_entries;
    };
}