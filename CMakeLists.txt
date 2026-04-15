cmake_minimum_required(VERSION 3.22)

project(crawler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(LIB_GQ libgq.a)
find_package(GTest REQUIRED)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
enable_testing()

add_subdirectory(requests)
add_subdirectory(tests)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE requests ${LIB_GQ})