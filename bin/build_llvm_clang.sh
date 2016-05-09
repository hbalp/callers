#!/bin/bash
set -x
# from http://clang.llvm.org/get_started.html
# Checkout LLVM:

src_dir=/home/hbalp/hugues/work/third_parties/src

llvm_src_dir=${src_dir}/llvm

function llvm_install ()
{
    cd ${llvm_src_dir}

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
}

# Build LLVM and Clang:
function llvm_build ()
{
    cd ${src_dir}

    #     mkdir build (in-tree build is not supported)
    mkdir -p build
    cd build
    cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/tools/exec -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release ../llvm
    make CXXFLAGS="-DKEY_WOW64_32KEY=0x0200 -D_GLIBCXX_HAVE_FENV_H"
    sudo make install
}
