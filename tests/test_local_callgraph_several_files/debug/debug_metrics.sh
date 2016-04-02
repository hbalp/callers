#!/bin/bash
#set -x

#canonical_pwd="/media/sf_users_3u$PWD"
#canonical_pwd="$PWD"
canonical_pwd="/net/alpha.sc2.theresis.org$PWD"
#canonical_pwd="/net/alpha.sc2.theresis.org/works/home/balp/third_parties/llvm/tools/clang/tools/extra/callers/tests/test_local_callgraph_several_files"

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
source test_clean.sh

source $common
source $launch_scan_build

# # launch the analysis
# launch_the_analysis ${build_tool} ${analysis_type}

if [ $build_tool != "scan-build" ]
#if false
then
if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ];
then

    cd $test_dir

    # List generated json files
    # find $callers_json_rootdir -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs $callers_json_rootdir .file.callers.gen.json dir.callers.gen.json "analysis"

    # Extract metrics
    extract_metrics metrics.callers.gen.json

    source indent_jsonfiles.sh .
    # source indent_jsonfiles.sh $callers_json_rootdir

fi
else
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    source indent_jsonfiles.sh $callers_json_rootdir
fi
