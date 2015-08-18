#!/bin/bash
#set -x

clean_test.sh
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
# generate callee's tree from main entry point
function_callers_to_dot.native callees "main" "int main()" `pwd`/test.cpp
dot -Tsvg main.fct.callees.gen.dot > main.fct.callees.gen.dot.svg
inkscape main.fct.callees.gen.dot.svg



