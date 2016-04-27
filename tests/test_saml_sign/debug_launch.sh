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

    source indent_jsonfiles.sh $callers_json_rootdir

fi
else
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    source indent_jsonfiles.sh .
fi
