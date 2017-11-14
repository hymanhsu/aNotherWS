cmake_minimum_required(VERSION 2.8)
project(uvws)

set(LIBUVWSDIR .)

add_subdirectory(
    deps/libuv
    deps/simplelog
    )

include_directories(
        ${LIBUVWSDIR}/include
        ${LIBUVWSDIR}/deps/libuv/include
)


set(SOURCES
        ${LIBUVWSDIR}/src/ws/membuf.cpp
        ${LIBUVWSDIR}/src/ws/sha1.cpp
        ${LIBUVWSDIR}/src/ws/encode.cpp
        ${LIBUVWSDIR}/src/ws/common.cpp
        ${LIBUVWSDIR}/src/ws/wsutil.cpp
        ${LIBUVWSDIR}/src/ws/uvutil.cpp
        ${LIBUVWSDIR}/src/ws/CLoop.cpp
        ${LIBUVWSDIR}/src/ws/CThread.cpp
        ${LIBUVWSDIR}/src/ws/CTask.cpp
        ${LIBUVWSDIR}/src/ws/CAsyncs.cpp
        ${LIBUVWSDIR}/src/ws/CWSClient.cpp
        ${LIBUVWSDIR}/src/ws/CWSServer.cpp
        ${LIBUVWSDIR}/src/ws/CChecker.cpp
        )


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g --std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter")
add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)

if (APPLE)
    add_definitions(-D_DARWIN_USE_64_BIT_INODE=1 -D_DARWIN_UNLIMITED_SELECT=1)
endif()

if (NOT APPLE)
    add_definitions(-Wstrict-aliasing)
    add_definitions(-std=c++11)
    add_definitions(-g)
    #add_definitions(-static)
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_BUILE_TYPE DEBUG)
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    #add_definitions(-static)
endif()

if ((${CMAKE_SYSTEM_NAME} MATCHES "Linux") OR (${CMAKE_SYSTEM_NAME} MATCHES "Android"))
    add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE=200112)
endif()

find_path(SIMPLELOG_HEADER NAMES simple_log.h)
include_directories(${SIMPLELOG_HEADER})

add_library(uvws STATIC ${SOURCES})
target_link_libraries(uvws uv)

find_library(SIMPLELOG_LIB NAMES simplelog)
find_library(GTEST_LIB NAMES gtest gtest_main)
find_library(M_LIB NAMES m)
find_package(Threads)
target_link_libraries(uvws ${CMAKE_THREAD_LIBS_INIT} ${M_LIB} ${SIMPLELOG_LIB})


# compile for executable target By Kevin
set(ECHO_SERVER_SOURCES
        ${LIBUVWSDIR}/test/echo-server.cpp
        )
add_executable(echo-server ${ECHO_SERVER_SOURCES})
target_link_libraries(echo-server uvws ${SIMPLELOG_LIB})


set(ECHO_CLIENT_SOURCES
        ${LIBUVWSDIR}/test/echo-client.cpp
        )
add_executable(echo-client ${ECHO_CLIENT_SOURCES})
target_link_libraries(echo-client uvws ${SIMPLELOG_LIB})


set(HELLOWORLD_SOURCES
        ${LIBUVWSDIR}/test/helloworld.cpp
        )
add_executable(helloworld ${HELLOWORLD_SOURCES})
target_link_libraries(helloworld uvws ${SIMPLELOG_LIB})


set(ONCHANGE2_SOURCES
        ${LIBUVWSDIR}/test/onchange.cpp
        )
add_executable(onchange ${ONCHANGE2_SOURCES})
target_link_libraries(onchange uvws ${SIMPLELOG_LIB})


set(QUEUEWORK_SOURCES
        test/queue-work.cpp
        )
add_executable(queue-work ${QUEUEWORK_SOURCES})
target_link_libraries(queue-work uvws ${SIMPLELOG_LIB})


set(SINGLETON_TEST_SOURCES
        ${LIBUVWSDIR}/test/test-singleton.cpp
        )
add_executable(test-singleton ${SINGLETON_TEST_SOURCES})
target_link_libraries(test-singleton uvws ${SIMPLELOG_LIB})


set(TASK_TEST_SOURCES
        ${LIBUVWSDIR}/test/CTaskTest.cpp
        )
add_executable(test-task ${TASK_TEST_SOURCES})
target_link_libraries(test-task uvws ${SIMPLELOG_LIB})


set(Protol_test_SOURCES
        ${LIBUVWSDIR}/test/ProtolParseTest.cpp
        )
add_executable(ProtolParseTest ${Protol_test_SOURCES})
target_link_libraries(ProtolParseTest uvws ${SIMPLELOG_LIB})


