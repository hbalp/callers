##===- tools/frama-c/Makefile ------------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

CLANG_LEVEL := ../..

TOOLNAME = callers

# No plugins, optimize startup time.
TOOL_NO_EXPORTS = 1

include $(CLANG_LEVEL)/../../Makefile.config
LINK_COMPONENTS := $(TARGETS_TO_BUILD)
USEDLIBS = clangFrontend.a clangDriver.a clangTooling.a clangParse.a \
  clangSema.a clangAnalysis.a clangEdit.a clangAST.a clangLex.a      \
  clangSerialization.a clangBasic.a LLVMBitReader.a LLVMMCParser.a   \
  LLVMOption.a

include $(CLANG_LEVEL)/Makefile
