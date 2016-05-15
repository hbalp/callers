#!/bin/bash
# @author Hugues Balp
# This script manages both build and analysis of the test_saml_sign example

################################################################################
#                         TO BE EDITED WHEN NEEDED
################################################################################

# callers: Extract some Function Call Graph adapted to the target example and to the verification goals
#  to be edited when needed
#  precondition: cmake_callers_extract_metrics (or at least list_files_in_dirs)
function cmake_callers_it_extract_fcg ()
{
    cd ${ici}
    canonical_pwd=`pwd`
    cd ${BUILD_DIR}
    #canonical_pwd="/net/alpha.sc2.theresis.org$PWD"

    ## generate callee's tree from main entry point
    #source extract_fcg.sh callees ${canonical_pwd}/test_saml_sign.it.c "main" "int main(int, ((char)*)*)" files
    source extract_fcg.sh callees ${canonical_pwd}/test_saml_sign.it.c "main" "int main(int, ((char)*)*)"
    
    ## generate caller's tree from main entry point
    # source extract_fcg.sh callers `pwd`/test_saml_sign.c "main" "int main()" files

    source callgraph_to_ecore.sh $callers_json_rootdir
    source callgraph_to_dot.sh $callers_json_rootdir files

    source process_dot_files.sh . callers

    source indent_jsonfiles.sh .
    source indent_jsonfiles.sh $callers_json_rootdir
    cd ${ici}
}

function libxml2_config_host_moriond ()
{
    LIBXML2_SYS_INCLUDES_DIR="/usr/include/libxml2"
    LIBXML2_SYS_LIB_DIR="/usr/lib/x86_64-linux-gnu"
    LIBXML2_SYS_LIB_INSTALL_PATH="${LIBXML2_SYS_LIB_DIR}/libxml2.so"
    
    LIBXML2_DEV_INCLUDES_DIR="/tools/exec/include/libxml2"
    LIBXML2_DEV_LIB_GDB_SRC_DIR="/home/hbalp/hugues/work/third_parties/src/libxml2_gdb"
    LIBXML2_DEV_LIB_FC_SRC_DIR="/home/hbalp/hugues/work/third_parties/src/libxml2_fc"
    LIBXML2_DEV_LIB_INSTALL_PATH="/tools/exec/lib/libxml2.a"
    EXTRA_DEV_LIBS_DIRS="${EXTRA_DEV_LIBS_DIRS} ${LIBXML2_DEV_LIB_FC_SRC_DIR}"
}

function libxml2_config_host_vm ()
{
    LIBXML2_SYS_INCLUDES_DIR="/usr/include/libxml2"
    LIBXML2_SYS_LIB_DIR="/usr/lib/x86_64-linux-gnu/libxml2.so"
    LIBXML2_SYS_LIB_PATH="${LIBXML2_SYS_LIB_DIR}/libxml2.so"
    
    LIBXML2_DEV_INCLUDES_DIR="/data/balp/src/tools/exec/include/libxml2"
    LIBXML2_DEV_LIB_GDB_SRC_DIR="/data/balp/src/tools/libxml2_gdb"
    LIBXML2_DEV_LIB_FC_SRC_DIR="/data/balp/src/tools/libxml2_fc"
    LIBXML2_DEV_LIB_INSTALL_PATH="/data/balp/src/tools/exec/lib/libxml2.a"
    EXTRA_DEV_LIBS_DIRS="${EXTRA_DEV_LIBS_DIRS} ${LIBXML2_DEV_LIB_FC_SRC_DIR}"
}

################################################################################
#                       SHOULD NOT BEEN EDITED NORMALLY
################################################################################

#
# Common
#

function usage_test_saml_sign_launch ()
{
    echo "test_saml_sign usage:"
    echo "either:"
    echo "  build all config with: cmake_build_all"
    echo "or:"
    echo "  1: cmake_config_common"
    echo "  2: choose one of the cmake_config_* function"
    echo "  3: call script cmake_build_execute to generate and build the config"
    echo "  with the appropriate generated cmake file CMakeLists.txt"
}
 
