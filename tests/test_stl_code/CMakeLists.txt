
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(test_std_set
  test_std_set.cpp)

add_executable(test_std_map
  test_std_map.cpp)

add_executable(test_std_map1
  test_std_map1.cpp)

add_executable(test_std_map2
  test_std_map2.cpp)

add_executable(test_std_map3
  test_std_map3.cpp)

set_target_properties(test_std_map2 test_std_map3
  PROPERTIES
  COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11"
)