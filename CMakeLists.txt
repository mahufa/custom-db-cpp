cmake_minimum_required(VERSION 3.29)
project(DatabaseProject)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt
        GIT_TAG         11.0.2
)

FetchContent_MakeAvailable(fmt)

add_executable(DatabaseProject main.cpp
        Database.cpp
        Database.h
        StructureElements.cpp
        StructureElements.h
        ProjectUtilities.cpp
        ProjectUtilities.h)

target_link_libraries(DatabaseProject fmt)