function cmake_build_all ()
{
    cmake_config_common
    cmake_build_all_gdb
    cmake_build_all_fc
    cmake_build_all_callers
    cd $ici
}

function cmake_build_all_gdb ()
{
    cmake_config_common
    cmake_build_it_gdb
    cmake_build_ut_gdb
    cd $ici
}

function cmake_build_all_callers ()
{
    cmake_build_it_callers
    cd $ici
}

function cmake_build_all_fc ()
{
    cmake_config_common
    cmake_build_it_fc
    cmake_build_ut_fc
    cd $ici
}

function cmake_config_common ()
{
    ici=`pwd`
    CMAKE_MINIMUM_VERSION="2.8"
    CMAKE_EXPORT_COMPILE_COMMANDS="ON"
    CMAKE_BUILD_TYPE="Debug"
    #CMAKE_BUILD_TYPE="Release"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C_SHARE_PATH=`frama-c -print-share-path`
    FRAMA_C_LIBC_DIR="${FRAMA_C_SHARE_PATH}/libc"
    # slevle should be upper then the sizeof XML_MEM_BREAKPOINT used as input in strcmp
    FRAMA_C_SLEVEL=80
    #FRAMA_C_SLEVEL=30
    #FRAMA_C_SLEVEL=2
    #FRAMA_C_MODIFIED_FILES="stub/tree.c libc/libc.c libc/stdlib.c libc/setgetenv.c"
    #FRAMA_C_MODIFIED_FILES="stub/tree.c libc/stdlib.c libc/setgetenv.c"
    FRAMA_C_MODIFIED_FILES="libc/setgetenv.c"
    EXTRA_DEV_LIBS_DIRS=""
    build_config_host
}

function cmake_build_it_gdb ()
{
    cmake_config_common
    # cmake_config_it_gdb_lib-sys    && cmake_build_execute
    cmake_config_it_gdb_lib-dev_without_xsw_countermeasure  && cmake_build_execute
    cmake_config_it_gdb_lib-dev_with_xsw_countermeasure     && cmake_build_execute && cmake_run_it_gdb
    # cmake_config_it_gdb_stub       && cmake_build_execute
}

# precondition: cmake_build_it_gdb
function cmake_run_it_gdb ()
{
    cd ${BUILD_DIR}
    
    # To check potential memory vulnerabilities with valgrind
    valgrind ./xsw_test_saml_sign ../data/SAMLResponse.malicious_xsw.xml

    # To get the execution path with valgrind
    valgrind --tool=callgrind --callgrind-out-file=callgrind.gen.out ./xsw_test_saml_sign ../data/SAMLResponse.malicious_xsw.xml
    
    kcachegrind callgrind.gen.out &
    cd ${ici}
}

function cmake_build_it_callers ()
{
    cmake_config_common &&
    cmake_config_it_callers_lib-dev &&
    cmake_build_generate &&
    cmake_callers_execute &&
    cmake_callers_extract_metrics &&
    cmake_callers_it_extract_fcg
}

function cmake_build_it_fc ()
{
    cmake_config_common
    # cmake_config_it_fc-va_lib-dev_with_xsw_countermeasure   && cmake_build_execute && it_fc_main_gen && fc_parse_prepare
    cmake_config_it_fc-va_lib-dev_with_xsw_countermeasure   && cmake_build_execute && fc_parse_prepare
    #cmake_config_it_fc-va_lib-dev_with_xsw_countermeasure   && cmake_build_execute && ( fc_parse_prepare > ${BUILD_DIR}/fc_va.stdout  2> ${BUILD_DIR}/fc_va.stderr )
    # # cmake_config_it_fc-va_stub     && cmake_build_execute && fc_parse_prepare
    cd $ici
}

