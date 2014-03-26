cmake_minimum_required(VERSION 2.8)

include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)

project(ipdf)

set(CMAKE_CXX_FLAGS "-std=gnu++0x")

include_directories(${SDL2_INCLUDE_DIRS})

add_library(ipdflib
common.h
log.h
log.cpp
ipdf.h
view.h
view.cpp
document.h
document.cpp
screen.h
screen.cpp
)

add_executable(ipdf
main.cpp
)

target_link_libraries(ipdf
ipdflib
GL
${SDL2_LIBRARIES}
)

add_executable(ipdf-testsaveload
tests/saveload.cpp
)

target_link_libraries(ipdf-testsaveload
ipdflib
GL
${SDL2_LIBRARIES}
)
