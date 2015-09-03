#!/bin/bash
#set -x

canonical_pwd="/media/sf_users_3u$PWD"

source "../../bin/common.sh"

#analysis_type=$1
#analysis_type=all
analysis_type=callers
#analysis_type=frama-clang
#analysis_type=framaCIRGen

# clean test
source clean.sh

# launch callers analysis
mkdir analysis
cd analysis
cmake ..
#make
cmake_analysis.sh compile_commands.json ${analysis_type} all 
# scan-build -o callers cmake ..
# scan-build -o callers make
if [ $? -ne 0 ]; then
    echo "################################################################################"
    echo "# ${analysis_type} analysis error. Stop here !"
    echo "################################################################################"
    exit -1
fi
cd ..

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
    #source add_extcallees.sh `pwd` defined_symbols.all.gen.json
    #source add_extcallees.sh `pwd` broken_symbols.json

    # add extcallers to json files
    source add_extcallers.sh .
    source indent_jsonfiles.sh .

    # generate callee's tree from main entry point
    source function_callers_to_dot.sh $canonical_pwd/test.cpp callees "main" "int main()" files

    # generate caller's tree from main entry point
    source function_callers_to_dot.sh $canonical_pwd/dirB/B.cpp callers "c" "int c()" files

    source process_dot_files.sh . analysis/${analysis_type}

    inkscape analysis/${analysis_type}/svg/main.fct.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/c.fct.callers.gen.dot.svg

fi
