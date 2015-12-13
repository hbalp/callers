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
launch_scan_build=`which launch_analysis.sh`

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

    # List all defined symbols in file defined_symbols.json
    list_defined_symbols defined_symbols.json
    #read_defined_symbols defined_symbols.json file.callers.gen.json

    # add extcallees to json files
    source add_extcallees.sh $callers_json_rootdir defined_symbols.json

    # add extcallers to json files
    source add_extcallers.sh $callers_json_rootdir

    ## generate callee's tree from main entry point
    source extract_fcg.sh callees `pwd`/test_dummy.c "main" "int main()" files

    ## generate caller's tree from main entry point
    source extract_fcg.sh callers `pwd`/test_dummy.c "main" "int main()" files

    source callgraph_to_ecore.sh $callers_json_rootdir
    source callgraph_to_dot.sh $callers_json_rootdir files

    source process_dot_files.sh . analysis/${analysis_type}

    source indent_jsonfiles.sh .
    source indent_jsonfiles.sh $callers_json_rootdir

    inkscape analysis/${analysis_type}/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/main.fct.callers.gen.dot.svg

fi
fi
