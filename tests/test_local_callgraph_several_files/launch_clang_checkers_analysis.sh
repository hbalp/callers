#!/bin/bash

export CC=`which clang`
export CXX=`which clang++`

#cmake -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_BUILD_TYPE=Debug .

#scan-build -v --use-analyzer=`which clang++` cmake -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_BUILD_TYPE=Debug .
scan-build -v -V --use-analyzer=`which clang++` --use-cc=`which clang`  --use-c++=`which clang++` cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .

scan-build -v -V -stats \
    --use-cc=`which clang`  --use-c++=`which clang++` \
    -analyze-headers \
    -enable-checker debug.DumpCallGraph \
    -enable-checker debug.ViewCallGraph \
    --use-analyzer=`which clang++` \
    -o scan-build-report make

# DotCallGraph
#scan-build -v --use-cc=`which clang`  --use-c++=`which clang++` --use-analyzer=`which clang++` -enable-checker debug.DotCallGraphGenerator -o scan-build-report make
