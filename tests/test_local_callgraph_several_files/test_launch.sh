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

# clean test
source clean.sh

source $common
source $launch_scan_build

# launch the analysis
launch_the_analysis ${build_tool} ${analysis_type}

if [ $build_tool != "scan-build" ]
#if false
then
if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ];
then

    # list the symbols referenced by the program and defined in the standard C++ library

    #includes_directories="/usr/include/c++/4.7"
    #includes_directories="/usr/include/c++/4.8"
    # includes_directories="/usr/include"

    # for inc_dir in $includes_directories
    # do
    #     cd $inc_dir
    #     list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json analysis

    #     # List all defined symbols in file defined_symbols.json
    #     list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json

    #     source indent_jsonfiles.sh .
    # done

    cd $test_dir

    # List generated json files
    find $callers_json_rootdir -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs $callers_json_rootdir .file.callers.gen.json dir.callers.gen.json "analysis"

    # List all defined symbols in file defined_symbols.all.gen.json
    list_defined_symbols defined_symbols.all.gen.json

    # read_defined_symbols.native defined_symbols.all.gen.json file.callers.gen.json

    # add extcallees to json files
    source add_extcallees.sh $callers_json_rootdir
    #source add_extcallees.sh $callers_json_rootdir broken_symbols.json

    # add extcallers to json files
    source add_extcallers.sh $callers_json_rootdir

    # generate callee's tree from main entry point
    # source function_calls_to_dot.sh callees $canonical_pwd/test.cpp "main" "int main()" files
    source extract_fcg.sh callees ${canonical_pwd}/test.cpp "main" "int main()" files

    # generate caller's tree from main entry point
    # source function_calls_to_dot.sh callers ${canonical_pwd}/dirB/B.cpp "c" "int c()" files
    source extract_fcg.sh callers ${canonical_pwd}/dirB/B.cpp "c" "int c()" files

    source callgraph_to_ecore.sh .
    source callgraph_to_dot.sh . files

    source process_dot_files.sh . analysis/${analysis_type}

    inkscape analysis/${analysis_type}/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/c.fct.callers.gen.dot.svg

    source indent_jsonfiles.sh .
    source indent_jsonfiles.sh $callers_json_rootdir
fi
fi
