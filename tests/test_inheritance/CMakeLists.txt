# cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories("dir1" "dir2" "dir3")

add_executable(test_dummy test_dummy.cpp)

