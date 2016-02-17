#!/bin/bash
set -x
# from http://clang.llvm.org/get_started.html
# Checkout LLVM:

#     Change directory to where you want the llvm directory placed.
svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm

# Checkout Clang:

cd llvm/tools
svn co http://llvm.org/svn/llvm-project/cfe/trunk clang
cd ../..

# Checkout extra Clang Tools: (optional)

cd llvm/tools/clang/tools
svn co http://llvm.org/svn/llvm-project/clang-tools-extra/trunk extra
cd ../../../..

# Checkout Compiler-RT:

cd llvm/projects
svn co http://llvm.org/svn/llvm-project/compiler-rt/trunk compiler-rt
cd ../..

# Checkout libcxx: (only required to build and run Compiler-RT tests on OS X, optional otherwise)

#     cd llvm/projects
#     svn co http://llvm.org/svn/llvm-project/libcxx/trunk libcxx
#     cd ../..

# Build LLVM and Clang:

#     mkdir build (in-tree build is not supported)
mkdir build
cd build
cmake -G "Unix Makefiles" ../llvm
make
