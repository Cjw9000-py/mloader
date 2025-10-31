#pragma once

#include "mtl/common.hxx"

namespace mloader::extension {
    constexpr cstr ARCHIVE_MAGIC = "MLDA";
    constexpr u32 ARCHIVE_MAGIC_SIZE = 4;
    constexpr u32 ARCHIVE_VERSION = 1;
}
