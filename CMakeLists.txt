cmake_minimum_required(VERSION 4.0)
project(Plumber)

set(CMAKE_CXX_STANDARD 23)

add_compile_definitions(CPPHTTPLIB_OPENSSL_SUPPORT)

find_package(OpenSSL REQUIRED)

add_executable(Plumber src/main.cpp
        src/server/Server.cpp
        src/server/Server.hpp
        src/data/Config.cpp
        src/data/Config.hpp
        src/managers/JobManager.cpp
        src/managers/JobManager.hpp
        src/managers/Job.cpp
        src/managers/Job.hpp/
)

include(FetchContent)

FetchContent_Declare(httplib SYSTEM
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib
        GIT_TAG master)

FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG master)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG develop)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x)

FetchContent_MakeAvailable(httplib)
FetchContent_MakeAvailable(yaml-cpp)
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(Plumber PRIVATE httplib)
target_link_libraries(Plumber PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(Plumber PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(Plumber PRIVATE spdlog::spdlog)
