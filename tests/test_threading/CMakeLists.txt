set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(LINKER_FLAG "-Map test_threading.map")
#set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,-Map=test_threading.map")

include_directories("." "/usr/include")

add_executable(test_threading
  test_threading.c
  )

target_link_libraries(test_threading pthread)
