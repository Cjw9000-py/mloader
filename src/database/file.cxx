#include "mloader/database/file.hxx"

using namespace mloader;


FilesystemDatabase::FilesystemDatabase(const Path &root) {
    m_root = root;
}

bool FilesystemDatabase::is_loaded() const noexcept {
    return true;
}

void FilesystemDatabase::load() {
    // do nothing
}

void FilesystemDatabase::unload() {
    // do nothing
}

vec<Path> FilesystemDatabase::list() {

}




