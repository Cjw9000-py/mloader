#include "mtl/testing.hxx"

#include "mloader/database/base.hxx"
#include "mloader/resource.hxx"

using mloader::Database;
using mloader::Resource;
using mloader::ResourceHandle;

namespace {

    struct TestDatabase final : Database {
        bool loaded = true;

        prop bool is_loaded() const noexcept override {
            return loaded;
        }

        Database& load() override {
            loaded = true;
            return *this;
        }

        Database& unload() override {
            loaded = false;
            return *this;
        }

        vec<Entry> list() override {
            return vec<Entry>{};
        }

        vec<Entry> list(const PurePath&) override {
            return vec<Entry>{};
        }

        ResourceHandle resolve(const PurePath&) override {
            return ResourceHandle();
        }

        bool exists(const PurePath&) const override {
            return false;
        }

        bool is_file(const PurePath&) const override {
            return false;
        }

        bool is_dir(const PurePath&) const override {
            return false;
        }
    };

    struct TestResource final : Resource {
        TestResource(Database& owner, bool* destroyed_flag, int* destroy_counter)
            : Resource(owner), destroyed(destroyed_flag), destroy_calls(destroy_counter) {}

        const void* data() const override {
            return nullptr;
        }

        u64 size() const override {
            return 0;
        }

    protected:
        void destroy_self() override {
            if (destroy_calls) {
                ++(*destroy_calls);
            }
            if (destroyed) {
                *destroyed = true;
            }
            delete this;
        }

    private:
        bool* destroyed;
        int* destroy_calls;
    };

} // namespace

MTL_TEST(resource, handle_destroys_resource_when_last_reference_released) {
    TestDatabase db;
    bool destroyed = false;
    int destroy_calls = 0;

    {
        auto* res = new TestResource(db, &destroyed, &destroy_calls);
        ResourceHandle handle(res);
        fassert(!destroyed, "resource should remain alive while handle exists");
    }

    fassert(destroyed, "resource destruction not triggered");
    fassert(destroy_calls == 1, "resource destroy should occur exactly once");
}

MTL_TEST(resource, copy_handle_shares_resource_until_all_destroyed) {
    TestDatabase db;
    bool destroyed = false;
    int destroy_calls = 0;

    {
        auto* res = new TestResource(db, &destroyed, &destroy_calls);
        ResourceHandle primary(res);
        {
            ResourceHandle secondary(primary);
            fassert(!destroyed, "resource should survive while any handle exists");
        }
        fassert(!destroyed, "resource should survive until last handle released");
    }

    fassert(destroyed, "resource should destroy after all handles");
    fassert(destroy_calls == 1, "resource destroy count mismatch");
}

MTL_TEST(resource, move_assignment_releases_previous_resource) {
    TestDatabase db;
    bool destroyed_first = false;
    bool destroyed_second = false;
    int destroy_calls_first = 0;
    int destroy_calls_second = 0;

    auto* first = new TestResource(db, &destroyed_first, &destroy_calls_first);
    auto* second = new TestResource(db, &destroyed_second, &destroy_calls_second);

    ResourceHandle handle(first);
    {
        ResourceHandle temp(second);
        handle = std::move(temp);
        fassert(destroyed_first, "move assignment should release previous resource");
        fassert(!destroyed_second, "new resource must remain alive after move");
        fassert(destroy_calls_first == 1, "previous resource should be destroyed exactly once");
    }

    fassert(!destroyed_second, "resource should remain until final release");

    ResourceHandle empty;
    handle = std::move(empty);

    fassert(destroyed_second, "resource should destroy after final handle cleared");
    fassert(destroy_calls_second == 1, "new resource destroy count mismatch");
}
