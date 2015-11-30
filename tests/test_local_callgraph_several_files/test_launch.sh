#!/bin/bash
#set -x

#canonical_pwd="/media/sf_users_3u$PWD"
canonical_pwd="$PWD"

build_tool="cmake"
#build_tool="scan-callers"
#build_tool="scan-build"

#analysis_type=$1
#analysis_type=all
analysis_type=callers
#analysis_type=frama-clang
#analysis_type=framaCIRGen

common=`which common.sh`
bin_dir=`dirname $common`
test_dir=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)
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
if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ];
then

    # list the symbols referenced by the program and defined in the standard C++ library

    #includes_directories="/usr/include/c++/4.7"
    includes_directories="/usr/include/c++/4.8"

    for inc_dir in $includes_directories
    do
	cd $inc_dir
	list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json analysis

        # List all declared and defined symbols in files defined_symbols.json of each directories
	list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json

	source indent_jsonfiles.sh .
    done

    cd $test_dir

    # List generated json files
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json "analysis"

    # List all defined symbols in file defined_symbols.all.gen.json
    list_defined_symbols defined_symbols.all.gen.json `pwd` dir.callers.gen.json
    # read_defined_symbols.native defined_symbols.all.gen.json file.callers.gen.json

    # add declarations to json files
    source add_declarations.sh `pwd` $includes_directories

    # add definitions to json files
    source add_definitions.sh `pwd` $includes_directories

    # add extcallees to json files
    source add_extcallees.sh `pwd` $includes_directories
    #source add_extcallees.sh `pwd` broken_symbols.json

    # add extcallers to json files
    source add_extcallers.sh .

    # add inherited to json files
    source add_inherited.sh .

    # add virtual function calls to json files
    source add_virtual_function_calls.sh `pwd`

    source indent_jsonfiles.sh .

    # generate callee's tree from main entry point
    source function_callgraph.sh callees $canonical_pwd/test.cpp "main" "int main()" files

    # generate caller's tree from main entry point
    source function_callgraph.sh callers $canonical_pwd/dirB/B.cpp "c" "int c()" files
    source function_callgraph.sh callers /usr/include/stdio.h "printf" "printf" files

    # generate classes tree from base class A
    source classes_depgraph.sh child $canonical_pwd/dirA/A.hpp "A"
    source classes_depgraph.sh base $canonical_pwd/dirC/D.hpp ":0:Newly:1:Added:3:D"

    source process_dot_files.sh . analysis/${analysis_type}

    inkscape analysis/${analysis_type}/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/c.fct.callers.gen.dot.svg
fi
else
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    source indent_jsonfiles.sh .
fi
