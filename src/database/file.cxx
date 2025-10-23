#include "mloader/database/file.hxx"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "mtl/error.hxx"

using namespace mloader;

namespace {

    using mtl::fs::Path;
    using mtl::fs::PurePath;

    [[nodiscard]] Path prepare_root(const Path& root) {
        Path resolved = root;
        resolved = resolved.expanduser();
        if (!resolved.is_absolute()) {
            resolved = resolved.absolute();
        }
        return resolved.resolve();
    }

    [[nodiscard]] Path join_under(const Path& base, const PurePath& relative) {
        Path combined = base;
        const str rel_string = relative.string();
        if (!rel_string.empty()) {
            combined.with(rel_string);
        }
        return combined;
    }

    class FilesystemResource final : public Resource {
    public:
        FilesystemResource(Database& owner, Path absolute, vec<byte> data)
            : Resource(owner), m_absolute(std::move(absolute)), m_data(std::move(data)) {}

        const void* data() const override {
            return m_data.empty() ? nullptr : m_data.data();
        }

        u64 size() const override {
            return static_cast<u64>(m_data.size());
        }

    private:
        Path m_absolute;
        vec<byte> m_data;
    };

} // namespace

FilesystemDatabase::FilesystemDatabase(const Path& root) {
    set_root(root);
}

void FilesystemDatabase::set_root(const Path& root) {
    if (m_root == root) {
        return;
    }

    m_root = root;
    if (m_loaded) {
        unload();
    }
}

const FilesystemDatabase::Path& FilesystemDatabase::root() const {
    return m_root;
}

bool FilesystemDatabase::is_loaded() const noexcept {
    return m_loaded;
}

FilesystemDatabase& FilesystemDatabase::load() {
    if (m_loaded) {
        return *this;
    }

    if (m_root.empty()) {
        throw RuntimeError("FilesystemDatabase root path is empty.");
    }

    Path resolved_root = prepare_root(m_root);
    if (!resolved_root.exists()) {
        throw RuntimeError("FilesystemDatabase root does not exist: " + resolved_root.string());
    }
    if (!resolved_root.is_dir()) {
        throw RuntimeError("FilesystemDatabase root is not a directory: " + resolved_root.string());
    }

    collect_entries(resolved_root);
    m_resolved_root = std::move(resolved_root);
    m_loaded = true;
    return *this;
}

FilesystemDatabase& FilesystemDatabase::unload() {
    m_entries.clear();
    m_resolved_root = {};
    m_loaded = false;
    return *this;
}

void FilesystemDatabase::ensure_loaded() const {
    if (!m_loaded) {
        const_cast<FilesystemDatabase*>(this)->load();
    }
}

FilesystemDatabase::PurePath FilesystemDatabase::normalise(const PurePath& rel) const {
    if (rel.is_absolute()) {
        throw RuntimeError("Database paths must be relative: " + rel.as_posix());
    }

    str posix = rel.as_posix();
    while (!posix.empty() && posix.front() == '/') {
        posix.erase(posix.begin());
    }
    while (!posix.empty() && posix.rfind("./", 0) == 0) {
        posix.erase(0, 2);
    }
    while (!posix.empty() && posix.back() == '/') {
        posix.pop_back();
    }
    if (posix == ".") {
        posix.clear();
    }
    if (posix.empty()) {
        return PurePath();
    }
    const str canonical = PurePath(posix).as_posix();
    return canonical.empty() ? PurePath() : PurePath(canonical);
}

vec<Database::Entry> FilesystemDatabase::list() {
    ensure_loaded();
    return m_entries;
}

vec<Database::Entry> FilesystemDatabase::list(const PurePath& rel) {
    ensure_loaded();

    auto relative = normalise(rel);
    if (relative.string().empty()) {
        return list();
    }

    const str filter = relative.as_posix();
    str prefix = filter;
    if (!prefix.empty()) {
        prefix += '/';
    }

    vec<Entry> subset;
    for (const auto& entry : m_entries) {
        const str entry_str = entry.path.as_posix();
        if (entry_str == filter || (!prefix.empty() && entry_str.rfind(prefix, 0) == 0)) {
            subset.emplace_back(entry);
        }
    }
    return subset;
}

