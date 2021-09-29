cmake_minimum_required(VERSION 3.20)

project(web_server)

find_package(yaml-cpp REQUIRED)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror")

# add_executable(web_server)

set(base
    src/config_parser.cpp
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

add_library(baselib STATIC ${base})
add_library(event_loop_lib STATIC ${event_loop})

set(commonlib
    baselib
    event_loop_lib
    gtest 
    yaml-cpp
    pthread
)

add_executable(base_test test/base_test.cpp)
target_link_libraries(base_test ${commonlib})

add_executable(epoll_test test/epoll_test.cpp)
target_link_libraries(epoll_test ${commonlib})
