#!/bin/bash
#set -x

#build_tool=cmake
build_tool="scan-callers"
#build_tool="scan-build"

analysis_type=all
#analysis_type=callers
#analysis_type=frama-c
#analysis_type=frama-clang
#analysis_type=framaCIRGen

common=`which common.sh`
bin_dir=`dirname $common`
launch_scan_build=`which ${bin_dir}/launch_analysis.sh`

source $common
source $launch_scan_build

# clean test
source clean.sh

# launch the analysis
launch_the_analysis ${build_tool} ${analysis_type}

if [ $build_tool != "scan-build" ]
#if false
then
if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ]
then

    # List generated json files
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json analysis

    # List all defined symbols in file defined_symbols.json
    list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json
    #read_defined_symbols.native defined_symbols.json file.callers.gen.json

    # add extcallees to json files
    source add_extcallees.sh `pwd` defined_symbols.json

    # # add extcallers to json files
    source add_extcallers.sh .
    source indent_jsonfiles.sh .

    # generate callee's tree from main entry points
    source function_calls_to_dot.sh callees `pwd`/simple.c "simple_c" "int main(int, ((char)*)*)" files
    #function_calls_to_dot.native callees `pwd`/simple.cpp "simple_cpp" "int main(int, ((char)*)*)" files

    source process_dot_files.sh . analysis/${analysis_type}

    inkscape analysis/${analysis_type}/svg/simple_c.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/simple_cpp.fct.callees.gen.dot.svg
fi
fi
