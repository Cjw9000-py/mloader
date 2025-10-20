#pragma once

#include "mtl/common.hxx"
#include "mtl/fs/path/path.hxx"
#include "mtl/serial.hxx"

namespace mloader {

    struct Definition : mtl::serial::Serial {
        dtor ~Definition() override = default;

        /// Returns the logical identifier for this definition (must be unique per type).
        use virt const str& identifier() cx = 0;

        void set_source(const mtl::fs::Path& path) {
            m_source = path;
        }

        use const mtl::fs::Path& source() cx {
            return m_source;
        }

    protected:
        mtl::fs::Path m_source;
    };

} // namespace mloader
