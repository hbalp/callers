
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(LINKER_FLAG "-Map mytest.map")

set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,-Map=mytest.map")

include_directories("." "dirA" "dirB")

add_executable(mytest
  dirA/A.cpp
  dirB/B.cpp
  test.cpp
  )