set(WS_SERVER4_SOURCES
        ${LIBUVWSDIR}/test/ws-server4.cpp
        )
add_executable(ws-server4 ${WS_SERVER4_SOURCES})
target_link_libraries(ws-server4 uvws ${SIMPLELOG_LIB})


set(WS_CLIENT4_SOURCES
        ${LIBUVWSDIR}/test/ws-client4.cpp
        )
add_executable(ws-client4 ${WS_CLIENT4_SOURCES})
target_link_libraries(ws-client4 uvws ${SIMPLELOG_LIB})


set(WS_CLIENT_CLASS_TEST_SOURCES
        ${LIBUVWSDIR}/test/ws-client-class-varLenData.cpp
        )
add_executable(ws-client-class-varLenData ${WS_CLIENT_CLASS_TEST_SOURCES})
target_link_libraries(ws-client-class-varLenData uvws ${SIMPLELOG_LIB})

set(WS_SERVER_CLASS_TEST_SOURCES
        ${LIBUVWSDIR}/test/ws-server-class-varLenData.cpp
        )
add_executable(ws-server-class-varLenData ${WS_SERVER_CLASS_TEST_SOURCES})
target_link_libraries(ws-server-class-varLenData uvws ${SIMPLELOG_LIB})

set(WS_SERVER_TEST_WS_AUTO_SOURCES
        ${LIBUVWSDIR}/test/ws-testAuto.cpp
        )
add_executable(ws-testAuto ${WS_SERVER_TEST_WS_AUTO_SOURCES})
target_link_libraries(ws-testAuto uvws ${SIMPLELOG_LIB})

set(WS_TEST
        ${LIBUVWSDIR}/test/testWs.cpp
        )
add_executable(testWs ${WS_TEST})
target_link_libraries(testWs uvws ${SIMPLELOG_LIB} ${GTEST_LIB})


set(WS_TEST_CLIENT_BASE_SOURCES
        ${LIBUVWSDIR}/test/WsClientTestBase.cpp
        )
add_executable(WsClientTestBase ${WS_TEST_CLIENT_BASE_SOURCES})
target_link_libraries(WsClientTestBase uvws ${SIMPLELOG_LIB})

set(WS_TEST_CLIENT_BASE_NOEND_SOURCES
        ${LIBUVWSDIR}/test/WsClientTestBaseNoEndMsg.cpp
        )
add_executable(WsClientTestBaseNoEndMsg ${WS_TEST_CLIENT_BASE_NOEND_SOURCES})
target_link_libraries(WsClientTestBaseNoEndMsg uvws ${SIMPLELOG_LIB})

set(WS_TEST_CLIENT_BASE_SEND_END_SOURCES
        ${LIBUVWSDIR}/test/WsClientTestSendEnd.cpp
        )
add_executable(WsClientTestSendEnd ${WS_TEST_CLIENT_BASE_SEND_END_SOURCES})
target_link_libraries(WsClientTestSendEnd uvws ${SIMPLELOG_LIB})

set(WS_TEST_SERVER_BASE_SOURCES
        ${LIBUVWSDIR}/test/WsServerTestBase.cpp
        )
add_executable(WsServerTestBase ${WS_TEST_SERVER_BASE_SOURCES})
target_link_libraries(WsServerTestBase uvws ${SIMPLELOG_LIB})

set(WS_SERVER_TEST_MULTIPLE_CLIENTS
        ${LIBUVWSDIR}/test/ws-testMultipleClients.cpp
        )
add_executable(ws-testMultipleClients ${WS_SERVER_TEST_MULTIPLE_CLIENTS})
target_link_libraries(ws-testMultipleClients uvws ${SIMPLELOG_LIB})

set(WS_TEST_STABLE_SMALL_SIZE_DATA_SOURCES
${LIBUVWSDIR}/test/ws-testStableSmallSizeData.cpp
)
add_executable(ws-testStableSmallSizeData ${WS_TEST_STABLE_SMALL_SIZE_DATA_SOURCES})
target_link_libraries(ws-testStableSmallSizeData uvws ${SIMPLELOG_LIB})

set(WS_TEST_STABLE_MEDIUM_SIZE_DATA_SOURCES
        ${LIBUVWSDIR}/test/ws-testStableMediumSizeData.cpp
        )
add_executable(ws-testStableMediumSizeData ${WS_TEST_STABLE_MEDIUM_SIZE_DATA_SOURCES})
target_link_libraries(ws-testStableMediumSizeData uvws ${SIMPLELOG_LIB})


#install directory
# cmake -DCMAKE_INSTALL_PREFIX=/usr
install(TARGETS uvws
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )


install(
        DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h*")

