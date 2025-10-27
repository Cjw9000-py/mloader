#include "mloader/asset.hxx"

#include <algorithm>
#include <cctype>

using namespace mloader;

namespace {

    str detect_image_format(const byte* data, usize size) {
        if (!data || size == 0) {
            return "unknown";
        }

        if (size >= 8 &&
            data[0] == 0x89 &&
            data[1] == 'P' &&
            data[2] == 'N' &&
            data[3] == 'G' &&
            data[4] == 0x0D &&
            data[5] == 0x0A &&
            data[6] == 0x1A &&
            data[7] == 0x0A) {
            return "png";
        }

        if (size >= 3 &&
            data[0] == 0xFF &&
            data[1] == 0xD8 &&
            data[2] == 0xFF) {
            return "jpeg";
        }

        if (size >= 6 &&
            data[0] == 'G' &&
            data[1] == 'I' &&
            data[2] == 'F') {
            return "gif";
        }

        if (size >= 4 &&
            data[0] == 'B' &&
            data[1] == 'M') {
            return "bmp";
        }

        return "unknown";
    }

    str detect_sound_format(const byte* data, usize size) {
        if (!data || size < 4) {
            return "unknown";
        }

        if (size >= 12 &&
            data[0] == 'R' &&
            data[1] == 'I' &&
            data[2] == 'F' &&
            data[3] == 'F' &&
            data[8] == 'W' &&
            data[9] == 'A' &&
            data[10] == 'V' &&
            data[11] == 'E') {
            return "wav";
        }

        if (size >= 4 &&
            data[0] == 'O' &&
            data[1] == 'g' &&
            data[2] == 'g' &&
            data[3] == 'S') {
            return "ogg";
        }

        if (size >= 2 &&
            data[0] == 0xFF &&
            (data[1] & 0xE0) == 0xE0) {
            return "mp3";
        }

        return "unknown";
    }

    str detect_font_format(const byte* data, usize size) {
        if (!data || size < 4) {
            return "unknown";
        }

        if (size >= 4 &&
            data[0] == 'O' &&
            data[1] == 'T' &&
            data[2] == 'T' &&
            data[3] == 'O') {
            return "otf";
        }

        if (size >= 4 &&
            data[0] == 0x00 &&
            data[1] == 0x01 &&
            data[2] == 0x00 &&
            data[3] == 0x00) {
            return "ttf";
        }

        if (size >= 4 &&
            data[0] == 'w' &&
            data[1] == 'O' &&
            data[2] == 'F' &&
            data[3] == 'F') {
            return "woff";
        }

        return "unknown";
    }

    void copy_bytes(const void* source, usize size, vec<byte>& target) {
        target.resize(size);
        if (size == 0) {
            return;
        }
        const auto* raw = static_cast<const byte*>(source);
        std::copy(raw, raw + size, target.begin());
    }

    str read_text(const void* source, usize size) {
        if (!source || size == 0) {
            return {};
        }
        const auto* raw = static_cast<const char*>(source);
        usize begin = 0;
        if (size >= 3 &&
            static_cast<unsigned char>(raw[0]) == 0xEF &&
            static_cast<unsigned char>(raw[1]) == 0xBB &&
            static_cast<unsigned char>(raw[2]) == 0xBF) {
            begin = 3;
        }
        usize end = size;
        while (end > begin && raw[end - 1] == '\0') {
            --end;
        }
        return str(raw + begin, raw + end);
    }

} // namespace

std::any BinaryAsset::parse_resource(Resource& resource) const {
    Data data;
    copy_bytes(resource.data(), static_cast<usize>(resource.size()), data);
    return data;
}

std::any ImageAsset::parse_resource(Resource& resource) const {
    Image image;
    const auto size = static_cast<usize>(resource.size());
    const auto* raw = static_cast<const byte*>(resource.data());
    image.format = detect_image_format(raw, size);
    copy_bytes(raw, size, image.pixels);
    return image;
}

std::any ShaderAsset::parse_resource(Resource& resource) const {
    str shader = read_text(resource.data(), static_cast<usize>(resource.size()));
    // Normalise line endings to LF for predictable shader processing.
    str normalised;
    normalised.reserve(shader.size());
    for (usize i = 0; i < shader.size(); ++i) {
        if (shader[i] == '\r') {
            if (i + 1 < shader.size() && shader[i + 1] == '\n') {
                continue;
            }
            normalised.push_back('\n');
        } else {
            normalised.push_back(shader[i]);
        }
    }
    if (normalised.empty()) {
        normalised = std::move(shader);
    }
    return normalised;
}

std::any SoundAsset::parse_resource(Resource& resource) const {
    Sound sound;
    const auto size = static_cast<usize>(resource.size());
    const auto* raw = static_cast<const byte*>(resource.data());
    sound.format = detect_sound_format(raw, size);
    copy_bytes(raw, size, sound.samples);
    return sound;
}

std::any FontAsset::parse_resource(Resource& resource) const {
    Font font;
    const auto size = static_cast<usize>(resource.size());
    const auto* raw = static_cast<const byte*>(resource.data());
    font.format = detect_font_format(raw, size);
    copy_bytes(raw, size, font.payload);
    return font;
}

std::any TextAsset::parse_resource(Resource& resource) const {
    return read_text(resource.data(), static_cast<usize>(resource.size()));
}

