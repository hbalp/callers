set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

#option(USE_CXX_EXCEPTIONS "Enable C++ exception support" ON)

#set(CMAKE_CXX_FLAGS "-fexceptions")

include_directories(..)

add_clang_executable(callers
  callers.cpp
  ../visitor.cpp
  ../CallersData.cpp
  ../gdb_backtrace.cpp
  #/home/hbalp/work/src/llvm/trunk/tools/clang/tools/extra/callers/visitor.cpp
  #/home/hbalp/work/src/llvm/trunk/tools/clang/tools/extra/callers/CallersData.cpp
  )

target_compile_options(callers PRIVATE -fexceptions)

#set_target_properties(callers PROPERTIES COMPILE_FLAGS "-fexceptions")
#set_target_properties(callers PROPERTIES CXX_FLAGS "-fexceptions")

target_link_libraries(callers
  PRIVATE
  clangAST
  clangBasic
  clangDriver
  clangFrontend
  clangRewriteFrontend
  clangStaticAnalyzerFrontend
  clangTooling
  boost_filesystem
  boost_regex
  boost_system
  )

install(TARGETS callers
  RUNTIME DESTINATION bin)
