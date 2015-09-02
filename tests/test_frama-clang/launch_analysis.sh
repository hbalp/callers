#!/bin/bash
#set -x

#build_tool=cmake
build_tool=scan_build

analysis_type=all
#analysis_type=callers
#analysis_type=frama-c
#analysis_type=frama-clang
#analysis_type=framaCIRGen

common=`which common.sh`
source $common

# clean test
source clean.sh

# analysis the application
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
    scan-build -o ${analysis_type} cmake ..
    scan-build -o ${analysis_type} make VERBOSE=yes
fi
if [ $? -ne 0 ]; then
    echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
    echo "ERROR ${analysis_type} analysis error. Stop here !"
    echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
    exit -1
fi
cd ..

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
    source function_callers_to_dot.sh callees "simple_c" "int main(int, ((char)*)*)" `pwd`/simple.c files
    #function_callers_to_dot.native callees "simple_cpp" "int main(int, ((char)*)*)" `pwd`/simple.cpp files

    source process_dot_files.sh . analysis/${analysis_type}

    inkscape analysis/${analysis_type}/svg/simple_c.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/simple_cpp.fct.callees.gen.dot.svg

fi
