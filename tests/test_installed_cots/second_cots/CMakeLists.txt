cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_INSTALL_PREFIX ../../install_dir)

include_directories("../install_dir/include")

add_executable(second_cots second_cots.c)

find_library(FIRST_COTS_LOCATION first_cots ../install_dir/lib)

target_link_libraries(second_cots ${FIRST_COTS_LOCATION})

install(TARGETS second_cots DESTINATION bin)
