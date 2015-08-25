#!/bin/bash

./test_clean.sh
mkdir build
cd build
cmake ..
make
cd ..

# launch callers analysis
cd build
cmake_callers_analysis.sh compile_commands.json all reports
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
list_defined_symbols.native defined_symbols.json test_local_callcycle dir.callers.gen.json
read_defined_symbols.native defined_symbols.json file.callers.gen.json

# add extcallees to json files
source add_extcallees.sh `pwd` defined_symbols.json

# add extcallers to json files
source add_extcallers.sh .
indent_jsonfiles.sh .

## generate callee's tree from main entry point
function_callers_to_dot.native callees "main" "int main()" `pwd`/test_dummy.c

## generate caller's tree from main entry point
function_callers_to_dot.native callers "main" "int main()" `pwd`/test_dummy.c

process_dot_files.sh . .

inkscape svg/main.fct.callees.gen.dot.svg
#inkscape svg/main.fct.callers.gen.dot.svg


