cmake_minimum_required(VERSION 4.0)
project(Plumber)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(CPPHTTPLIB_OPENSSL_SUPPORT)
find_package(OpenSSL REQUIRED)

# third party libraries
include(third_party/cpp-httplib.cmake)
include(third_party/nlohmann-json.cmake)
include(third_party/spdlog.cmake)
include(third_party/efsw.cmake)

add_executable(Plumber
        src/main.cpp
        src/server/Server.cpp
        src/server/wrapper/HttpsServer.hpp
        src/server/wrapper/HttpServer.hpp
        src/server/Job.cpp
        src/managers/JobManager.cpp
        src/factory/ServerFactory.cpp
        src/listeners/FileModifiedListener.cpp
        src/utils/ServerUtils.cpp
)

target_link_libraries(Plumber PRIVATE
        httplib
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        efsw
)