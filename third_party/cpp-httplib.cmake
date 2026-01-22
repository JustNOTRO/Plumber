include(FetchContent)

FetchContent_Declare(
        httplib SYSTEM
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib
        GIT_TAG master
)

FetchContent_MakeAvailable(httplib)