function cmake_build_ut_gdb ()
{
    # cmake_config_ut_gdb_lib-sys    && cmake_build_execute
    cmake_config_ut_gdb_lib-dev_without_xsw_countermeasure  && cmake_build_execute
    cmake_config_ut_gdb_lib-dev_with_xsw_countermeasure     && cmake_build_execute
    # cmake_config_ut_gdb_stub       && cmake_build_execute
    cd $ici
}

function cmake_build_ut_fc ()
{
    cmake_config_common
    # cmake_config_ut_fc-va_lib-sys  && cmake_build_execute && fc_parse_prepare
    cmake_config_ut_fc-va_lib-dev_with_xsw_countermeasure    && cmake_build_execute && fc_parse_prepare
    cmake_config_ut_fc-va-wd_lib-dev_with_xsw_countermeasure && cmake_build_execute && fc_parse_prepare
    # cmake_config_ut_fc-va_stub     && cmake_build_execute && fc_parse_prepare
    cd $ici
}

function cmake_clean_all_callers ()
{
    cmake_config_common
    cmake_config_it_callers_lib-dev && cmake_build_clean
    cd ${ici}
    rm -f *.gen.callgraph
    rm -f *.gen.dot
    rm -f *.gen.json
    source callers_analysis.sh
    callers_reset
}

function cmake_clean_all_fc ()
{
    cmake_config_common
    # cmake_config_it_gdb_lib-sys    && cmake_build_clean
    # cmake_config_it_fc-va_lib-sys  && cmake_build_clean
    # cmake_config_it_gdb_lib-dev_without_xsw_countermeasure  && cmake_build_clean
    # cmake_config_it_gdb_lib-dev_with_xsw_countermeasure     && cmake_build_clean
    cmake_config_it_fc-va_lib-dev_with_xsw_countermeasure   && cmake_build_clean
    # cmake_config_it_gdb_stub       && cmake_build_clean
    # cmake_config_it_fc-va_stub     && cmake_build_clean
    # cmake_config_ut_gdb_lib-sys    && cmake_build_clean
    # cmake_config_ut_fc-va_lib-sys  && cmake_build_clean
    # cmake_config_ut_gdb_lib-dev_without_xsw_countermeasure   && cmake_build_clean
    # cmake_config_ut_gdb_lib-dev_with_xsw_countermeasure      && cmake_build_clean
    cmake_config_ut_fc-va_lib-dev_with_xsw_countermeasure    && cmake_build_clean
    cmake_config_ut_fc-va-wd_lib-dev_with_xsw_countermeasure && cmake_build_clean
    # cmake_config_ut_gdb_stub       && cmake_build_clean
    # cmake_config_ut_fc-va_stub     && cmake_build_clean
    cd $ici
}

function build_config_host()
{    
    build_config_host_moriond
    # build_config_host_vm
}

function build_config_host_moriond ()
{
    libxml2_config_host_moriond
}

function build_config_host_vm ()
{
    libxml2_config_host_vm
}

#
# Integration tests
#

# function cmake_config_it_gdb_lib-sys_without_xsw_countermeasure ()
# {
#     XSW_COUNTERMEASURE="OFF"
#     BUILD_DIR="test_it_gdb_lib-sys_without_xsw_countermeasure.gen"
#     TEST_MAIN_SRC_FILE="test_saml_sign.it.c"
#     SYSTEM_LIB="ON"
#     DEV_LIB="OFF"
#     INTG_TEST="ON"
#     UNIT_TEST="OFF"
#     ADAPTED_CALL_CONTEXT="ON"
#     USE_XML_MEM_TRACE="ON"
#     USE_XML_MEM_BREAKPOINT="ON"
#     FRAMA_C="OFF"
#     LIBXML2_STUB="OFF"
#     SAVE_TEMPS="ON"
# }

