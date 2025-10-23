#include "mloader/resource.hxx"

#include <utility>

using namespace mloader;

Resource::Resource(Database& owner)
    : m_db(&owner) {}

Database& Resource::db() const {
    return *m_db;
}

void Resource::inc_ref() {
    ++m_refcount;
}

void Resource::dec_ref() {
    if (--m_refcount == 0) {
        destroy_self();
    }
}

void Resource::destroy_self() {
    delete this;
}

ResourceHandle::ResourceHandle(Resource* res)
    : m_res(res) {
    if (m_res) {
        m_res->inc_ref();
    }
}

ResourceHandle::ResourceHandle(const ResourceHandle& other)
    : m_res(other.m_res) {
    if (m_res) {
        m_res->inc_ref();
    }
}

ResourceHandle::ResourceHandle(ResourceHandle&& other) noexcept
    : m_res(std::exchange(other.m_res, nullptr)) {}

ResourceHandle::~ResourceHandle() {
    if (m_res) {
        m_res->dec_ref();
    }
}

ResourceHandle& ResourceHandle::operator=(const ResourceHandle& other) {
    if (this == &other) {
        return *this;
    }
    if (m_res) {
        m_res->dec_ref();
    }
    m_res = other.m_res;
    if (m_res) {
        m_res->inc_ref();
    }
    return *this;
}

ResourceHandle& ResourceHandle::operator=(ResourceHandle&& other) noexcept {
    if (m_res) {
        m_res->dec_ref();
    }
    m_res = std::exchange(other.m_res, nullptr);
    return *this;
}

Resource* ResourceHandle::operator->() const {
    return m_res;
}

Resource& ResourceHandle::operator*() const {
    return *m_res;
}

bool ResourceHandle::valid() const {
    return m_res != nullptr;
}

