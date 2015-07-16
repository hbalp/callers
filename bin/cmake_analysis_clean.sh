#!/bin/bash

make clean
rm -rf CMakeFiles
rm -f compile_commands.json
rm -f Makefile
rm -f cmake_install.cmake
rm -f CMakeCache.txt
rm -f *.gen.*
rm -rf scan-build-report
rm -rf callers-report

