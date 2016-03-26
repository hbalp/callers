#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

OPENSSL_ARCH_NAME=openssl-OpenSSL_1_0_2f

function openssl_clean ()
{
    echo "openssl clean..."
    rm -rf ${OPENSSL_ARCH_NAME}
}

function openssl_install ()
{
    echo "openssl install..."
    tar -zxf ../.svg/${OPENSSL_ARCH_NAME}.tar.gz

    cd ${OPENSSL_ARCH_NAME}
    #make clean          # Clean the current configuration away
    cd ..
}

function openssl_build ()
{
    echo "openssl build..."
    cd ${OPENSSL_ARCH_NAME}
    #make clean          # Clean the current configuration away
    #mkdir ../${OPENSSL_BUILD_DIR}
    #cd ../${OPENSSL_BUILD_DIR}
    #../${OPENSSL_DIR}/config --unified shared --prefix=/opt/stance
    ../${OPENSSL_ARCH_NAME}/Configure debug-linux-x86_64-clang shared --prefix=/opt/stance
    make
    make test
    cd ..
}

function openssl_callers ()
{
    export CALLERS_ANALYSIS_TYPE=callers
    rm -rf /tmp/callers

    cd ${OPENSSL_ARCH_NAME}
    #echo "openssl clean..."
    make clean > /dev/null 2> /dev/null # Clean the current configuration away

    echo "openssl config..."
    ../${OPENSSL_ARCH_NAME}/Configure debug-linux-x86_64-clang shared --prefix=/opt/stance > ../.openssl_callers.config.stdout # 2> ../.openssl_callers.config.stderr
    #../${OPENSSL_ARCH_NAME}/Configure debug-linux-x86_64-callers shared --prefix=/opt/stance

    echo "openssl callers analysis..."
    scan-callers --use-analyzer `which clang` -o analysis make > ../.openssl_callers.gen.stdout 2> ../.openssl_callers.gen.stderr
    result=$?
    echo "retcode: ${result}"

    #make test

    cd ..
}

function openssl_frontends ()
{
    canonical_pwd="$PWD"

    #build_tool="cmake"
    build_tool="scan-callers"
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
    open_ssl_dir=$test_dir/${OPENSSL_ARCH_NAME}

    source $common
    source $launch_scan_build

    cd ${openssl_dir}

    # List generated json files
    # find $callers_json_rootdir -type f -name "*.gen.json.gz" -exec gunzip {} \;
    # list_files_in_dirs $callers_json_rootdir .file.callers.gen.json dir.callers.gen.json "analysis"

    # # List all defined symbols in file defined_symbols.all.gen.json
    # list_defined_symbols defined_symbols.all.gen.json

    # # # add declarations to json files
    # # source add_declarations.sh $callers_json_rootdir

    # # # add definitions to json files
    # # source add_definitions.sh $callers_json_rootdir

    # # # add extcallees to json files
    # # source add_extcallees.sh $callers_json_rootdir
    # # #source add_extcallees.sh $callers_json_rootdir broken_symbols.json

    # # # add extcallers to json files
    # # source add_extcallers.sh $callers_json_rootdir

    # # add inherited to json files
    # source add_inherited.sh $callers_json_rootdir

    # # add virtual function calls to json files
    # source add_virtual_function_calls.sh $callers_json_rootdir

    # generate callee's tree from main entry point
    # source extract_fcg.sh callees "$canonical_pwd/test.cpp" "main" "int main()" files
    # source extract_fcg.sh callees "${canonical_pwd}/${OPENSSL_ARCH_NAME}/ssl/ssltest.c" "main" "int main()" files
    source extract_fcg.sh callees "${canonical_pwd}/${OPENSSL_ARCH_NAME}/ssl/ssltest.c" "main" "int main(int, ((char)*)[])"

    # source extract_fcg.sh callees "${canonical_pwd}/${OPENSSL_ARCH_NAME}/ssl/ssltest.c" "main" "toto"
    # source extract_fcg.sh callees "${canonical_pwd}/${OPENSSL_ARCH_NAME}/ssl/ssltest.c" "main" "toto titi"
    # source extract_fcg.sh callees "${canonical_pwd}/${OPENSSL_ARCH_NAME}/ssl/ssltest.c" "main" "toto (tutu)*"
    # source extract_fcg.sh callees "${canonical_pwd}/${OPENSSL_ARCH_NAME}/ssl/ssltest.c" "main" "int toto(tutu, ((titi)*)[])"
    # source extract_fcg.sh callees "${canonical_pwd}/${OPENSSL_ARCH_NAME}/ssl/ssltest.c" "main" "int tarzan(int, ((char)*)[])"

    # generate caller's tree from main entry point
    # source extract_fcg.sh callers "${canonical_pwd}/dirB/B.cpp" "c" "int c()" files
    # source extract_fcg.sh callers ${canonical_pwd}/dirB/B.hpp "c" "int c()" files
    # source extract_fcg.sh callers "/usr/include/stdio.h" "printf" "printf" files
    # source extract_fcg.sh callers "/usr/include/malloc.h" "malloc" "malloc" files
    source extract_fcg.sh callers "${canonical_pwd}/${OPENSSL_ARCH_NAME}/crypto/evp/evp.h" "EVP_EncryptUpdate" "int EVP_EncryptUpdate((evp_cipher_ctx_st)*, (unsigned char)*, (int)*, (const unsigned char)*, int)"

    # source extract_fcg.sh c2c "$canonical_pwd/dirA/A.cpp" "A_a" "int A::a()" "/usr/include/stdio.h" "printf" "printf" files

    # source callgraph_to_ecore.sh $callers_json_rootdir
    # source callgraph_to_dot.sh $callers_json_rootdir files

    # # generate classes tree from base class A
    # # source classes_depgraph.sh child "$canonical_pwd/dirA/A.hpp" "A"
    # # source classes_depgraph.sh base "$canonical_pwd/dirC/D.hpp" "::Newly::Added::D"

    # source process_dot_files.sh . analysis/${analysis_type}

    # #source indent_jsonfiles.sh .
    # source indent_jsonfiles.sh $callers_json_rootdir

#    inkscape analysis/${analysis_type}/svg/main.fcg.callees.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/printf.fcg.callees.gen.dot.svg

    #inkscape analysis/${analysis_type}/svg/A.child.classes.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/::Newly::Added::D.base.classes.gen.dot.svg

    #inkscape analysis/${analysis_type}/svg/c.fcg.callers.gen.dot.svg
    #inkscape analysis/${analysis_type}/svg/c.fct.callers.gen.dot.svg
}

# openssl_clean
# openssl_install
# #openssl_build
# time openssl_callers
#openssl_frontends
time openssl_frontends > .openssl_fcg.gen.stdout 2> .openssl_fcg.gen.stderr
