#!/bin/bash
#set -x

source test_clean.sh
mkdir build
cd build
cmake ..
make
cd ..
# generation of file build/defined_symbols.json
list_defined_symbols.sh `pwd`
# launch callers analysis
cd build
cmake_callers_analysis.sh compile_commands.json all callers-reports 2>&1 | tee analysis.log
cd ..
# add extcallers to json files
add_extcallers.sh .
indent_jsonfiles.sh .

## generate callee's tree from main entry point
#function_callers_to_dot.native callees "main" "int main()" `pwd`/test.cpp
function_callers_to_dot.native callees "main" "int main()" `pwd`/test.cpp files

## generate caller's tree from main entry point
#function_callers_to_dot.native callers "main" "int main()" `pwd`/test.cpp
function_callers_to_dot.native callers "main" "int main()" `pwd`/test.cpp files

## generate a call graph from "int A::a()" to "int c()"
function_callers_to_dot.native c2c "A_a" "int A::a()" `pwd`/A.cpp "c" "int c()" `pwd`/B.cpp

process_dot_files.sh . .

#inkscape main.fct.callers.gen.dot.svg
#inkscape main.fct.callers.gen.dot.svg
inkscape svg/main.fct.callees.gen.dot.svg
