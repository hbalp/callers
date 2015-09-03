#!/bin/bash
#set -x

source "../../bin/common.sh"

#analysis_type=$1
analysis_type=all
#analysis_type=callers
#analysis_type=framaCIRGen
#analysis_type=frama-clang

# clean test
source test_clean.sh

# launch callers analysis
mkdir analysis
cd analysis
cmake ..
cmake_analysis.sh compile_commands.json $analysis_type all
if [ $? -ne 0 ]; then
    echo "################################################################################"
    echo "# Callers analysis launch error. Stop here !"
    echo "################################################################################"
    exit -1
fi
cd ..

if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ]; 
then

    # List generated json files
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json analysis

    # List all defined symbols in file defined_symbols.json
    list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json
    #read_defined_symbols.native defined_symbols.json file.callers.gen.json

    # add extcallees to json files
    source add_extcallees.sh `pwd` defined_symbols.json

    # add extcallers to json files
    source add_extcallers.sh .
    source indent_jsonfiles.sh .

    # generate callee's tree from main entry point
    #function_callers_to_dot.native callees "main" "int main()" `pwd`/test_local_callcycle.c
    source function_callers_to_dot.sh `pwd`/test_local_callcycle.c callees "main" "int main()"

    # generate caller's tree from main entry point
    #source function_callers_to_dot.sh callers "main" "int main()" `pwd`/test_local_callcycle.c
    source function_callers_to_dot.sh `pwd`/test_local_callcycle.c callers "a" "void a()"

    source process_dot_files.sh . analysis/callers

    inkscape analysis/callers/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/callers/svg/main.fct.callers.gen.dot.svg
    #inkscape analysis/callers/svg/a.fct.callers.gen.dot.svg

fi
