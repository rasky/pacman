cmake_minimum_required(VERSION 2.6)
set(CMAKE_BUILD_TYPE "Debug")

include(FindSDL)
find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})
add_executable(pacman pacman.c hw.c gfx.c Z80/Z80.c)
target_link_libraries(pacman ${SDL_LIBRARY} SDL_gfx)
