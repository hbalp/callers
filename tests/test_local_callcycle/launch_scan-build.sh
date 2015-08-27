#!/bin/bash
#set -x

# clean test
source test_clean.sh

# launch callers analysis
mkdir analysis
cd analysis
scan-build -o callers cmake ..
scan-build -o callers make
if [ $? -ne 0 ]; then
    echo "################################################################################"
    echo "# Scan-build analysis launch error. Stop here !"
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
source indent_jsonfiles.sh .

# generate callee's tree from main entry point
#function_callers_to_dot.native callees "main" "int main()" `pwd`/test_local_callcycle.c
function_callers_to_dot.native callees "main" "int main()" `pwd`/test_local_callcycle.c files

# generate caller's tree from main entry point
#function_callers_to_dot.native callers "main" "int main()" `pwd`/test_local_callcycle.c
function_callers_to_dot.native callers "a" "void a()" `pwd`/test_local_callcycle.c

source process_dot_files.sh . analysis/callers
inkscape analysis/callers/svg/main.fct.callees.gen.dot.svg
#inkscape analysis/callers/svg/main.fct.callers.gen.dot.svg
#inkscape analysis/callers/svg/a.fct.callers.gen.dot.svg
