set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(LINKER_FLAG "-Map mytest.map")
#set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,-Map=mytest.map")

include_directories("." "dirA" "dirB" "dirC")

add_executable(mytest
  dirA/A.cpp
  dirB/B.cpp
  dirC/C.cpp
  dirC/D.cpp
  test.cpp
  )

target_link_libraries(mytest pthread)
