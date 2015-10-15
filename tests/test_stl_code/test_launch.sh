#!/bin/bash
#echo "user_dir: $PWD"
test_dir=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)
#echo "test_dir: $test_dir"
#echo "check_pwd: $PWD"

#build_tool="cmake"
build_tool="scan-callers"
#build_tool="scan-build"

#analysis_type=all
analysis_type=callers
#analysis_type=frama_c
#analysis_type=frama_clang
#analysis_type=framaCIRGen

#clang=`which clang`

common=`which common.sh`
launch_scan_build=`which launch_analysis.sh`

source $common
source $launch_scan_build

source test_clean.sh

launch_the_analysis ${build_tool} ${analysis_type}

if [ $build_tool != "scan-build" ]
#if false
then
if [ $analysis_type == "callers" ] || [ $analysis_type == "all" ];
then
    # list the symbols referenced by the program and defined in the standard C++ library

    includes_directories="/usr/include/c++/4.7"
    #includes_directories="/usr/include/c++/4.8"

    for inc_dir in $includes_directories
    do
	cd $inc_dir
	list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json analysis

        # List all defined symbols in file defined_symbols.json
	list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json

	source indent_jsonfiles.sh .
    done

    cd $test_dir

    # List generated json files
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    list_files_in_dirs `pwd` .file.callers.gen.json dir.callers.gen.json analysis

    # List all defined symbols in application root directory
    list_defined_symbols defined_symbols.json `pwd` dir.callers.gen.json
    #read_defined_symbols defined_symbols.json file.callers.gen.json

    # add extcallees to json files
    #source add_extcallees.sh `pwd`
    source add_extcallees.sh `pwd` $includes_directories

    # add extcallers to json files
    source add_extcallers.sh .
    source indent_jsonfiles.sh .

    ## generate callee's tree from main entry point
    #source function_calls_to_dot.sh callees `pwd`/test_std_map.cpp "std_map" "int main()" files
    #source function_calls_to_dot.sh callees `pwd`/test_std_map0.cpp "std_map0" "int main()" files
    source function_calls_to_dot.sh callees `pwd`/test_std_map1.cpp "std_map1" "int main()" files
    # source function_calls_to_dot.sh callees `pwd`/test_std_map2.cpp "std_map2" "int main(int argc, char** argv)" files
    # source function_calls_to_dot.sh callees `pwd`/test_std_map3.cpp "std_map3" "int main(int argc, char** argv)" files
    #source function_calls_to_dot.sh callees `pwd`/test_std_set.cpp "std_set" "int main()" files

    ## generate caller's tree from main entry point
    source function_calls_to_dot.sh callers `pwd`/test_std_map.cpp "std_map" "int main()" files
    #source function_calls_to_dot.sh callers `pwd`/test_std_set.cpp "std_set" "int main()" files

    source process_dot_files.sh . analysis/${analysis_type}

    #inkscape analysis/${analysis_type}/svg/std_map.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/std_map0.fct.callees.gen.dot.svg
    inkscape analysis/${analysis_type}/svg/std_map1.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/std_set.fct.callers.gen.dot.svg
fi
else
    find . -type f -name "*.gen.json.gz" -exec gunzip {} \;
    source indent_jsonfiles.sh .
fi
