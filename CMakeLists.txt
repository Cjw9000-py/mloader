cmake_minimum_required(VERSION 3.10)


project(mloader VERSION 0.1)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MTL_ENABLE_EXTENDED ON)
add_subdirectory(extern/MTL)


### mloader Library

add_library(mloader STATIC
        inc/mloader/loader.hxx
)
target_include_directories(mloader PUBLIC inc)
target_link_libraries(mloader PUBLIC MTL)


### Example

add_subdirectory(example)


###  Tests

add_executable(test_main
    tests/main.cxx
)
target_link_libraries(test_main PRIVATE mloader)




