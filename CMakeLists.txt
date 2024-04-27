cmake_minimum_required(VERSION 3.0)
project(netsketch CXX C)
set(CMAKE_CXX_STANDARD 17)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

add_executable(netsketch main.cpp)

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(netsketch PRIVATE "-framework IOKit")
    target_link_libraries(netsketch PRIVATE "-framework Cocoa")
    target_link_libraries(netsketch PRIVATE "-framework OpenGL")
endif ()

target_link_libraries(netsketch PRIVATE
        raylib
        )
target_include_directories(netsketch PRIVATE
        .
        )