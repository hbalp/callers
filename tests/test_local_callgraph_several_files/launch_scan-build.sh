#!/bin/bash
#set -x

source "../../bin/common.sh"

#analysis_type=$1
#analysis_type=all
analysis_type=callers
#analysis_type=frama-clang
#analysis_type=framaCIRGen

# clean test
source clean.sh

# launch callers analysis
mkdir analysis
cd analysis
# cmake ..
# #make
# cmake_callers_analysis.sh compile_commands.json all callers
export CALLERS_ANALYSIS_TYPE="$analysis_type"
scan-build -o callers cmake ..
scan-build -o callers make VERBOSE=yes
if [ $? -ne 0 ]; then
    echo "################################################################################"
    echo "# ${analysis_type} analysis error. Stop here !. Analysis type is ${CALLERS_ANALYSIS_TYPE}"
    echo "################################################################################"
    exit -1
fi
cd ..

if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ];
then
    # List generated json files
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json

    # List all defined symbols in file defined_symbols.json
    source list_defined_symbols.sh defined_symbols.json `pwd` dir.callers.gen.json
    # read_defined_symbols.native defined_symbols.json file.callers.gen.json

    # add extcallees to json files
    source add_extcallees.sh `pwd` defined_symbols.json

    # add extcallers to json files
    source add_extcallers.sh .
    source indent_jsonfiles.sh .

    # generate callee's tree from main entry point
    source function_callers_to_dot.sh callees "main" "int main()" `pwd`/test.cpp
    source function_callers_to_dot.sh callees "main" "int main()" `pwd`/test.cpp files
    source process_dot_files.sh . analysis/callers
    inkscape analysis/callers/svg/main.fct.callees.gen.dot.svg
fi
