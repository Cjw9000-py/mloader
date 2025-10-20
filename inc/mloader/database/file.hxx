#pragma once

#include "mtl/common.hxx"
#include "mtl/fs/stat.hxx"

#include "base.hxx"

namespace mloader {
    using namespace mtl::fs;

    struct FilesystemDatabase : Database {
        struct Entry {
            Path absolute;
            Path relative;
            meta::Snapshot metadata{};

            use bool matches(const Path& candidate) const;
        };

        ctor FilesystemDatabase() = default;
        ctor FilesystemDatabase(const Path& root);

        ~FilesystemDatabase() override = default;

        prop bool is_loaded() cx override;
        FilesystemDatabase& load() override;
        FilesystemDatabase& unload() override;

        vec<Path> list() override;
        Resource resolve(const Path& path) override;

        void set_root(const Path& root);
        prop const Path& root() cx;

    protected:
        use Path make_absolute(const Path& path) const;
        Entry* find_entry(const Path& absolute);
        const Entry* find_entry(const Path& absolute) const;
        void collect_entries(const Path& resolved_root);

        Path m_root;
        Path m_resolved_root;
        vec<Entry> m_entries;
        bool m_loaded = false;
    };
}
