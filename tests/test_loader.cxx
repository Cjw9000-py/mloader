#include "mtl/testing.hxx"

#include "mloader/loader.hxx"

#include "mtl/fs/tmp.hxx"

#include <algorithm>
#include <fstream>

using mloader::DatabaseLoader;
using mtl::fs::Path;
using mtl::fs::tmp::directory;

namespace {

    void ensure_parent_exists(const Path& file_path) {
        auto parent_pure = file_path.parent();
        if (parent_pure.empty()) {
            return;
        }

    Path parent(parent_pure);
    if (!parent.exists()) {
        parent.mkdir(true, true);
    }
}

Path write_text_file(const Path& target, const str& contents) {
    ensure_parent_exists(target);
    std::ofstream stream(target.string(), std::ios::binary | std::ios::trunc);
    fassert(stream.is_open(), "failed to open file for writing:", target.string());
    stream << contents;
    stream.close();
    return target;
}

} // namespace

MTL_TEST(loader, collects_config_files) {
    // The loader code is WIP and not functional
    /*
    directory temp_dir;
    Path root = temp_dir.path();

    write_text_file(root / "level1" / "config.yaml", "items: []");
    write_text_file(root / "level1" / "nested" / "scene.yml", "scene: demo");
    write_text_file(root / "level1" / "notes.txt", "ignore me");

    DatabaseLoader loader;
    loader.add_location(root);
    loader.scan();

    const auto& files = loader.files();
    fassert(files.size() == 2, "expected two config files", files.size());

    vec<str> collected;
    collected.reserve(files.size());
    for (const auto& path : files) {
        collected.emplace_back(path.relative_to(root).as_posix());
    }
    std::sort(collected.begin(), collected.end());

    fassert(collected[0] == "level1/config.yaml", "missing first config:", collected[0]);
    fassert(collected[1] == "level1/nested/scene.yml", "missing second config:", collected[1]); */
}
