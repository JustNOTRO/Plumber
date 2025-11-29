cmake_minimum_required(VERSION 4.0)
project(Plumber)

set(CMAKE_CXX_STANDARD 23)

add_executable(Plumber src/main.cpp src/httplib.h src/config.yaml)

include(FetchContent)

FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)

target_link_libraries(Plumber PUBLIC yaml-cpp::yaml-cpp)