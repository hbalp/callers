
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(LINKER_FLAG "-Map mytest.map")

set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,-Map=mytest.map")

add_executable(mytest
  A.cpp
  B.cpp
  test.cpp
  )