# function cmake_config_it_gdb_lib-sys_with_xsw_countermeasure ()
# {
#     XSW_COUNTERMEASURE="ON"
#     BUILD_DIR="test_it_gdb_lib-sys_with_xsw_countermeasure.gen"
#     TEST_MAIN_SRC_FILE="test_saml_sign.it.c"
#     SYSTEM_LIB="ON"
#     DEV_LIB="OFF"
#     INTG_TEST="ON"
#     UNIT_TEST="OFF"
#     ADAPTED_CALL_CONTEXT="ON"
#     USE_XML_MEM_TRACE="ON"
#     USE_XML_MEM_BREAKPOINT="ON"
#     FRAMA_C="OFF"
#     LIBXML2_STUB="OFF"
#     SAVE_TEMPS="ON"
# }

# function cmake_config_it_fc-va_lib-sys_with_xsw_countermeasure ()
# {
#     XSW_COUNTERMEASURE="ON"
#     BUILD_DIR="test_it_fc-va_lib-sys_with_xsw_countermeasure.gen"
#     TEST_MAIN_SRC_FILE="test_saml_sign.it.c"
#     SYSTEM_LIB="ON"
#     DEV_LIB="OFF"
#     INTG_TEST="ON"
#     UNIT_TEST="OFF"
#     ADAPTED_CALL_CONTEXT="ON"
#     USE_XML_MEM_TRACE="ON"
#     USE_XML_MEM_BREAKPOINT="ON"
#     FRAMA_C="ON"
#     LIBXML2_STUB="OFF"
#     SAVE_TEMPS="ON"
# }

