
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,-Map=mytest.map")

add_executable(simple_c
  simple.c
  a.c)

add_executable(simple_cpp
  simple.cpp
  a.c)
