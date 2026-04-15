add_executable(test_requests test_requests.cpp)
target_link_libraries(test_requests PRIVATE
        requests
        GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(test_requests)