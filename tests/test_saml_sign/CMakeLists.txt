cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

include_directories("/data/balp/src/tools/exec/include/libxml2")

add_executable(test_saml_sign test_saml_sign.c)
# add_executable(test_saml_sign test_saml_sign.cpp)

target_link_libraries(test_saml_sign /data/balp/src/tools/exec/lib/libxml2.so)

