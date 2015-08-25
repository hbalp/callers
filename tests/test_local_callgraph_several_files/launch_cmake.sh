#!/bin/bash
#set -x

# clean test
source clean.sh

# build the application and get all defined symbols
mkdir analysis
cd analysis
cmake ..
#make
cd ..

# launch callers analysis
cd analysis
cmake_callers_analysis.sh compile_commands.json all callers
if [ $? -ne 0 ]; then
    echo "################################################################################"
    echo "# Callers analysis error. Stop here !"
    echo "################################################################################"
    exit -1
fi
cd ..

# List generated json files
list_json_files_in_dirs.native `pwd` .json dir.callers.gen.json

# List all defined symbols in file defined_symbols.json
list_defined_symbols.native defined_symbols.json test_local_callgraph_several_files dir.callers.gen.json
read_defined_symbols.native defined_symbols.json file.callers.gen.json

# add extcallees to json files
source add_extcallees.sh `pwd` defined_symbols.json

# add extcallers to json files
source add_extcallers.sh .
indent_jsonfiles.sh .

# generate callee's tree from main entry point
#function_callers_to_dot.native callees "main" "int main()" `pwd`/test.cpp
function_callers_to_dot.native callees "main" "int main()" `pwd`/test.cpp files
process_dot_files.sh . analysis/callers
inkscape analysis/callers/svg/main.fct.callees.gen.dot.svg
