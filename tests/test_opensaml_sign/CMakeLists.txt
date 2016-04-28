
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
#set(LINKER_FLAG "-Map mytest.map")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,-Map=mytest.map")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-L /opt/stance/lib")

include_directories("/opt/stance/include/saml")

add_executable(testSign
    SAML2AssertionTest.stance.cpp
  )

target_link_libraries(testSign saml xmltooling xerces-c)

