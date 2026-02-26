include(FetchContent)

FetchContent_Declare(
        prometheus-cpp
        GIT_REPOSITORY https://github.com/jupp0r/prometheus-cpp.git
        GIT_TAG master
)

FetchContent_MakeAvailable(prometheus-cpp)