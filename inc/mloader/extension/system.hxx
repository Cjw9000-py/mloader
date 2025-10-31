#pragma once

#include "extension.hxx"


namespace mloader::extension {
    using mtl::fs::Path;

    struct ExtensionSystem {
        ExtensionSystem(Path data_root) {
            
        }


        void scan() {
            // scan extension locations for extensions
        }



        void add_location(const Path& location);
        void remove_location(const Path& location);
        const vec<Path>& locations();

    protected:
        vec<uptr<Extension>> m_extensions;
        vec<Path> m_locations;
    };

}