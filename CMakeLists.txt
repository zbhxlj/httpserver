cmake_minimum_required(VERSION 3.20)

project(web_server)

find_package(yaml-cpp REQUIRED)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror")

# add_executable(web_server)

set(base
    src/config.cpp
    src/count_down_latch.cpp
    src/thread_pool.cpp
    src/thread.cpp
    src/socket.cpp
)

set(event_loop
    src/thread.cpp
    src/count_down_latch.cpp
    src/epoll.cpp
    src/channel.cpp
    src/event_loop.cpp
    src/event_loop_thread.cpp
    src/event_loop_thread_pool.cpp
)

set(http
    src/timer.cpp
    src/http_server.cpp
    src/http_manager.cpp
    src/http_handler.cpp
    src/http_connection.cpp
    src/socket.cpp
    src/config.cpp
    )

add_library(baselib STATIC ${base})
add_library(event_loop_lib STATIC ${event_loop})
add_library(http_lib STATIC ${http})

set(commonlib
    baselib
    event_loop_lib
    http_lib
    event_loop_lib
    gtest 
    yaml-cpp
    pthread
)

add_executable(base_test test/base_test.cpp)
target_link_libraries(base_test ${commonlib})

add_executable(epoll_test test/epoll_test.cpp)
target_link_libraries(epoll_test ${commonlib})

add_executable(http_test test/http_test.cpp)
target_link_libraries(http_test ${commonlib})

add_executable(web_server web_server.cpp)
target_link_libraries(web_server ${commonlib})