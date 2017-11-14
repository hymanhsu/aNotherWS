cmake_minimum_required(VERSION 2.6.4)
project(simplelog)


set(TOPDIR .)

include_directories(${TOPDIR}/include)

set(SOURCES
        ${TOPDIR}/src/simple_log.cpp
        ${TOPDIR}/src/simple_config.cpp
        )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g --std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter")
add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)
if (APPLE)
    add_definitions(-D_DARWIN_USE_64_BIT_INODE=1 -D_DARWIN_UNLIMITED_SELECT=1)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_BUILE_TYPE DEBUG)
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
else()
    add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE=200112)
    add_definitions(-Wstrict-aliasing)
    add_definitions(-std=c++11)
    add_definitions(-g)
endif()


add_library(simplelog ${SOURCES})
find_library(M_LIB NAMES m)
find_package(Threads)
target_link_libraries(simplelog ${CMAKE_THREAD_LIBS_INIT} ${M_LIB})


install(TARGETS simplelog
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )


install(
        DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h*")