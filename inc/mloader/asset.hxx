#pragma once

#include "mtl/common.hxx"
#include "mtl/error.hxx"
#include "mtl/fs/path/pure.hxx"

#include "mloader/database/base.hxx"
#include "mloader/database/registry.hxx"
#include "mloader/resource.hxx"

#include <any>
#include <functional>
#include <mutex>
#include <optional>

namespace mloader {

    enum class AssetType : u8 {
        invalid = 0,
        binary,
        image,
        shader,
        sound,
        font,
        text,
    };

    enum class AssetState : u8 {
        unloaded = 0,
        unparsed = 1,
        parsed = 2,
    };

    class Asset {
    public:
        Asset();
        explicit Asset(AssetType type);
        Asset(Database& database, const Database::PurePath& path, AssetType type);
        Asset(const str& path, AssetType type);
        Asset(const char* path, AssetType type);
        virtual ~Asset() = default;

        Asset(const Asset&) = default;
        Asset& operator=(const Asset&) = default;
        Asset(Asset&&) noexcept = default;
        Asset& operator=(Asset&&) noexcept = default;

        prop AssetState state() const noexcept { return m_state; }

        prop const Database::PurePath& path() const noexcept { return m_path; }

        void bind(Database& database);
        void bind(Database& database, const Database::PurePath& path);
        void set_path(const Database::PurePath& path);

        void unload() const;
        void touch() const;

        ResourceHandle handle() const;

    protected:
        Database& ensure_database() const;
        Resource& ensure_resource() const;

        template<typename Payload>
        const Payload& payload() const;

        virtual std::any parse_resource(Resource& resource) const = 0;

        prop AssetType type() const noexcept { return m_type; }

    private:
        int cache_key() const noexcept;
        std::optional<std::reference_wrapper<const std::any>> try_cached(Resource& resource) const;
        const std::any& cache(Resource& resource, std::any value) const;

        mutable Database* m_database = nullptr;
        Database::PurePath m_path;
        mutable ResourceHandle m_handle;
        AssetType m_type = AssetType::invalid;
        mutable AssetState m_state = AssetState::unloaded;
    };

    class BinaryAsset : public Asset {
    public:
        using Data = vec<byte>;

        BinaryAsset();
        explicit BinaryAsset(const char* path);
        explicit BinaryAsset(const str& path);
        BinaryAsset(Database& database, const Database::PurePath& path);

        const Data& data() const;

    protected:
        std::any parse_resource(Resource& resource) const override;
    };

    class ImageAsset : public Asset {
    public:
        struct Image {
            str format;
            vec<byte> pixels;
        };

        ImageAsset();
        explicit ImageAsset(const char* path);
        explicit ImageAsset(const str& path);
        ImageAsset(Database& database, const Database::PurePath& path);

        const Image& image() const;

    protected:
        std::any parse_resource(Resource& resource) const override;
    };

    class ShaderAsset : public Asset {
    public:
        ShaderAsset();
        explicit ShaderAsset(const char* path);
        explicit ShaderAsset(const str& path);
        ShaderAsset(Database& database, const Database::PurePath& path);

        const str& source() const;

    protected:
        std::any parse_resource(Resource& resource) const override;
    };

    class SoundAsset : public Asset {
    public:
        struct Sound {
            str format;
            vec<byte> samples;
        };

        SoundAsset();
        explicit SoundAsset(const char* path);
        explicit SoundAsset(const str& path);
        SoundAsset(Database& database, const Database::PurePath& path);

        const Sound& sound() const;

    protected:
        std::any parse_resource(Resource& resource) const override;
    };

    class FontAsset : public Asset {
    public:
        struct Font {
            str format;
            vec<byte> payload;
        };

        FontAsset();
        explicit FontAsset(const char* path);
        explicit FontAsset(const str& path);
        FontAsset(Database& database, const Database::PurePath& path);

        const Font& font() const;

    protected:
        std::any parse_resource(Resource& resource) const override;
    };

    class TextAsset : public Asset {
    public:
        TextAsset();
        explicit TextAsset(const char* path);
        explicit TextAsset(const str& path);
        TextAsset(Database& database, const Database::PurePath& path);

        const str& text() const;

    protected:
        std::any parse_resource(Resource& resource) const override;
    };

    inline Asset::Asset()
        : Asset(AssetType::invalid) {}

    inline Asset::Asset(AssetType type)
        : m_type(type) {}

    inline Asset::Asset(Database& database, const Database::PurePath& path, AssetType type)
        : Asset(type) {
        bind(database, path);
    }

    inline Asset::Asset(const str& path, AssetType type)
        : Asset(type) {
        bind(ensure_database(), Database::PurePath(path));
    }

    inline Asset::Asset(const char* path, AssetType type)
        : Asset(str(path ? path : ""), type) {}

    inline void Asset::bind(Database& database) {
        m_database = &database;
        unload();
    }

    inline void Asset::bind(Database& database, const Database::PurePath& path) {
        bind(database);
        m_path = path;
    }

    inline void Asset::set_path(const Database::PurePath& path) {
        m_path = path;
        unload();
    }

