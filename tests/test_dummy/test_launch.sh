#!/bin/bash

#build_tool="cmake"
build_tool="scan-callers"
#build_tool="scan-build"

analysis_type=all
#analysis_type=callers
#analysis_type=frama_c
#analysis_type=frama_clang
#analysis_type=framaCIRGen

#clang=`which clang`

common=`which common.sh`
launch_scan_build=`which launch_analysis.sh`

source $common
source $launch_scan_build

source test_clean.sh

launch_the_analysis ${build_tool} ${analysis_type}

if [ $build_tool != "scan-build" ]
#if false
then
if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ]; 
then

    # List generated json files
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json analysis

    # List all defined symbols in file defined_symbols.json
    list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json
    #read_defined_symbols defined_symbols.json file.callers.gen.json

    # # add extcallees to json files
    # source add_extcallees.sh `pwd` defined_symbols.json

    # # add extcallers to json files
    # source add_extcallers.sh .
    source indent_jsonfiles.sh .

    # ## generate callee's tree from main entry point
    # source function_calls_to_dot.sh callees `pwd`/test_dummy.c "main" "int main()" files

    # ## generate caller's tree from main entry point
    # source function_calls_to_dot.sh callers `pwd`/test_dummy.c "main" "int main()" files

    # source process_dot_files.sh . analysis/${analysis_type}

    # inkscape analysis/${analysis_type}/svg/main.fct.callees.gen.dot.svg
    # #inkscape analysis/${analysis_type}/svg/main.fct.callers.gen.dot.svg
fi
fi
