cmake_minimum_required(VERSION 3.20)

project(web_server)

find_package(yaml-cpp REQUIRED)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror")

# add_executable(web_server)

add_executable(config_parser_test test/test.cpp test/config_parser_test.cpp src/config_parser.cpp)
target_link_libraries(config_parser_test gtest yaml-cpp)

add_executable(thread_test test/test.cpp test/thread_test.cpp src/thread.cpp src/count_down_latch.cpp)
target_link_libraries(thread_test gtest pthread)