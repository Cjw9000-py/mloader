#pragma once

#include "mtl/common.hxx"
#include "mtl/fs/path/path.hxx"
#include "mtl/serial.hxx"

namespace mloader {

    struct Definition : mtl::serial::Serial {
        dtor ~Definition() override = default;

        /// Returns the logical identifier for this definition (must be unique per type).
        use virt const str& identifier() cx = 0;
    };

} // namespace mloader
