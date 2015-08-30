#!/bin/bash

#build_tool=cmake
build_tool=scan_build

analysis_type=all
#analysis_type=callers
#analysis_type=frama_c
#analysis_type=frama_clang
#analysis_type=framaCIRGen

common=`which common.sh`
source $common

source test_clean.sh

mkdir analysis
cd analysis
export CALLERS_ANALYSIS_TYPE="$analysis_type"
if [ $build_tool == "cmake" ]
then
    cmake ..
    # make VERBOSE=yes
    cmake_analysis.sh compile_commands.json ${analysis_type} all
elif [ $build_tool == "scan_build" ]
then
    scan_build_checkers_options=""
    #scan_build_checkers_options="-v -v -V -enable-checker debug.DumpCallGraph -enable-checker debug.DumpCalls -enable-checker debug.ViewCallGraph"
    scan-build -o ${analysis_type} cmake ..
    scan-build ${scan_build_checkers_options} -o ${analysis_type} make VERBOSE=yes
fi
if [ $? -ne 0 ]; then
    echo "################################################################################"
    echo "# ${analysis_type} analysis error. Stop here !"
    echo "################################################################################"
    exit -1
fi
cd ..

if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ]; 
then

    # List generated json files
    list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json

    # List all defined symbols in file defined_symbols.json
    list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json
    #read_defined_symbols defined_symbols.json file.callers.gen.json

    # add extcallees to json files
    source add_extcallees.sh `pwd` defined_symbols.json

    # add extcallers to json files
    source add_extcallers.sh .
    source indent_jsonfiles.sh .

    ## generate callee's tree from main entry point
    source function_callers_to_dot.sh callees "main" "int main()" `pwd`/test_dummy.c

    ## generate caller's tree from main entry point
    source function_callers_to_dot.sh callers "main" "int main()" `pwd`/test_dummy.c

    source process_dot_files.sh . analysis/${analysis_type}

    inkscape analysis/${analysis_type}/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/main.fct.callers.gen.dot.svg
fi