    inline void Asset::unload() const {
        m_handle = ResourceHandle();
        m_state = AssetState::unloaded;
    }

    inline void Asset::touch() const {
        ensure_resource();
    }

    inline ResourceHandle Asset::handle() const {
        ensure_resource();
        return m_handle;
    }

    inline Database& Asset::ensure_database() const {
        if (m_database) {
            return *m_database;
        }
        Database* active = DatabaseRegistry::get().database();
        if (!active) {
            throw RuntimeError("No database bound to asset and no active database.");
        }
        m_database = active;
        return *m_database;
    }

    inline Resource& Asset::ensure_resource() const {
        if (!m_handle.valid()) {
            Database& db = ensure_database();
            if (!db.is_loaded()) {
                db.load();
            }
            m_handle = db.resolve(m_path);
            m_state = AssetState::unparsed;
        }
        return *m_handle;
    }

    inline int Asset::cache_key() const noexcept {
        return static_cast<int>(m_type);
    }

    inline std::optional<std::reference_wrapper<const std::any>> Asset::try_cached(Resource& resource) const {
        std::lock_guard lock(resource.m_asset_mutex);
        auto it = resource.m_asset_cache.find(cache_key());
        if (it == resource.m_asset_cache.end()) {
            return std::nullopt;
        }
        return std::cref(it->second);
    }

    inline const std::any& Asset::cache(Resource& resource, std::any value) const {
        std::lock_guard lock(resource.m_asset_mutex);
        auto& slot = resource.m_asset_cache[cache_key()];
        slot = std::move(value);
        return slot;
    }

    template<typename Payload>
    inline const Payload& Asset::payload() const {
        Resource& resource = ensure_resource();
        if (auto cached = try_cached(resource)) {
            m_state = AssetState::parsed;
            return std::any_cast<const Payload&>(cached->get());
        }
        std::any parsed = parse_resource(resource);
        const std::any& slot = cache(resource, std::move(parsed));
        m_state = AssetState::parsed;
        return std::any_cast<const Payload&>(slot);
    }

    inline BinaryAsset::BinaryAsset()
        : Asset(AssetType::binary) {}

    inline BinaryAsset::BinaryAsset(const char* path)
        : Asset(path, AssetType::binary) {}

    inline BinaryAsset::BinaryAsset(const str& path)
        : Asset(path, AssetType::binary) {}

    inline BinaryAsset::BinaryAsset(Database& database, const Database::PurePath& path)
        : Asset(database, path, AssetType::binary) {}

    inline const BinaryAsset::Data& BinaryAsset::data() const {
        return payload<Data>();
    }

    inline ImageAsset::ImageAsset()
        : Asset(AssetType::image) {}

    inline ImageAsset::ImageAsset(const char* path)
        : Asset(path, AssetType::image) {}

    inline ImageAsset::ImageAsset(const str& path)
        : Asset(path, AssetType::image) {}

    inline ImageAsset::ImageAsset(Database& database, const Database::PurePath& path)
        : Asset(database, path, AssetType::image) {}

    inline const ImageAsset::Image& ImageAsset::image() const {
        return payload<Image>();
    }

    inline ShaderAsset::ShaderAsset()
        : Asset(AssetType::shader) {}

    inline ShaderAsset::ShaderAsset(const char* path)
        : Asset(path, AssetType::shader) {}

    inline ShaderAsset::ShaderAsset(const str& path)
        : Asset(path, AssetType::shader) {}

    inline ShaderAsset::ShaderAsset(Database& database, const Database::PurePath& path)
        : Asset(database, path, AssetType::shader) {}

    inline const str& ShaderAsset::source() const {
        return payload<str>();
    }

    inline SoundAsset::SoundAsset()
        : Asset(AssetType::sound) {}

    inline SoundAsset::SoundAsset(const char* path)
        : Asset(path, AssetType::sound) {}

    inline SoundAsset::SoundAsset(const str& path)
        : Asset(path, AssetType::sound) {}

    inline SoundAsset::SoundAsset(Database& database, const Database::PurePath& path)
        : Asset(database, path, AssetType::sound) {}

    inline const SoundAsset::Sound& SoundAsset::sound() const {
        return payload<Sound>();
    }

    inline FontAsset::FontAsset()
        : Asset(AssetType::font) {}

    inline FontAsset::FontAsset(const char* path)
        : Asset(path, AssetType::font) {}

    inline FontAsset::FontAsset(const str& path)
        : Asset(path, AssetType::font) {}

    inline FontAsset::FontAsset(Database& database, const Database::PurePath& path)
        : Asset(database, path, AssetType::font) {}

    inline const FontAsset::Font& FontAsset::font() const {
        return payload<Font>();
    }

    inline TextAsset::TextAsset()
        : Asset(AssetType::text) {}

    inline TextAsset::TextAsset(const char* path)
        : Asset(path, AssetType::text) {}

    inline TextAsset::TextAsset(const str& path)
        : Asset(path, AssetType::text) {}

    inline TextAsset::TextAsset(Database& database, const Database::PurePath& path)
        : Asset(database, path, AssetType::text) {}

    inline const str& TextAsset::text() const {
        return payload<str>();
    }

} // namespace mloader
