#pragma once

#include "mtl/common.hxx"
#include "mtl/fs/path/path.hxx"
#include "mtl/fs/path/pure.hxx"

#include "base.hxx"

namespace mloader {

    struct FilesystemDatabase : Database {
        using Database::Entry;
        using PurePath = Database::PurePath;
        using Path = mtl::fs::Path;

        ctor FilesystemDatabase() = default;
        ctor FilesystemDatabase(const Path& root);
        ~FilesystemDatabase() override = default;

        prop bool is_loaded() const noexcept override;
        FilesystemDatabase& load() override;
        FilesystemDatabase& unload() override;

        vec<Entry> list() override;
        vec<Entry> list(const PurePath& rel) override;
        ResourceHandle resolve(const PurePath& rel) override;

        use bool exists(const PurePath& rel) const override;
        use bool is_file(const PurePath& rel) const override;
        use bool is_dir(const PurePath& rel) const override;

        void set_root(const Path& root);
        prop const Path& root() const;

    protected:
        void ensure_loaded() const;
        use PurePath normalise(const PurePath& rel) const;
        Path make_absolute(const PurePath& rel) const;

        Entry* find_entry(const PurePath& rel);
        const Entry* find_entry(const PurePath& rel) const;
        void collect_entries(const Path& resolved_root);

        Path m_root;
        Path m_resolved_root;
        vec<Entry> m_entries;
        bool m_loaded = false;
    };

} // namespace mloader
