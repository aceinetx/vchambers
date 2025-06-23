cmake_minimum_required(VERSION 3.20)
project(vchambers)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cc")

add_executable(vchambers ${SOURCES})
target_link_libraries(vchambers ncurses)
