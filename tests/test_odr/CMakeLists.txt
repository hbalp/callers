
# add debug symbols
set(CMAKE_BUILD_TYPE Debug)

#include_directories(.)

add_executable(test_odr application.cpp library.cpp)