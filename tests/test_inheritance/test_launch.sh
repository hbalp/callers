#!/bin/bash

build_tool="cmake"
#build_tool="scan-callers"
#build_tool="scan-build"

#analysis_type=all
analysis_type=callers
#analysis_type=frama_c
#analysis_type=frama_clang
#analysis_type=framaCIRGen

#clang=`which clang`

common=`which common.sh`
bin_dir=`dirname $common`
test_dir=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)
launch_scan_build=`which ${bin_dir}/launch_analysis.sh`

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

    # add declarations to json files
    source add_declarations.sh `pwd` $includes_directories

    # add definitions to json files
    source add_definitions.sh `pwd` $includes_directories

    # add extcallees to json files
    source add_extcallees.sh `pwd` defined_symbols.json

    # add extcallers to json files
    source add_extcallers.sh .

    # add inherited to json files
    source add_inherited.sh .

    # add virtual function calls to json files
    source add_virtual_function_calls.sh `pwd`

    source indent_jsonfiles.sh .

    ## generate callee's tree from main entry point
    source function_calls_to_dot.sh callees `pwd`/test_dummy.cpp "main" "int main()" files

    ## generate caller's tree from main entry point
    source function_calls_to_dot.sh callers `pwd`/test_dummy.cpp "main" "int main()" files

    # add inherited classes to json files
    source add_inherited.sh .
    source indent_jsonfiles.sh .

    # generate class inheritance tree from A base class
    source classes_to_dot.sh child `pwd`/test_dummy.cpp "A"
    source classes_to_dot.sh base `pwd`/test_dummy.cpp "B"
    
    source process_dot_files.sh . analysis/${analysis_type}

    inkscape analysis/${analysis_type}/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/main.fct.callers.gen.dot.svg
fi
else
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    source indent_jsonfiles.sh .
fi