ResourceHandle FilesystemDatabase::resolve(const PurePath& rel) {
    ensure_loaded();

    auto relative = normalise(rel);
    if (relative.string().empty()) {
        throw RuntimeError("Cannot resolve the database root as a resource.");
    }

    const Entry* entry = find_entry(relative);
    if (!entry) {
        throw RuntimeError("Failed to resolve resource: " + relative.as_posix());
    }

    if (!is_file(relative)) {
        throw RuntimeError("Requested path is not a file: " + relative.as_posix());
    }

    Path absolute = make_absolute(relative);
    auto data = absolute.read_bytes();
    auto* resource = new FilesystemResource(*this, std::move(absolute), std::move(data));
    return ResourceHandle(resource);
}

bool FilesystemDatabase::exists(const PurePath& rel) const {
    ensure_loaded();
    auto relative = normalise(rel);
    if (relative.string().empty()) {
        return true;
    }
    return find_entry(relative) != nullptr;
}

bool FilesystemDatabase::is_file(const PurePath& rel) const {
    ensure_loaded();
    auto relative = normalise(rel);
    if (relative.string().empty()) {
        return false;
    }
    Path absolute = make_absolute(relative);
    return absolute.exists() && absolute.is_file();
}

bool FilesystemDatabase::is_dir(const PurePath& rel) const {
    ensure_loaded();
    auto relative = normalise(rel);
    if (relative.string().empty()) {
        return true;
    }
    Path absolute = make_absolute(relative);
    return absolute.exists() && absolute.is_dir();
}

FilesystemDatabase::Path FilesystemDatabase::make_absolute(const PurePath& rel) const {
    if (!m_loaded) {
        throw RuntimeError("FilesystemDatabase has not been loaded.");
    }
    Path absolute = join_under(m_resolved_root, rel);
    return absolute.resolve();
}

FilesystemDatabase::Entry* FilesystemDatabase::find_entry(const PurePath& rel) {
    auto relative = normalise(rel);
    const str target = relative.as_posix();
    for (auto& entry : m_entries) {
        if (entry.path.as_posix() == target) {
            return &entry;
        }
    }
    return nullptr;
}

const FilesystemDatabase::Entry* FilesystemDatabase::find_entry(const PurePath& rel) const {
    auto relative = normalise(rel);
    const str target = relative.as_posix();
    for (const auto& entry : m_entries) {
        if (entry.path.as_posix() == target) {
            return &entry;
        }
    }
    return nullptr;
}

void FilesystemDatabase::collect_entries(const Path& resolved_root) {
    m_entries.clear();

    std::unordered_set<str> seen;
    auto add_entry = [&](const Path& absolute) {
        auto relative_path = absolute.relative_to(resolved_root);
        str rel_string = relative_path.string();
        if (rel_string.empty()) {
            return;
        }
        PurePath rel(rel_string);
        str key = rel.as_posix();
        if (seen.insert(key).second) {
            Entry entry;
            entry.path = rel;
            entry.db = this;
            m_entries.emplace_back(std::move(entry));
        }
    };

    for (const auto& walk_entry : resolved_root.walk()) {
        Path current = walk_entry.path;
        str current_rel = current.relative_to(resolved_root).string();
        if (!current_rel.empty()) {
            add_entry(current);
        }

        for (const auto& dir_name : walk_entry.dirs) {
            add_entry((walk_entry.path / dir_name).resolve());
        }
        for (const auto& file_name : walk_entry.files) {
            add_entry((walk_entry.path / file_name).resolve());
        }
    }

    std::sort(m_entries.begin(), m_entries.end(), [](const Entry& lhs, const Entry& rhs) {
        return lhs.path.as_posix() < rhs.path.as_posix();
    });
}
