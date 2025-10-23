#pragma once

#include "mtl/common.hxx"

#include <atomic>
namespace mloader {

    struct Database;

    /**
     * Base class representing a contiguous data payload owned by a Database.
     * Concrete database implementations provide derived resources that expose
     * their raw memory while retaining ownership inside the database.
     */
    struct Resource {
        explicit Resource(Database& owner);
        virt ~Resource() = default;

        /// @return Database that created and manages this resource instance.
        use Database& db() const;

        /// @return Pointer to the immutable data backing the resource.
        virt const void* data() const = 0;
        /// @return Size in bytes of the resource payload.
        virt u64 size() const = 0;

        /// Increases the external reference count.
        void inc_ref();

        /// Decreases the external reference count and destroys at zero.
        void dec_ref();

    protected:
        /// Allows derived classes to customise destruction strategies.
        virt void destroy_self();

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
        explicit ResourceHandle(Resource* res = nullptr);

        ResourceHandle(const ResourceHandle& other);
        ResourceHandle(ResourceHandle&& other) noexcept;
        ~ResourceHandle();

        ResourceHandle& operator=(const ResourceHandle& other);
        ResourceHandle& operator=(ResourceHandle&& other) noexcept;

        use Resource* operator->() const;
        use Resource& operator*() const;

        use bool valid() const;
    private:
        Resource* m_res;
    };

} // namespace mloader
