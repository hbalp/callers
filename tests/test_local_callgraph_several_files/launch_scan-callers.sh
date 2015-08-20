#!/bin/bash
#set -x

# clean test
source clean_test.sh

# build the application and get all defined symbols
mkdir build
cd build
cmake ..
make
cd ..

# generation of file defined_symbols.json
list_defined_symbols.sh `pwd`
ln -s `pwd`/defined_symbols.json /tmp/defined_symbols.json
rm -rf build

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

# add extcallers to json files
add_extcallers.sh .
indent_jsonfiles.sh .

# generate callee's tree from main entry point
function_callers_to_dot.native callees "main" "int main()" `pwd`/test.cpp
process_dot_files.sh . analysis/callers
inkscape analysis/callers/svg/main.fct.callees.gen.dot.svg
