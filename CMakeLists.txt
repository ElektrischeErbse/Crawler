cmake_minimum_required(VERSION 3.22)

project(crawler)

add_subdirectory(requests)

add_executable(test_requests test_requests.cpp)
target_link_libraries(test_requests PRIVATE requests)