#include "mloader/database/file.hxx"

#include <algorithm>
#include <utility>

#include "mtl/error.hxx"

using namespace mloader;

namespace {
    [[nodiscard]] static Path prepare_root(const Path& root) {
        Path resolved = root;
        resolved = resolved.expanduser();
        if (!resolved.is_absolute()) {
            resolved = resolved.absolute();
        }
        return resolved;
    }

    [[nodiscard]] static Path join_under(const Path& base, const Path& relative) {
        if (relative.empty()) {
            return base;
        }

        Path combined = base;
        combined.with(relative.string());
        return combined;
    }
}

use bool FilesystemDatabase::Entry::matches(const Path& candidate) const {
    if (candidate.empty()) {
        return false;
    }

    if (candidate.is_absolute()) {
        const str& target = candidate.string();
        const str& absolute_str = absolute.string();
        if (absolute_str == target) {
            return true;
        }
        return absolute_str == candidate.resolve().string();
    }
    return relative.string() == candidate.string();
}

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

prop const Path& FilesystemDatabase::root() cx {
    return m_root;
}

prop bool FilesystemDatabase::is_loaded() cx {
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

    resolved_root = resolved_root.resolve();
    collect_entries(resolved_root);
    m_resolved_root = resolved_root;
    m_loaded = true;
    return *this;
}

FilesystemDatabase& FilesystemDatabase::unload() {
    m_entries.clear();
    m_resolved_root = {};
    m_loaded = false;
    return *this;
}

vec<Path> FilesystemDatabase::list() {
    if (!m_loaded) {
        load();
    }

    vec<Path> paths;
    paths.reserve(m_entries.size());
    for (const auto& entry : m_entries) {
        paths.emplace_back(entry.relative);
    }
    return paths;
}

Resource FilesystemDatabase::resolve(const Path& path) {
    if (!m_loaded) {
        load();
    }

    Path absolute = make_absolute(path);
    Entry* entry = find_entry(absolute);
    if (!entry) {
        throw RuntimeError("Failed to resolve resource: " + path.string());
    }

    if (auto snapshot = mtl::fs::meta::inspect_if(entry->absolute)) {
        entry->metadata = *snapshot;
    }

    return Resource{};
}

use Path FilesystemDatabase::make_absolute(const Path& path) const {
    if (path.empty()) {
        return m_resolved_root;
    }

    if (path.is_absolute()) {
        return path.resolve();
    }

    Path absolute = join_under(m_resolved_root, path);
    absolute = absolute.resolve();

    if (!absolute.is_relative_to(m_resolved_root)) {
        throw RuntimeError("Path escapes database root: " + path.string());
    }

    return absolute;
}

FilesystemDatabase::Entry* FilesystemDatabase::find_entry(const Path& absolute) {
    const str& target = absolute.string();
    auto it = std::find_if(m_entries.begin(), m_entries.end(), [&](const Entry& entry) {
        return entry.absolute.string() == target;
    });
    if (it == m_entries.end()) {
        return nullptr;
    }
    return &(*it);
}

const FilesystemDatabase::Entry* FilesystemDatabase::find_entry(const Path& absolute) const {
    const str& target = absolute.string();
    auto it = std::find_if(m_entries.cbegin(), m_entries.cend(), [&](const Entry& entry) {
        return entry.absolute.string() == target;
    });
    if (it == m_entries.cend()) {
        return nullptr;
    }
    return &(*it);
}

void FilesystemDatabase::collect_entries(const Path& resolved_root) {
    m_entries.clear();

    for (const auto& walk_entry : resolved_root.walk()) {
        for (const auto& filename : walk_entry.files) {
            Path absolute = (walk_entry.path / filename).resolve();

            Entry entry;
            entry.absolute = absolute;

            if (absolute.is_relative_to(resolved_root)) {
                auto relative_path = absolute.relative_to(resolved_root);
                entry.relative = Path(relative_path.string());
            } else {
                entry.relative = Path(filename);
            }

            if (auto snapshot = mtl::fs::meta::inspect_if(absolute)) {
                entry.metadata = *snapshot;
            } else {
                entry.metadata = {};
            }

            m_entries.emplace_back(std::move(entry));
        }
    }

    std::sort(m_entries.begin(), m_entries.end(), [](const Entry& lhs, const Entry& rhs) {
        return lhs.relative.string() < rhs.relative.string();
    });
}
