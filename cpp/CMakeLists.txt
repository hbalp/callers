set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

#option(USE_CXX_EXCEPTIONS "Enable C++ exception support" ON)

#set (CMAKE_CXX_FLAGS "-fexceptions")

include_directories(..)

add_clang_executable(callers++
  callers++.cpp
  ../visitor.cpp
  ../CallersData.cpp
  #/home/hbalp/work/src/llvm/trunk/tools/clang/tools/extra/callers/visitor.cpp
  #/home/hbalp/work/src/llvm/trunk/tools/clang/tools/extra/callers/CallersData.cpp
  )

target_compile_options(callers++ PRIVATE -fexceptions)

target_link_libraries(callers++
  clangAST
  clangBasic
  clangDriver
  clangFrontend
  clangRewriteFrontend
  clangStaticAnalyzerFrontend
  clangTooling
  boost_filesystem
  boost_system
  )

install(TARGETS callers++
  RUNTIME DESTINATION bin)
