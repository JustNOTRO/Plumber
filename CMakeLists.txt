cmake_minimum_required(VERSION 4.0)
project(Plumber)

set(CMAKE_CXX_STANDARD 23)

add_executable(Plumber src/main.cpp src/httplib.h src/config.yaml
        src/Server.h
        src/Server.cpp
)

include(FetchContent)

FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)

target_link_libraries(Plumber PRIVATE yaml-cpp::yaml-cpp)


FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG develop
)

FetchContent_MakeAvailable(nlohmann_json)

target_link_libraries(Plumber PRIVATE nlohmann_json::nlohmann_json)