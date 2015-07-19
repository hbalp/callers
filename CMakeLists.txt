set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

add_clang_executable(callers
  callers.cpp
  visitor.cpp
  )

target_link_libraries(callers
  clangAST
  clangBasic
  clangDriver
  clangFrontend
  clangRewriteFrontend
  clangStaticAnalyzerFrontend
  clangTooling
  )

install(TARGETS callers
  RUNTIME DESTINATION bin)
