#!/bin/bash
#set -x

# clean test
source clean.sh

# launch callers analysis
mkdir analysis
cd analysis
scan-callers -o callers cmake ..
scan-callers -o callers make
if [ $? -ne 0 ]; then
    echo "################################################################################"
    echo "# Scan-Callers analysis launch error. Stop here !"
    echo "################################################################################"
    exit -1
fi
#cmake_callers_analysis.sh compile_commands.json all callers-reports 2>&1 | tee analysis.log
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
source indent_jsonfiles.sh .

# generate callee's tree from main entry point
#function_callers_to_dot.native callees "main" "int main()" `pwd`/test.cpp
function_callers_to_dot.native callees "main" "int main()" `pwd`/test.cpp files
source process_dot_files.sh . analysis/callers
inkscape analysis/callers/svg/main.fct.callees.gen.dot.svg
