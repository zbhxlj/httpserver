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

add_library(baselib STATIC ${base})

set(commonlib
    baselib
    gtest 
    yaml-cpp
    pthread
)

add_executable(base_test test/base_test.cpp)
target_link_libraries(base_test ${commonlib})
