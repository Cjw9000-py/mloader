#pragma once

#include "mtl/common.hxx"
#include "mtl/fs/path.hxx"

#include "registry.hxx"
#include "mloader/resource.hxx"

namespace mloader {

    /**
     * Virtual archive interface abstracting resources stored within a
     * backend-neutral container. Paths are logical, relative PurePath values
     * that describe entries inside the archive regardless of the underlying
     * storage (filesystem directories, packed binaries, etc.).
     */
    struct Database {
        using PurePath = mtl::fs::PureUnixPath;

        ctor Database() = default;
        virt ~Database() = default;

        /// Indicates whether the database is ready for queries.
        prop virt bool is_loaded() const noexcept = 0;

        /// Performs any loading required to service requests.
        virt Database& load() = 0;

        /// Releases resources held by the implementation.
        virt Database& unload() = 0;

        Database& activate() {
            return DatabaseRegistry::get().activate(*this);
        }

        Database& deactivate() {
            return DatabaseRegistry::get().deactivate(*this);
        }

        /**
         * Logical entry describing a path inside the database. Entries carry a
         * back-reference to their owning Database so convenience queries can be
         * issued without requiring clients to hold additional state.
         */
        struct Entry {
            PurePath path;
            Database* db = nullptr;

            use bool exists() const {
                return db && db->exists(path);
            }

            use bool is_file() const {
                return db && db->is_file(path);
            }

            use bool is_dir() const {
                return db && db->is_dir(path);
            }
        };

        /// Lists entries at the root of the database.
        virt vec<Entry> list() = 0;
        /// Lists entries under the given relative path.
        virt vec<Entry> list(const PurePath& rel) = 0;
        /// Resolves the given entry to a managed resource handle.
        virt ResourceHandle resolve(const PurePath& rel) = 0;
        vec<ResourceHandle> resolve(const vec<PurePath>& rels) {
            vec<ResourceHandle> handles;
            handles.reserve(rels.size());
            for (const auto& rel : rels) {
                handles.emplace_back(resolve(rel));
            }
            return handles;
        }

        /// Checks whether a logical path exists within the archive.
        virt bool exists(const PurePath& rel) const = 0;
        /// Checks whether the path refers to a file-like payload.
        virt bool is_file(const PurePath& rel) const = 0;
        /// Checks whether the path refers to a directory-like entry.
        virt bool is_dir(const PurePath& rel) const = 0;
    };

} // namespace mloader
