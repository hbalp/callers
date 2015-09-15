#!/bin/bash
#set -x

# launch scan-callers = scan-build adapted for callers analysis over a cmake build
function launch_scan_callers_over_cmake()
{
    here=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)
    #this_script=`dirname "${BASH_SOURCE[0]}"`
    analysis_type=$1
    echo "Launch scan-callers \"${analysis_type}\" analysis..."
    export CALLERS_ANALYSIS_TYPE="$analysis_type"
    clang=`which clang`
    # uses the scan-build script adapted for callers analysis
    $here/scan-callers --use-analyzer ${clang} -o ${analysis_type} cmake ..
    $here/scan-callers --use-analyzer ${clang} -o ${analysis_type} make VERBOSE=yes
    if [ $? -ne 0 ]; then
	echo "################################################################################"
	echo "# scan-callers \"${analysis_type}\" analysis error. Stop here !"
	echo "################################################################################"
	exit -1
    fi
}

# launch a clang scan-build analysis over a cmake build
function launch_scan_build_over_cmake()
{
    #scan_build_checkers_options=$1
    scan_build_checkers_options="-v -v -V -enable-checker debug.DumpCallGraph -enable-checker debug.DumpCalls -enable-checker debug.ViewCallGraph"

    echo "Launch scan-build \"${scan_build_checkers_options}\" analysis..."
    clang=`which clang`
    # uses the scan-build script adapted for build analysis
    scan-build --use-analyzer ${clang} ${scan_build_checkers_options} -o ${analysis_type} cmake ..
    scan-build --use-analyzer ${clang} ${scan_build_checkers_options} -o ${analysis_type} make VERBOSE=yes
    if [ $? -ne 0 ]; then
	echo "################################################################################"
	echo "# scan-build \"${scan_build_checkers_options}\" analysis error. Stop here !"
	echo "################################################################################"
	exit -1
    fi
}

# launch cmake analysis
function launch_cmake_analysis()
{
    here=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)
    #this_script=`dirname "${BASH_SOURCE[0]}"`
    analysis_type=$1
    echo "Launch cmake \"${analysis_type}\" analysis..."
    cmake_analysis.sh compile_commands.json ${analysis_type} all
    if [ $? -ne 0 ]; then
	echo "################################################################################"
	echo "# cmake \"${analysis_type}\" analysis error. Stop here !"
	echo "################################################################################"
	exit -1
    fi
}

# launch the analysis
function launch_the_analysis()
{
    build_tool=$1
    analysis_type=$2
    mkdir analysis
    cd analysis
    if [ $build_tool == "cmake" ]
    then
	launch_cmake_analysis ${analysis_type}
    elif [ $build_tool == "scan-callers" ]
    then
	launch_scan_callers_over_cmake ${analysis_type}
    elif [ $build_tool == "scan-build" ]
    then
	launch_scan_build_over_cmake
    fi
    cd ..
}
