#include "mtl/testing.hxx"

#include "mloader/database/file.hxx"
#include "mloader/resource.hxx"

#include "mtl/error.hxx"
#include "mtl/fs/tmp.hxx"

#include <algorithm>

using mloader::FilesystemDatabase;
using mloader::ResourceHandle;
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
        target.write_text(contents);
        return target;
    }

} // namespace

MTL_TEST(filesystem_db, discovers_entries) {
    directory temp_dir;
    Path root = temp_dir.path();

    write_text_file(root / "assets" / "levels" / "intro.txt", "intro level");
    write_text_file(root / "assets" / "levels" / "boss.txt", "boss level");
    (root / "assets" / "textures").mkdir(true, true);

    FilesystemDatabase db(root);
    db.load();

    fassert(db.is_loaded(), "database should be marked loaded");

    auto entries = db.list();
    fassert(entries.size() == 5, "expected 5 entries (assets dir + contents)", entries.size());

    vec<str> names;
    names.reserve(entries.size());
    for (const auto& entry : entries) {
        names.emplace_back(entry.path.as_posix());
        fassert(entry.db == &db, "entry should reference database");
    }
    std::sort(names.begin(), names.end());

    fassert(names == vec<str>{
        "assets",
        "assets/levels",
        "assets/levels/boss.txt",
        "assets/levels/intro.txt",
        "assets/textures"
    }, "unexpected entries");

    fassert(db.is_dir(FilesystemDatabase::PurePath("assets/textures")), "textures directory should be detected");
}

MTL_TEST(filesystem_db, resolves_files_to_resources) {
    directory temp_dir;
    Path root = temp_dir.path();

    const str data = "payload";
    write_text_file(root / "data" / "file.bin", data);

    FilesystemDatabase db(root);
    db.load();

    auto handle = db.resolve(FilesystemDatabase::PurePath("data/file.bin"));
    fassert(handle.valid(), "resource handle should be valid");
    fassert(handle->size() == data.size(), "resource size mismatch", handle->size());

    auto* raw = static_cast<const char*>(handle->data());
    fassert(raw != nullptr, "resource data should not be null");
    str resolved(raw, raw + handle->size());
    fassert(resolved == data, "resource payload mismatch", resolved);
}

MTL_TEST(filesystem_db, reports_missing_entries) {
    directory temp_dir;
    Path root = temp_dir.path();

    FilesystemDatabase db(root);
    db.load();

    auto missing = FilesystemDatabase::PurePath("missing.file");
    fassert(!db.exists(missing), "missing path should not exist");
    fassert(!db.is_file(missing), "missing path should not be a file");
    fassert(!db.is_dir(missing), "missing path should not be a dir");

    bool threw = false;
    try {
        (void)db.resolve(missing);
    } catch (const RuntimeError&) {
        threw = true;
    }
    fassert(threw, "resolving missing path should throw");
}

