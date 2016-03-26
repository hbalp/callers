#!/bin/bash

function list_all_clang_checkers ()
{
    clang -cc1 -analyzer-checker-help | awk '{print $1}' | egrep "\."
}

function list_all_clang_checkers_families ()
{
    clang -cc1 -analyzer-checker-help | awk '{print $1}' | egrep "\." | sed -e "s/\./ /g" | awk '{print $1}' | sort -u
}

function scan_build_enable_some_clang_checkers_families ()
{
    #all_clang_checkers_families="alpha core cplusplus deadcode debug llvm nullability optin osx security unix"
    some_clang_checkers_families="alpha core cplusplus deadcode nullability optin security unix";
    enable_checkers=""
    for checker_family in ${some_clang_checkers_families};
    do
        enable_checkers="${enable_checkers} -enable-checker ${checker_family}";
    done
    echo "${enable_checkers}"
}

function scan_build_enable_all_clang_checkers_families ()
{
    enable_checkers=""
    for checker_family in `list_all_clang_checkers_families`;
    do
        enable_checkers="${enable_checkers} -enable-checker ${checker_family}";
    done
    echo "${enable_checkers}"
}

function scan_build_filter_some_clang_checkers_families ()
{
    enable_checkers=""
    for checker_family in `list_all_clang_checkers_families`;
    do
        # if [ [ ${checker_family} != "osx" ] -o [ ${checker_family} != "debug" ] ]; then
        # if [ [ ${checker_family} != "osx" ] || [ ${checker_family} != "debug" ] ]; then
        if [ ${checker_family} != "osx" ]; then
            if [ ${checker_family} != "debug" ]; then
                enable_checkers="${enable_checkers} -enable-checker ${checker_family}";
            fi
        fi
    done
    echo "${enable_checkers}"
}

function launch_scan_build ()
{
    build_cmd=$@
    #scan_build_checkers_options=$2
    #scan_build_checkers_options="-v -v -V -enable-checker debug.DumpCallGraph -enable-checker debug.DumpCalls -enable-checker debug.ViewCallGraph"
    enable_clang_checkers_families=`scan_build_enable_some_clang_checkers_families`
    scan_build_checkers_options="-v -v -V ${enable_clang_checkers_families}"
    output_dir="scan-build"
    mkdir -p ${output_dir}
    echo "Launch scan-build analysis..."
    #echo "Launch scan-build \"${scan_build_checkers_options}\" analysis..."
    clang=`which clang`
    # uses the scan-build script adapted for build analysis
    #scan_cmd="scan-build --use-analyzer ${clang} ${scan_build_checkers_options} -o ${output_dir} ${build_cmd} > test.gen.stdout 2> test.gen.stderr"
    scan_cmd="scan-build --use-analyzer ${clang} ${scan_build_checkers_options} -o ${output_dir} ${build_cmd}"
    echo $scan_cmd;
    $scan_cmd
    if [ $? -ne 0 ]; then
	echo "################################################################################"
	echo "# Analysis error. Stop here ! : scan-build --use-analyzer ${clang} ${scan_build_checkers_options}"
	echo "################################################################################"
	exit -1
    fi
}

#launch_scan_build cmake ..
#launch_scan_build make

