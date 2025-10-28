cmake_minimum_required(VERSION 3.10)


project(mloader_example VERSION 0.1)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mloader_example
        main.cxx
)
target_link_libraries(mloader_example PUBLIC mloader)

