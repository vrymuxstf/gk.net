cmake_minimum_required(VERSION 3.28)
project(gk.net)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE include/*.h)
add_executable(test ${SOURCE} udp_test.cpp)
target_include_directories(test PRIVATE include)
