cmake_minimum_required(VERSION 3.10)


project(mloader VERSION 0.1)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MTL_ENABLE_EXTENDED ON)
add_subdirectory(extern/MTL)


### mloader Library

add_library(mloader STATIC
        inc/mloader/asset.hxx
        inc/mloader/scanner.hxx
        inc/mloader/resource.hxx
        inc/mloader/defs/definition.hxx
        inc/mloader/defs/registry.hxx
        src/defs/registry.cxx
        inc/mloader/database/base.hxx
        inc/mloader/database/file.hxx
        inc/mloader/database/registry.hxx
        src/asset.cxx
        src/database/file.cxx
        src/database/registry.cxx
        src/resource.cxx
)
target_include_directories(mloader PUBLIC inc)
target_link_libraries(mloader PUBLIC MTL)


### Example

add_subdirectory(example)


###  Tests

add_executable(test_main
    tests/main.cxx
    tests/test_asset.cxx
    tests/test_scanner.cxx
    tests/test_filesystem_db.cxx
    tests/test_resource.cxx
)
target_link_libraries(test_main PRIVATE mloader)
