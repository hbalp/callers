#!/bin/bash
#set -x

#build_tool=cmake
build_tool="scan-callers"
#build_tool="scan-build"

#analysis_type=all
analysis_type=callers
#analysis_type=frama-c
#analysis_type=frama-clang
#analysis_type=framaCIRGen

common=`which common.sh`
launch_scan_build=`which launch_analysis.sh`

source $common
source $launch_scan_build

source test_clean.sh

# analysis the application
launch_the_analysis ${build_tool} ${analysis_type}

if [ $build_tool != "scan-build" ]
then
if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ]
then

    # List generated json files
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs ${callers_json_rootdir} .file.callers.gen.json dir.callers.gen.json callers_analysis
    extract_metrics metrics.callers.gen.json
    
    # List all defined symbols in file defined_symbols.json
    #list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json
    #read_defined_symbols.native defined_symbols.json file.callers.gen.json

    # # add declarations to json files
    # source add_declarations.sh `pwd` $includes_directories

    # # add definitions to json files
    # source add_definitions.sh `pwd` $includes_directories

    # # add extcallees to json files
    # source add_extcallees.sh `pwd`

    # # add extcallers to json files
    # source add_extcallers.sh .

    # # add inherited to json files
    # source add_inherited.sh .

    # # add virtual function calls to json files
    # source add_virtual_function_calls.sh `pwd`

    # source indent_jsonfiles.sh .

    # generate callee's tree from main entry point
    source extract_fcg.sh callees `pwd`/test.cpp "main" "int main()" files

    # generate caller's tree from main entry point
    #source extract_fcg.sh callers `pwd`/test.cpp "main" "int main()" files

    # generate a call graph from "int A::a()" to "int c()"
    source extract_fcg.sh c2c `pwd`/A.cpp "A_a" "int A::a()" `pwd`/B.cpp "c" "int c()"

    # generate callgraphs
    source callgraph_to_ecore.sh $callers_json_rootdir
    source callgraph_to_dot.sh $callers_json_rootdir files
    
    # source process_dot_files.sh . analysis/${analysis_type}

    # inkscape analysis/${analysis_type}/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/main.fct.callers.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/A_a.c.c2c.gen.dot.svg
fi
else
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    source indent_jsonfiles.sh .
fi
