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
        GIT_TAG 5.0
        GIT_PROGRESS true
)

FetchContent_Declare(
        cereal
        GIT_REPOSITORY "https://github.com/USCiLab/cereal"
        GIT_TAG v1.3.2
        GIT_PROGRESS true
        SYSTEM
)

set(BUILD_DOC OFF)
set(BUILD_SANDBOX OFF)
set(SKIP_PERFORMANCE_COMPARISON ON)

FetchContent_MakeAvailable(cereal)
FetchContent_MakeAvailable(raylib)

add_executable(netsketch main.cpp draw.h global_client.cpp global_client.h)
# add_executable(client client.cpp client.h connect_server.cpp connect_server.h draw.h global_client.cpp global_client.h global_clientb.cpp global_clientb.h)
add_executable(server server.cpp server.h connect_server.cpp connect_server.h draw.h)



# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
  #  target_link_libraries(client PRIVATE "-framework IOKit")
    target_link_libraries(netsketch PRIVATE "-framework IOKit")
    target_link_libraries(netsketch PRIVATE "-framework Cocoa")
  #  target_link_libraries(client PRIVATE "-framework Cocoa")
    target_link_libraries(netsketch PRIVATE "-framework OpenGL")
   # target_link_libraries(client PRIVATE "-framework OpenGL")
endif ()

target_link_libraries(netsketch PRIVATE
        raylib cereal::cereal
        )
#target_link_libraries(client PRIVATE
      #  raylib cereal::cereal
      #  )
target_link_libraries(server PRIVATE
        raylib cereal::cereal
        )
target_include_directories(netsketch PRIVATE
        .
        )

#(base) juliannevella@Juliannes-MacBook-Pro build % gmake -j 8