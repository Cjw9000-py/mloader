#pragma once

#include <cstring>

#include "mtl/binary/binary.hxx"

#include "mloader/extension/archive/constants.hxx"
#include "mloader/extension/extension.hxx"

namespace mloader::extension {
    using mtl::binary::DecodeStream;

    struct ArchiveDecoder {
        u32 crc32 = 0;
        byte sha256[32]{};
        u32 flags = 0;

        Extension extension;

        void decode_header(const byte* data, usize size) {
            DecodeStream stream(data, size);
            decode_stream(stream);
        }

        void decode_header(const vec<byte>& buffer) {
            decode_header(buffer.data(), buffer.size());
        }

    private:
        void decode_stream(DecodeStream& stream) {
            char magic[ARCHIVE_MAGIC_SIZE];
            stream.read(magic, ARCHIVE_MAGIC_SIZE);
            fassert(std::memcmp(magic, ARCHIVE_MAGIC, ARCHIVE_MAGIC_SIZE) == 0, "Invalid archive magic");

            auto version = stream.integer<u32>();
            fassert(version == ARCHIVE_VERSION, "Unsupported archive version:", version);

            crc32 = stream.integer<u32>();
            stream.read(sha256, sizeof(sha256));
            flags = stream.integer<u32>();

            extension.version.major = stream.integer<u32>();
            extension.version.minor = stream.integer<u32>();
            extension.version.patch = stream.integer<u32>();
            extension.version.build = stream.integer<u32>();
            extension.name = stream.cstring();

            read_string_list(stream, extension.dependencies);
            read_string_list(stream, extension.incompatible);
            read_string_list(stream, extension.optional);

            extension.definitions = Extension::PurePath(stream.cstring());
            extension.entrypoints.win32 = Extension::PurePath(stream.cstring());
            extension.entrypoints.linux_ = Extension::PurePath(stream.cstring());
        }

        void read_string_list(DecodeStream& stream, vec<str>& out) {
            auto count = stream.integer<u64>();
            out.clear();
            out.reserve(static_cast<usize>(count));
            for (u64 i = 0; i < count; ++i) {
                out.emplace_back(stream.cstring());
            }
        }
    };
}
