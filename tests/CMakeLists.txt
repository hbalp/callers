cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(test_local_callgraph_several_files)

add_executable(simple simple.cpp)

# add_executable(test_boost_regex test_boost_regex.cpp)

# add_library (boost_regex SHARED IMPORTED)

# target_link_libraries(boost_regex)

add_executable(c_calls test_local_callgraph.c)

add_executable(cpp_calls test_local_callgraph.cpp)

# add_executable(std_regex test_std_regex.cpp)
