#!/bin/bash

./test_clean.sh
mkdir build
cd build
cmake ..
make
cd ..
# generation of file build/defined_symbols.json
list_defined_symbols.sh `pwd`
# launch callers analysis
cd build
cmake_callers_analysis.sh compile_commands.json all reports
cd ..
# add extcallers to json files
add_extcallers.sh .
indent_jsonfiles.sh .

## generate callee's tree from main entry point
function_callers_to_dot.native callees "main" "int main()" `pwd`/test_local_callcycle.c

## generate caller's tree from main entry point
#function_callers_to_dot.native callers "main" "int main()" `pwd`/test_local_callcycle.c
function_callers_to_dot.native callers "a" "void a()" `pwd`/test_local_callcycle.c

process_dot_files.sh . .

inkscape svg/main.fct.callees.gen.dot.svg
#inkscape svg/main.fct.callers.gen.dot.svg


