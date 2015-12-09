#!/bin/bash
#set -x

build_tool="cmake"
#build_tool="scan-callers"
#build_tool="scan-build"

#analysis_type=$1
#analysis_type=all
analysis_type=callers
#analysis_type=frama-c
#analysis_type=framaCIRGen

common=`which common.sh`
bin_dir=`dirname $common`
launch_scan_build=`which ${bin_dir}/launch_analysis.sh`

# clean test
source test_clean.sh

source $common
source $launch_scan_build

# launch the analysis
launch_the_analysis ${build_tool} ${analysis_type}

callers_json_rootdir=/tmp/callers

if [ $build_tool != "scan-build" ]
#if false
then
if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ];
then

    # List generated json files
    find $callers_json_rootdir -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs $callers_json_rootdir .file.callers.gen.json dir.callers.gen.json "analysis"

    # List all defined symbols in file defined_symbols.json
    list_defined_symbols defined_symbols.json $callers_json_rootdir dir.callers.gen.json
    #read_defined_symbols.native defined_symbols.json file.callers.gen.json

    # add extcallees to json files
    source add_extcallees.sh $callers_json_rootdir defined_symbols.json

    # add extcallers to json files
    source add_extcallers.sh $callers_json_rootdir

    # generate callee's tree from main entry point
    #source function_calls_to_dot.sh callees `pwd`/test_local_callcycle.c "main" "int main()" files
    source extract_fcg.sh callees /tmp/callers`pwd`/test_local_callcycle.c "main" "int main()" files

    # generate caller's tree from main entry point
    #source function_calls_to_dot.sh callers /tmp/callers/`pwd`/test_local_callcycle.c "main" "int main()"
    source extract_fcg.sh callers /tmp/callers`pwd`/test_local_callcycle.c "a" "void a()"

    source callgraph_to_ecore.sh .
    source callgraph_to_dot.sh . files

    source process_dot_files.sh . analysis/${analysis_type}

    inkscape analysis/callers/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/callers/svg/main.fct.callers.gen.dot.svg
    #inkscape analysis/callers/svg/a.fct.callers.gen.dot.svg

    source indent_jsonfiles.sh .
    source indent_jsonfiles.sh $callers_json_rootdir
fi
fi
