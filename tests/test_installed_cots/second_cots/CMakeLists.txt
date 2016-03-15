cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

include_directories("../install_dir/include")

add_executable(second_cots second_cots.c)

find_library(FIRST_COTS_LOCATION first_cots /home/balp/third_parties/llvm/tools/clang/tools/extra/callers/tests/test_installed_cots/first_cots/build)

target_link_libraries(second_cots ${FIRST_COTS_LOCATION})
