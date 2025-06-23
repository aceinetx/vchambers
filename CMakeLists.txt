cmake_minimum_required(VERSION 3.20)
project(vchambers)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cc")
set(PLATFORM_SOURCES "")

if(UNIX)
    file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS "platform/unix/*.cc")
elseif(WIN32)
    file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS "platform/win32/*.cc")
endif()

set(SOURCES ${SOURCES} ${PLATFORM_SOURCES})

include_directories("src")

add_executable(vchambers ${SOURCES})
target_link_libraries(vchambers ncurses)
