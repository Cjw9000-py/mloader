#include "mtl/testing.hxx"

#include "mloader/scanner.hxx"
#include "mloader/database/file.hxx"

#include "mtl/fs/tmp.hxx"

#include <algorithm>
#include <filesystem>
#include <fstream>

using mloader::DatabaseScanner;
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

Path write_text_file(const Path& target, const str& contents) {
    ensure_parent_exists(target);
    std::ofstream stream(
        target.string(),
        std::ios::binary | std::ios::trunc | std::ios::out
    );
    fassert(stream.is_open(), "failed to open file for writing:", target.string());
    stream << contents;
    stream.close();
    return target;
}

} // namespace

MTL_TEST(scanner, collects_config_files) {
    directory temp_dir;
    Path root = temp_dir.path();

    write_text_file(root / "level1" / "config.yaml", "items: []");
    write_text_file(root / "level1" / "nested" / "scene.yml", "scene: demo");
    write_text_file(root / "level1" / "notes.txt", "ignore me");

    mloader::FilesystemDatabase db(root);

    DatabaseScanner scanner(db);
    scanner.scan();

    const auto& configs = scanner.configs();
    fassert(configs.size() == 2, "expected two config files", configs.size());

    vec<str> collected;
    collected.reserve(configs.size());
    for (const auto& cfg : configs) {
        fassert(cfg.db == &db, "config entry should report originating database");
        fassert(cfg.resource.valid(), "config resource handle must be valid");
        collected.emplace_back(cfg.path.as_posix());

        str payload(static_cast<const char*>(cfg.resource->data()),
                    static_cast<usize>(cfg.resource->size()));
        fassert(!payload.empty(), "config payload should not be empty");
    }
    std::sort(collected.begin(), collected.end());

    fassert(collected[0] == "level1/config.yaml", "missing first config:", collected[0]);
    fassert(collected[1] == "level1/nested/scene.yml", "missing second config:", collected[1]);
}
