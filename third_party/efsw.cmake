include(FetchContent)

FetchContent_Declare(
        efsw
        GIT_REPOSITORY https://github.com/SpartanJ/efsw.git
        GIT_TAG master
)

FetchContent_MakeAvailable(efsw)