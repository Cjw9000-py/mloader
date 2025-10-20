#pragma once

#include "mtl/common.hxx"

#include <atomic>
#include <utility>

namespace mloader {

    struct Database;

    /**
     * Base class representing a contiguous data payload owned by a Database.
     * Concrete database implementations provide derived resources that expose
     * their raw memory while retaining ownership inside the database.
     */
    struct Resource {
        ctor Resource(Database& owner) : m_db(&owner) {}
        virt ~Resource() = default;

        /// @return Database that created and manages this resource instance.
        use Database& db() const {
            return *m_db;
        }

        /// @return Pointer to the immutable data backing the resource.
        virt const void* data() const = 0;
        /// @return Size in bytes of the resource payload.
        virt u64 size() const = 0;

        /// Increases the external reference count.
        void inc_ref() {
            ++m_refcount;
        }

        /// Decreases the external reference count and destroys at zero.
        void dec_ref() {
            if (--m_refcount == 0) {
                destroy_self();
            }
        }

    protected:
        /// Allows derived classes to customise destruction strategies.
        virt void destroy_self() {
            delete this;
        }

    private:
        Database* m_db;
        std::atomic<u32> m_refcount{0};
    };

    /**
     * Reference-counting handle for Resource instances. Handles mirror the
     * behaviour of lightweight smart pointers while delegating ownership to the
     * producing Database through explicit reference counting.
     */
    struct ResourceHandle {
        ctor ResourceHandle(Resource* res = nullptr)
            : m_res(res) {
            if (m_res) {
                m_res->inc_ref();
            }
        }

        ResourceHandle(const ResourceHandle& other)
            : m_res(other.m_res) {
            if (m_res) {
                m_res->inc_ref();
            }
        }

        ResourceHandle(ResourceHandle&& other) noexcept
            : m_res(std::exchange(other.m_res, nullptr)) {}

        ~ResourceHandle() {
            if (m_res) {
                m_res->dec_ref();
            }
        }

        ResourceHandle& operator=(const ResourceHandle& other) {
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

        ResourceHandle& operator=(ResourceHandle&& other) noexcept {
            if (m_res) {
                m_res->dec_ref();
            }
            m_res = std::exchange(other.m_res, nullptr);
            return *this;
        }

        use Resource* operator->() const {
            return m_res;
        }

        use Resource& operator*() const {
            return *m_res;
        }

        use bool valid() const {
            return m_res != nullptr;
        }

    private:
        Resource* m_res;
    };

} // namespace mloader
