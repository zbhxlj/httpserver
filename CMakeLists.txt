cmake_minimum_required(VERSION 3.20)

project(web_server)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

include_directories(include)

# add_executable(web_server)

add_executable(test_config test/test.cpp test/test_config.cpp src/config_parser.cpp)
target_link_libraries(test_config gtest yaml-cpp)