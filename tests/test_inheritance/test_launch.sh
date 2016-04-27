#!/bin/bash

build_tool="cmake"
#build_tool="scan-callers"
#build_tool="scan-build"

#analysis_type=all
analysis_type=callers
#analysis_type=frama-c
#analysis_type=frama-clang
#analysis_type=framaCIRGen

#clang=`which clang`

common=`which common.sh`
bin_dir=`dirname $common`
test_dir=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)
launch_scan_build=`which ${bin_dir}/launch_analysis.sh`

source test_clean.sh

source $common
source $launch_scan_build

launch_the_analysis ${build_tool} ${analysis_type}

if [ $build_tool != "scan-build" ]
#if false
then
if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ];
then

    # List generated json files
    find $callers_json_rootdir -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs $callers_json_rootdir .file.callers.gen.json dir.callers.gen.json analysis

    # List all defined symbols in file defined_symbols.gen.json
    list_defined_symbols defined_symbols.gen.json
    #read_defined_symbols defined_symbols.gen.json file.callers.gen.json

    # add declarations to json files
    source add_declarations.sh $callers_json_rootdir net

    # add definitions to json files
    source add_definitions.sh $callers_json_rootdir net

    # add extcallees to json files
    source add_extcallees.sh $callers_json_rootdir defined_symbols.gen.json

    # add extcallers to json files
    source add_extcallers.sh $callers_json_rootdir

    # add inherited classes to json files
    source add_inherited.sh $callers_json_rootdir

    # add virtual function calls to json files
    source add_virtual_function_calls.sh `pwd`

    ## generate callee's tree from main entry point
    source extract_fcg.sh callees `pwd`/test_inheritance.cpp "main" "int main()" files

    ## generate caller's tree from main entry point
    #source extract_fcg.sh callers `pwd`/test_inheritance.cpp "main" "int main()" files

    source callgraph_to_ecore.sh $callers_json_rootdir
    source callgraph_to_dot.sh $callers_json_rootdir files

    source indent_jsonfiles.sh .
    source indent_jsonfiles.sh $callers_json_rootdir

    # generate class inheritance tree from A base class
    # source classes_depgraph.sh child `pwd`/test_dummy.cpp "A"
    # source classes_depgraph.sh base `pwd`/test_dummy.cpp "B"
    # source classes_depgraph.sh base `pwd`/test_inheritance.cpp "B"
    source classes_depgraph.sh child `pwd`/test_inheritance.h A

    source process_dot_files.sh . analysis/${analysis_type}

    source indent_jsonfiles.sh .
    source indent_jsonfiles.sh $callers_json_rootdir

    inkscape analysis/${analysis_type}/svg/main.fcg.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/main.fcg.callers.gen.dot.svg
fi
else
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    source indent_jsonfiles.sh .
fi
