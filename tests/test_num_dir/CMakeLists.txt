
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(LINKER_FLAG "-Map mytest.map")

set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,-Map=mytest.map")

#include_directories("." "dirA" "dirB")
include_directories("." "dirA" "4.8")

add_executable(mytest
  dirA/A.cpp
#  dirB/B.cpp
  4.8/B.cpp
  test.cpp
  )
