#!/bin/bash

export CC=`which clang`
export CXX=`which clang++`

cmake -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_BUILD_TYPE=Debug .

#scan-build -v -analyze-headers --use-cc=`which clang`  --use-c++=`which clang++` --use-analyzer=`which callers` -stats -o scan-build-report make

# DumpCallGraph
#scan-build -v -analyze-headers --use-cc=`which clang`  --use-c++=`which clang++` --use-analyzer=`which clang++` -enable-checker debug.DumpCallGraph -stats -o scan-build-report make
#scan-build -v -V -enable-checker debug.DumpCallGraph -analyze-headers --use-analyzer=`which clang++` -o scan-build-report make
scan-build -v -V -enable-checker debug.DumpCallGraph -analyze-headers --use-analyzer=`which clang` -o scan-build-report make
#scan-build -v -V -enable-checker debug.DumpCallGraph -analyze-headers --use-cc=`which clang`  --use-c++=`which clang++` --use-analyzer=`which clang++` -o scan-build-report make

# ViewCallGraph
#scan-build -v -analyze-headers --use-cc=`which clang`  --use-c++=`which clang++` --use-analyzer=`which clang++` -enable-checker debug.ViewCallGraph -stats -o scan-build-report make

