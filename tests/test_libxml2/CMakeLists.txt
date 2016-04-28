cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

include_directories("/data/balp/src/tools/exec/include/libxml2")

add_executable(test_libxml2 gjobread.c)

target_link_libraries(test_libxml2 /data/balp/src/tools/exec/lib/libxml2.so)
