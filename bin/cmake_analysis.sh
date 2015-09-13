#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

progname=$0
version=0.0.1

source "prepare_frama_clang_analysis.sh"

# func_usage
# outputs to stdout the --help usage message.
function func_usage ()
{
    provided_cmds=$@
    echo "################################################################################"
    echo "# shell script to launch some clang analysis plugins"
    echo "# version $version"
    echo "################################################################################"
    echo "# Usage:"
    echo "# cmake_analysis.sh <cmake_compile_commands.json> <analysis_type=callers|frama-clang|framaCIRGen|all> <files=all|specific_file>"
    echo "# Provided command was: ${provided_cmds}"
    exit -1
}

# func_version
# outputs to stdout the --version message.
function func_version ()
{
    echo "################################################################################"
    echo "# clang frama-clang plugin v$version"
    echo "# located at $progname"
    echo "# Copyright (C) 2015 Thales Communication & Security"
    echo "# Written by Hugues Balp"
    echo "#  - All Rights Reserved"
    echo "# There is NO WARRANTY, to the extent permitted by law."
    echo "################################################################################"
    exit 0
}

provided_cmds=$@

if test $# = 0; then
    func_usage $provided_cmds; 
    exit 0
    
elif test $# = 1; then

    case "$1" in
	--help | --hel | --he | --h )
	    func_usage; exit 0 ;;
	--version | --versio | --versi | --vers | --ver | --ve | --v )
	    func_version ;;
	*)
	    func_usage $provided_cmds;;
    esac

elif test $# = 3; then

    compile_commands_json=$1
    # analysis_type = callers | frama-clang | framaCIRGen | all
    analysis_type=$2
    files=$3

    json_filename=`basename ${compile_commands_json}`
    
    case $json_filename in

	"compile_commands.json" )
	    echo "json_file: ${json_filename}"
	    ;;
	*)
	    func_usage $provided_cmds
	    ;;
    esac

    export CALLERS_ANALYSIS_TYPE=${analysis_type}

    # prepare launch analysis script
    launch_script=.tmp.gen.analysis.launch.cmake.sh
    stderr_file="${launch_script}.stderr"
    prepare_analysis_from_cmake_compile_commands $stderr_file $compile_commands_json > ${launch_script}
    echo "generated launch script: ${launch_script}"

    # launch the analysis
    echo "launch the analysis..."
    analysis_report=$analysis_type
    mkdir -p ${analysis_report}
    source ${launch_script} > ${analysis_report}/${launch_script}.stdout 2> ${analysis_report}/${launch_script}.stderr    
else
    func_usage $provided_cmds
fi