function cmake_config_it_gdb_lib-dev_without_xsw_countermeasure ()
{
    XSW_COUNTERMEASURE="OFF"
    BUILD_DIR="test_it_gdb_lib-dev_without_xsw_countermeasure.gen"
    TEST_MAIN_SRC_FILE="test_saml_sign.it.c"
    SYSTEM_LIB="OFF"
    DEV_LIB="ON"
    INTG_TEST="ON"
    UNIT_TEST="OFF"
    ADAPTED_CALL_CONTEXT="OFF"
    USE_XML_MEM_TRACE="OFF"
    USE_XML_MEM_BREAKPOINT="OFF"
    FRAMA_C="OFF"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_it_gdb_lib-dev_with_xsw_countermeasure ()
{
    XSW_COUNTERMEASURE="ON"
    BUILD_DIR="test_it_gdb_lib-dev_with_xsw_countermeasure.gen"
    TEST_MAIN_SRC_FILE="test_saml_sign.it.c"
    SYSTEM_LIB="OFF"
    DEV_LIB="ON"
    INTG_TEST="ON"
    UNIT_TEST="OFF"
    ADAPTED_CALL_CONTEXT="OFF"
    USE_XML_MEM_TRACE="OFF"
    USE_XML_MEM_BREAKPOINT="OFF"
    FRAMA_C="OFF"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_it_callers_lib-dev ()
{
    XSW_COUNTERMEASURE="ON"
    BUILD_DIR="test_it_callers_lib-dev.gen"
    TEST_MAIN_SRC_FILE="test_saml_sign.it.c"
    SYSTEM_LIB="OFF"
    DEV_LIB="ON"
    INTG_TEST="ON"
    UNIT_TEST="OFF"
    ADAPTED_CALL_CONTEXT="OFF"
    USE_XML_MEM_TRACE="OFF"
    USE_XML_MEM_BREAKPOINT="OFF"
    FRAMA_C="OFF"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="OFF" # Important to avoid an error reported by clang during callers analysis
}

function cmake_config_it_fc-va_lib-dev_with_xsw_countermeasure ()
{
    XSW_COUNTERMEASURE="ON"
    FRAMA_C_VA_WIDENING="OFF"
    BUILD_DIR="test_it_fc-va_lib-dev_with_xsw_countermeasure.gen"
    TEST_MAIN_SRC_FILE="test_saml_sign.it.c"
    SYSTEM_LIB="OFF"
    DEV_LIB="ON"
    INTG_TEST="ON"
    UNIT_TEST="OFF"
    ADAPTED_CALL_CONTEXT="OFF"
    USE_XML_MEM_TRACE="OFF"
    USE_XML_MEM_BREAKPOINT="OFF"
    FRAMA_C="ON"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

# this function generates an fc_main for FC-VA of the inttegation test
function it_fc_main_gen ()
{
    source fc_analysis.sh
    cd ${BUILD_DIR}
    exe="../test_it_gdb_lib-dev_with_xsw_countermeasure.gen/xsw_test_saml_sign"
    if [ -f ${exe} ]; then
       fc_main_gen ${exe} ../data/SAMLResponse.malicious_xsw.xml
    fi
}

# function cmake_config_it_gdb_stub ()
# {
#     XSW_COUNTERMEASURE="ON"
#     BUILD_DIR="test_it_gdb_stub.gen"
#     TEST_MAIN_SRC_FILE="test_saml_sign.it.c"
#     SYSTEM_LIB="OFF"
#     DEV_LIB="OFF"
#     INTG_TEST="ON"
#     UNIT_TEST="OFF"
#     ADAPTED_CALL_CONTEXT="ON"
#     USE_XML_MEM_TRACE="ON"
#     USE_XML_MEM_BREAKPOINT="ON"
#     FRAMA_C="OFF"
#     LIBXML2_STUB="ON"
#     SAVE_TEMPS="ON"
# }

# function cmake_config_it_fc-va_stub ()
# {
#     XSW_COUNTERMEASURE="ON"
#     BUILD_DIR="test_it_fc-va_stub.gen"
#     TEST_MAIN_SRC_FILE="test_saml_sign.it.c"
#     SYSTEM_LIB="OFF"
#     DEV_LIB="OFF"
#     INTG_TEST="ON"
#     UNIT_TEST="OFF"
#     ADAPTED_CALL_CONTEXT="ON"
#     USE_XML_MEM_TRACE="ON"
#     USE_XML_MEM_BREAKPOINT="ON"
#     FRAMA_C="ON"
#     LIBXML2_STUB="ON"
#     SAVE_TEMPS="ON"
# }

#
# Unitary tests
#

# function cmake_config_ut_gdb_lib-sys ()
# {
#     BUILD_DIR="test_ut_gdb_lib-sys.gen"
#     TEST_MAIN_SRC_FILE="test_saml_sign.ut.c"
#     SYSTEM_LIB="ON"
#     DEV_LIB="OFF"
#     INTG_TEST="OFF"
#     UNIT_TEST="ON"
#     ADAPTED_CALL_CONTEXT="ON"
#     USE_XML_MEM_TRACE="ON"
#     USE_XML_MEM_BREAKPOINT="ON"
#     FRAMA_C="OFF"
#     LIBXML2_STUB="OFF"
#     SAVE_TEMPS="ON"
# }

# function cmake_config_ut_fc-va_lib-sys ()
# {
#     BUILD_DIR="test_ut_fc-va_lib-sys.gen"
#     TEST_MAIN_SRC_FILE="test_saml_sign.ut.c"
#     SYSTEM_LIB="ON"
#     DEV_LIB="OFF"
#     INTG_TEST="OFF"
#     UNIT_TEST="ON"
#     ADAPTED_CALL_CONTEXT="ON"
#     USE_XML_MEM_TRACE="ON"
#     USE_XML_MEM_BREAKPOINT="ON"
#     FRAMA_C="ON"
#     LIBXML2_STUB="OFF"
#     SAVE_TEMPS="ON"
# }

function cmake_config_ut_gdb_lib-dev_without_xsw_countermeasure ()
{
    XSW_COUNTERMEASURE="OFF"
    BUILD_DIR="test_ut_gdb_lib-dev_without_xsw_countermeasure.gen"
    TEST_MAIN_SRC_FILE="test_saml_sign.ut.c"
    SYSTEM_LIB="OFF"
    DEV_LIB="ON"
    INTG_TEST="OFF"
    UNIT_TEST="ON"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="OFF"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_ut_gdb_lib-dev_with_xsw_countermeasure ()
{
    XSW_COUNTERMEASURE="ON"
    BUILD_DIR="test_ut_gdb_lib-dev_with_xsw_countermeasure.gen"
    TEST_MAIN_SRC_FILE="test_saml_sign.ut.c"
    SYSTEM_LIB="OFF"
    DEV_LIB="ON"
    INTG_TEST="OFF"
    UNIT_TEST="ON"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="OFF"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_ut_fc-va_lib-dev_with_xsw_countermeasure ()
{
    XSW_COUNTERMEASURE="ON"
    FRAMA_C_VA_WIDENING="OFF"
    BUILD_DIR="test_ut_fc-va_lib-dev_with_xsw_countermeasure.gen"
    TEST_MAIN_SRC_FILE="test_saml_sign.ut.c"
    SYSTEM_LIB="OFF"
    DEV_LIB="ON"
    INTG_TEST="OFF"
    UNIT_TEST="ON"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="ON"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_ut_fc-va-wd_lib-dev_with_xsw_countermeasure ()
{
    XSW_COUNTERMEASURE="ON"
    FRAMA_C_VA_WIDENING="ON"
    BUILD_DIR="test_ut_fc-va-wd_lib-dev_with_xsw_countermeasure.gen"
    TEST_MAIN_SRC_FILE="test_saml_sign.ut.c"
    SYSTEM_LIB="OFF"
    DEV_LIB="ON"
    INTG_TEST="OFF"
    UNIT_TEST="ON"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="ON"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

# function cmake_config_ut_gdb_stub ()
# {
#     BUILD_DIR="test_ut_gdb_stub.gen"
#     TEST_MAIN_SRC_FILE="test_saml_sign.ut.c"
#     SYSTEM_LIB="OFF"
#     DEV_LIB="OFF"
#     INTG_TEST="OFF"
#     UNIT_TEST="ON"
#     ADAPTED_CALL_CONTEXT="ON"
#     USE_XML_MEM_TRACE="ON"
#     USE_XML_MEM_BREAKPOINT="ON"
#     FRAMA_C="OFF"
#     LIBXML2_STUB="ON"
#     SAVE_TEMPS="ON"
# }

# function cmake_config_ut_fc-va_stub ()
# {
#     BUILD_DIR="test_ut_fc-va_stub.gen"
#     TEST_MAIN_SRC_FILE="test_saml_sign.ut.c"
#     SYSTEM_LIB="OFF"
#     DEV_LIB="OFF"
#     INTG_TEST="OFF"
#     UNIT_TEST="ON"
#     ADAPTED_CALL_CONTEXT="ON"
#     USE_XML_MEM_TRACE="ON"
#     USE_XML_MEM_BREAKPOINT="ON"
#     FRAMA_C="ON"
#     LIBXML2_STUB="ON"
#     SAVE_TEMPS="ON"
# }

function cmake_build_generate ()
{
    cat > CMakeLists.txt <<EOF
# File generated by test_saml_sign.launch.sh
# content manually adapted after a first generation by Callers's cmake_init.sh

cmake_minimum_required(VERSION ${CMAKE_MINIMUM_VERSION})

# generate list of source files with related compilation command
set(CMAKE_EXPORT_COMPILE_COMMANDS ${CMAKE_EXPORT_COMPILE_COMMANDS})

# add debug symbols
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})

# option to activate the XSW countermeasure
option(XSW_COUNTERMEASURE "Option to activate the XSW countermeasure" ${XSW_COUNTERMEASURE})

# option to use the system library libxml2-dev
option(SYSTEM_LIB "Use the system library libxml2" ${SYSTEM_LIB})

# option to use a manual installed library libxml2-dev
option(DEV_LIB "Use the manual installed library libxml2" ${DEV_LIB})

# option to build the integration test
option(INTG_TEST "Build the integration test" ${INTG_TEST})

# option to build the unitary test
option(UNIT_TEST "Build the unitary test" ${UNIT_TEST})

# option to use a wellformed call context
option(ADAPTED_CALL_CONTEXT "Use a wellformed call context" ${ADAPTED_CALL_CONTEXT})

# option to set the XML_MEM_TRACE environment variable
option(USE_XML_MEM_TRACE "Option to set the XML_MEM_TRACE environment variable" ${USE_XML_MEM_TRACE})

# option to set the XML_MEM_BREAKPOINT environment variable
option(USE_XML_MEM_BREAKPOINT "Option to set the XML_MEM_BREAKPOINT environment variable" ${USE_XML_MEM_BREAKPOINT})

# option to activate Frama-C analysis
option(FRAMA_C "Option to activate Frama-C VA analysis" ${FRAMA_C})

# option to widen Frama-C analysis
option(FRAMA_C_VA_WIDENING "Option to widen Frama-C VA analysis" ${FRAMA_C_VA_WIDENING})

# option to stub the libxml2 library
option(LIBXML2_STUB "Option to stub the libxml2 library" ${LIBXML2_STUB})

# save temporary files when needed
option(SAVE_TEMPS "Save temporary files" ${SAVE_TEMPS})

configure_file (
  "validate_saml_sign.h.in"
  "../validate_saml_sign.h"
  )

if(SAVE_TEMPS)
  set(CMAKE_C_FLAGS   "\${CMAKE_C_FLAGS}   -save-temps " )
  set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -save-temps " )
endif()

if(FRAMA_C)
  if(FRAMA_C_VA_WIDENING)
    set(CMAKE_C_FLAGS    "\${CMAKE_C_FLAGS}   -C -DFRAMA_C -DFRAMA_C_VA_WIDENING -D__FC_MACHDEP_X86_64") # -DFRAMA_C_MALLOC_INDIVIDUAL")
    set(CMAKE_CXX_FLAGS  "\${CMAKE_CXX_FLAGS} -C -DFRAMA_C -DFRAMA_C_VA_WIDENING -D__FC_MACHDEP_X86_64") # -DFRAMA_C_MALLOC_INDIVIDUAL")
  else()
    set(CMAKE_C_FLAGS    "\${CMAKE_C_FLAGS}   -C -DFRAMA_C -D__FC_MACHDEP_X86_64") # -DFRAMA_C_MALLOC_INDIVIDUAL")
    set(CMAKE_CXX_FLAGS  "\${CMAKE_CXX_FLAGS} -C -DFRAMA_C -D__FC_MACHDEP_X86_64") # -DFRAMA_C_MALLOC_INDIVIDUAL")
  endif()
endif()

# add subdirectories when needed
#add_subdirectory(data)

# add path to header files
#include_directories(".")
if(FRAMA_C)
  include_directories(${FRAMA_C_LIBC_DIR})
endif()
if(DEV_LIB)
  include_directories(${LIBXML2_DEV_INCLUDES_DIR})
endif()
if(SYSTEM_LIB)
  include_directories(${LIBXML2_SYS_INCLUDES_DIR})
endif()
if(LIBXML2_STUB)
  include_directories(${LIBXML2_DEV_INCLUDES_DIR})
endif()

# define the build target depending on the configuration

if(LIBXML2_STUB)
  # add_executable(xsw_test_saml_sign validate_saml_sign.c libxml2_empty_stub.c test_saml_sign.ut.c)
  # add_executable(xsw_test_saml_sign validate_saml_sign.c libxml2_stub.c test_saml_sign.ut.c)
  add_executable(
    xsw_test_saml_sign
    validate_saml_sign.c
    stub/buf.c
    stub/catalog.c
    stub/dict.c
    stub/entities.c
    stub/error.c
    stub/globals.c
    stub/hash.c
    stub/list.c
    stub/parser.c
    stub/parserInternals.c
    stub/threads.c
    stub/tree.c
    stub/uri.c
    stub/valid.c
    stub/xmlmemory.c
    stub/xmlstring.c
    stub/SAX.c
    stub/SAX2.c
    stub/setgetenv.c
    ${TEST_MAIN_SRC_FILE})
elseif(FRAMA_C)
  if(INTG_TEST)
    add_executable(xsw_test_saml_sign parse_saml_response.c validate_saml_sign.c ${FRAMA_C_MODIFIED_FILES} ${TEST_MAIN_SRC_FILE})
  elseif(UNIT_TEST)
    add_executable(xsw_test_saml_sign validate_saml_sign.c ${FRAMA_C_MODIFIED_FILES} ${TEST_MAIN_SRC_FILE})
  endif()
else()
  if(INTG_TEST)
    add_executable(xsw_test_saml_sign parse_saml_response.c validate_saml_sign.c ${TEST_MAIN_SRC_FILE})
  elseif(UNIT_TEST)
    add_executable(xsw_test_saml_sign validate_saml_sign.c ${TEST_MAIN_SRC_FILE})
  endif()
endif()

# add path to external libraries
if (LIBXML2_STUB)
else()
  if(DEV_LIB)
    target_link_libraries(xsw_test_saml_sign ${LIBXML2_DEV_LIB_INSTALL_PATH})
  endif()
  if(SYSTEM_LIB)
    target_link_libraries(xsw_test_saml_sign ${LIBXML2_SYS_LIB_INSTALL_PATH})
  endif()
endif()

EOF
}
#option(USE_CXX_EXCEPTIONS "Enable C++ exception support" ${USE_CXX_EXCEPTIONS})

function cmake_build_execute()
{
    cd ${ici}
    cmake_build_generate
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    cmake ..
    make VERBOSE=yes
    cd ${ici}
}

function cmake_build_clean()
{
    cd ${ici}
    rm -rf ${BUILD_DIR}
}

function cmake_callers_execute ()
{
    common=`which common.sh`
    launch_scan_build=`which launch_analysis.sh`

    source $common
    source $launch_scan_build

    cd ${ici}
    launch_the_analysis cmake callers ${BUILD_DIR}
}

# callers: Extract metrics
#  precondition: cmake_callers_execute
function cmake_callers_extract_metrics ()
{
    cd ${BUILD_DIR}
    list_files_in_dirs $callers_json_rootdir .file.callers.gen.json dir.callers.gen.json "analysis"
    extract_metrics metrics.callers.tmp.json
    ydump metrics.callers.tmp.json > metrics.callers.gen.json
    rm metrics.callers.tmp.json
    cd ${ici}
}
   
function fc_parse_prepare()
{
    source fc_analysis.sh
    cd ${ici}
    cd ${BUILD_DIR}

    if [ ${INTG_TEST} == "ON" ]; then
	fc_entrypoint="fc_va_entrypoint"
	it_fc_main_gen
    else
	fc_entrypoint="main"
    fi
    
    BUILD_PATH="${ici}/${BUILD_DIR}"
    fc_parse ${TEST_MAIN_SRC_FILE} ${BUILD_PATH} ${EXTRA_DEV_LIBS_DIRS} &&
    # launch fc_parse
    source fc_parse_preproc_files.gen.sh &&
    # launch fc-va
    fc_va ${fc_entrypoint} ${FRAMA_C_SLEVEL}
    #fc_va ${fc_entrypoint} ${FRAMA_C_SLEVEL} > /dev/stdout > fc_va.gen.stdout 2> fc_va.gen.stderr
    #fc_va ${fc_entrypoint} ${FRAMA_C_SLEVEL} | tee fc_va.gen.stdout.stderr
}
