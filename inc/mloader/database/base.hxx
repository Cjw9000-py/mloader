#pragma once


#include "mtl/common.hxx"
#include "mtl/fs/path/path.hxx"
#include "mtl/fs/stat.hxx"

#include "registry.hxx"
#include "mloader/resource.hxx"


namespace mloader {

    using mtl::fs::Path;

    struct Database {
        /*
         * - Holds resource entries
         * - resolves requests against contents
         * - snapshot
         * - holds and manages resources
        */

        struct Entry {
            Path path;

            use mtl::fs::meta::Snapshot probe() const;
        };

        ctor Database() = default;
        virt ~Database() = default;


        ///// Loading methods

        /**
         * Checks if the database has finished loading and is in a usable state.
         */
        prop virt bool is_loaded() cx = 0;

        /**
         * Initializes and loads the database into memory.
         */
        virt Database& load() = 0;

        /**
         * Unloads the database and releases associated resources.
         */
        virt Database& unload() = 0;

        Database& activate() {
            return DatabaseRegistry::get().activate(*this);
        }

        Database& deactivate() {
            return DatabaseRegistry::get().deactivate(*this);
        }

        ///// Navigation methods

        virt vec<Entry> list() = 0;
        virt vec<Entry> list(const Path& rel) = 0;
        virt Resource resolve(const Path& rel) = 0;

    };
}
