set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

option(USE_CXX_EXCEPTIONS "Enable C++ exception support" ON)

add_subdirectory(c)
add_subdirectory(cpp)
