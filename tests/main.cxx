
#include "mtl/testing.hxx"

int main() {
    auto summary = mtl::testing::run_registered_tests();
    return summary.success() ? 0 : 1;
}
