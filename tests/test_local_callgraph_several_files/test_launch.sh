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

    # List generated json files
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json "analysis"

    # List all defined symbols in file defined_symbols.all.gen.json
    list_defined_symbols defined_symbols.all.gen.json `pwd` dir.callers.gen.json
    # read_defined_symbols.native defined_symbols.all.gen.json file.callers.gen.json

    # add extcallees to json files
    source add_extcallees.sh `pwd`
    #source add_extcallees.sh `pwd` broken_symbols.json

    # add extcallers to json files
    source add_extcallers.sh .
    source indent_jsonfiles.sh .

    # generate callee's tree from main entry point
    source function_callers_to_dot.sh callees $canonical_pwd/test.cpp "main" "int main()" files

    # generate caller's tree from main entry point
    source function_callers_to_dot.sh callers $canonical_pwd/dirB/B.cpp "c" "int c()" files

    source process_dot_files.sh . analysis/${analysis_type}

    inkscape analysis/${analysis_type}/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/c.fct.callers.gen.dot.svg
fi
fi
