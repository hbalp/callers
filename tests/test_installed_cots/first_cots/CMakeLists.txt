cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_INSTALL_PREFIX ../../install_dir)

add_library(first_cots first_cots.c)

install(TARGETS first_cots DESTINATION lib)
install(FILES test_dummy.h DESTINATION include)

