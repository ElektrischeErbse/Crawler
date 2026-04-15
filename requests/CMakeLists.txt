find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(requests requests.cpp)
target_include_directories(requests PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(requests PUBLIC
        CURL::libcurl
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
)