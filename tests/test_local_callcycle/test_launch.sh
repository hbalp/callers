#!/bin/bash

./test_clean.sh
mkdir build
cd build
cmake ..
make
cd ..
list_defined_symbols.sh `pwd`
cd build
cmake_callers_analysis.sh compile_commands.json all reports
cd ..
indent_jsonfiles.sh .
function_callers_to_dot.native callees "main" "int main()" `pwd`/test_local_callcycle.c
dot -Tsvg main.fct.callees.gen.dot > main.fct.callees.gen.dot.svg
inkscape main.fct.callees.gen.dot.svg


