cmake_minimum_required(VERSION 4.0)
project(Plumber)

set(CMAKE_CXX_STANDARD 23)

add_compile_definitions(CPPHTTPLIB_OPENSSL_SUPPORT)

find_package(OpenSSL REQUIRED)

add_executable(Plumber src/main.cpp src/httplib.h config.yaml
        src/server/Server.cpp
        src/server/Server.hpp
        src/data/Config.cpp
        src/data/Config.hpp
        src/managers/JobManager.cpp
        src/managers/JobManager.hpp
        src/Job.cpp
        src/Job.hpp
)

include(FetchContent)

FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)

target_link_libraries(Plumber PRIVATE yaml-cpp::yaml-cpp OpenSSL::SSL OpenSSL::Crypto)


FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG develop
)

FetchContent_MakeAvailable(nlohmann_json)

target_link_libraries(Plumber PRIVATE nlohmann_json::nlohmann_json)


FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x
)

FetchContent_MakeAvailable(spdlog)

target_link_libraries(Plumber PRIVATE spdlog::spdlog)
