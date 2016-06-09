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
bin_dir=`dirname $common`
launch_scan_build=`which ${bin_dir}/launch_analysis.sh`

source $common
source $launch_scan_build

# clean test
source test_clean.sh

# launch the analysis
launch_the_analysis ${build_tool} ${analysis_type}

if [ $build_tool != "scan-build" ]
#if false
then
if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ]
then

    # List generated json files
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json callers_analysis
    extract_metrics metrics.callers.gen.json
    
    # List all defined symbols in file defined_symbols.json
    #list_defined_symbols defined_symbols.gen.json `pwd` dir.callers.gen.json
    #read_defined_symbols.native defined_symbols.json file.callers.gen.json

    # # add extcallees to json files
    # source add_extcallees.sh `pwd`

    # # add extcallers to json files
    # source add_extcallers.sh .
    # source indent_jsonfiles.sh .

    # generate callee's tree from main entry point
    #function_calls_to_dot.native callees `pwd`/test.cpp "main" "int main()" files
    source extract_fcg.sh callees `pwd`/test.cpp "main" "int main()" files

    # generate caller's tree from main entry point
    #source extract_fcg.sh callers `pwd`/test.cpp "main" "int main()" files

    # generate caller's tree from B::B() constructor
    source extract_fcg.sh callers `pwd`/B.cpp "B" "void B::B()" files

    # generate caller's tree from printf builtin function
    source extract_fcg.sh callers /usr/include/stdio.h "printf" "printf" files

    # generate a call graph from "int A::a()" to "int c()"
    source extract_fcg.sh c2c `pwd`/A.cpp "A_a" "int A::a()" `pwd`/B.hpp "c" "int c()"
    source callgraph_to_ecore.sh $callers_json_rootdir
    source callgraph_to_dot.sh $callers_json_rootdir files

    #source process_dot_files.sh . analysis/${analysis_type}

    #inkscape analysis/${analysis_type}/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/main.fct.callers.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/A_a.c.c2c.gen.dot.svg
fi
fi
