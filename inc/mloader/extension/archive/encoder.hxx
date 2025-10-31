#pragma once

#include "mtl/version.hxx"
#include "mtl/binary/binary.hxx"

#include "mloader/extension/archive/constants.hxx"
#include "mloader/extension/extension.hxx"

namespace mloader::extension {
    using mtl::binary::EncodeStream;

    struct ArchiveEncoder {
        u32 crc32;
        byte sha256[32];
        u32 flags;

        Extension extension;

        vec<byte> encode_header() const {
            EncodeStream stream;

            // primary fields
            stream.write(ARCHIVE_MAGIC, ARCHIVE_MAGIC_SIZE);
            stream.integer(ARCHIVE_VERSION);
            stream.integer(crc32);
            stream.write(sha256);
            stream.integer(flags);

            // extension details
            stream.integer<u32>(extension.version.major);
            stream.integer<u32>(extension.version.minor);
            stream.integer<u32>(extension.version.patch);
            stream.integer<u32>(extension.version.build);
            stream.cstring(extension.name);

            stream.integer<u64>(extension.dependencies.size());
            for (const auto& entry : extension.dependencies) {
                stream.cstring(entry);
            }

            stream.integer<u64>(extension.incompatible.size());
            for (const auto& entry : extension.incompatible) {
                stream.cstring(entry);
            }

            stream.integer<u64>(extension.optional.size());
            for (const auto& entry : extension.optional) {
                stream.cstring(entry);
            }

            stream.cstring(extension.definitions.string());

            stream.cstring(extension.entrypoints.win32.string());
            stream.cstring(extension.entrypoints.linux_.string());

            return stream.finish();
        }

    };
}
