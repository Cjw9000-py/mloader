#include "mtl/testing.hxx"

#include "mloader/asset.hxx"
#include "mloader/database/file.hxx"

#include "mtl/fs/tmp.hxx"

#include <algorithm>
#include <filesystem>
#include <fstream>

using mloader::BinaryAsset;
using mloader::FilesystemDatabase;
using mloader::TextAsset;
using mtl::fs::Path;
using mtl::fs::tmp::directory;

namespace {

    void ensure_parent_exists(const Path& file_path) {
        std::filesystem::path parent = std::filesystem::path(file_path.string()).parent_path();
        if (parent.empty()) {
            return;
        }
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        fassert(!ec, "failed to create directories:", parent.string(), ec.message());
    }

    void write_bytes(const Path& target, const vec<byte>& data) {
        ensure_parent_exists(target);
        std::ofstream stream(target.string(), std::ios::binary | std::ios::trunc | std::ios::out);
        fassert(stream.is_open(), "failed to open file for writing:", target.string());
        if (!data.empty()) {
            stream.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        }
        stream.close();
    }

    void write_text(const Path& target, const str& contents) {
        ensure_parent_exists(target);
        std::ofstream stream(target.string(), std::ios::binary | std::ios::trunc | std::ios::out);
        fassert(stream.is_open(), "failed to open file for writing:", target.string());
        stream << contents;
        stream.close();
    }

} // namespace

MTL_TEST(asset, text_asset_returns_utf8_content) {
    directory temp_dir;
    Path root = temp_dir.path();

    write_text(root / "assets" / "messages" / "greeting.txt", "hello world");

    FilesystemDatabase db(root);
    db.load();
    db.activate();

    TextAsset asset("assets/messages/greeting.txt");
    fassert(asset.state() == mloader::AssetState::unloaded, "asset should start unloaded");
    const str& contents = asset.text();
    fassert(contents == "hello world", "unexpected text payload:", contents);
    fassert(asset.state() == mloader::AssetState::parsed, "asset state should be parsed after access");
    fassert(asset.text() == "hello world", "cached text retrieval mismatch");

    db.deactivate();
}

MTL_TEST(asset, binary_asset_loads_raw_bytes) {
    directory temp_dir;
    Path root = temp_dir.path();

    vec<byte> payload{0xDE, 0xAD, 0xBE, 0xEF};
    write_bytes(root / "data" / "blob.bin", payload);

    FilesystemDatabase db(root);
    db.load();
    db.activate();

    BinaryAsset blob("data/blob.bin");
    const auto& data = blob.data();
    fassert(data.size() == payload.size(), "payload size mismatch", data.size());
    fassert(std::equal(data.begin(), data.end(), payload.begin(), payload.end()), "payload content mismatch");

    db.deactivate();
}
