#!/bin/bash

#canonical_pwd="$PWD"
canonical_pwd="/net/alpha.sc2.theresis.org/$PWD"
#canonical_pwd="/net/alpha.sc2.theresis.org/works$PWD"

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

#source test_clean.sh

source $common
source $launch_scan_build

launch_the_analysis ${build_tool} ${analysis_type}

source indent_jsonfiles.sh $callers_json_rootdir
