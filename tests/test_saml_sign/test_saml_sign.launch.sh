#!/bin/bash
# @author Hugues Balp
# This script manages both build and analysis of the test_saml_sign example

#
# Integration test
#

function cmake_config_it_gdb_lib-sys ()
{
    BUILD_DIR="it_gdb_lib-sys"
    SYSTEM_LIB="ON"
    DEV_LIB="OFF"
    INTG_TEST="ON"
    UNIT_TEST="OFF"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="OFF"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_it_fc-va_lib-sys ()
{
    BUILD_DIR="it_fc-va_lib-sys"
    SYSTEM_LIB="ON"
    DEV_LIB="OFF"
    INTG_TEST="ON"
    UNIT_TEST="OFF"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="ON"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_it_gdb_lib-dev ()
{
    BUILD_DIR="it_gdb_lib-dev"
    SYSTEM_LIB="OFF"
    DEV_LIB="ON"
    INTG_TEST="ON"
    UNIT_TEST="OFF"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="OFF"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_it_fc-va_lib-dev ()
{
    BUILD_DIR="it_fc-va_lib-dev"
    SYSTEM_LIB="OFF"
    DEV_LIB="ON"
    INTG_TEST="ON"
    UNIT_TEST="OFF"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="ON"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_it_gdb_stub ()
{
    BUILD_DIR="it_gdb_stub"
    SYSTEM_LIB="OFF"
    DEV_LIB="OFF"
    INTG_TEST="ON"
    UNIT_TEST="OFF"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="OFF"
    LIBXML2_STUB="ON"
    SAVE_TEMPS="ON"
}

function cmake_config_it_fc-va_stub ()
{
    BUILD_DIR="it_fc-va_stub"
    SYSTEM_LIB="OFF"
    DEV_LIB="OFF"
    INTG_TEST="ON"
    UNIT_TEST="OFF"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="ON"
    LIBXML2_STUB="ON"
    SAVE_TEMPS="ON"
}

#
# Unitary test
#

function cmake_config_ut_gdb_lib-sys ()
{
    BUILD_DIR="ut_gdb_lib-sys"
    SYSTEM_LIB="ON"
    DEV_LIB="OFF"
    INTG_TEST="OFF"
    UNIT_TEST="ON"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="OFF"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_ut_fc-va_lib-sys ()
{
    BUILD_DIR="ut_fc-va_lib-sys"
    SYSTEM_LIB="ON"
    DEV_LIB="OFF"
    INTG_TEST="OFF"
    UNIT_TEST="ON"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="ON"
    LIBXML2_STUB="OFF"
    SAVE_TEMPS="ON"
}

function cmake_config_ut_gdb_lib-dev ()
{
    BUILD_DIR="ut_gdb_lib-dev"
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

function cmake_config_ut_fc-va_lib-dev ()
{
    BUILD_DIR="ut_fc-va_lib-dev"
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

function cmake_config_ut_gdb_stub ()
{
    BUILD_DIR="ut_gdb_stub"
    SYSTEM_LIB="OFF"
    DEV_LIB="OFF"
    INTG_TEST="OFF"
    UNIT_TEST="ON"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="OFF"
    LIBXML2_STUB="ON"
    SAVE_TEMPS="ON"
}

function cmake_config_ut_fc-va_stub ()
{
    BUILD_DIR="ut_fc-va_stub"
    SYSTEM_LIB="OFF"
    DEV_LIB="OFF"
    INTG_TEST="OFF"
    UNIT_TEST="ON"
    ADAPTED_CALL_CONTEXT="ON"
    USE_XML_MEM_TRACE="ON"
    USE_XML_MEM_BREAKPOINT="ON"
    FRAMA_C="ON"
    LIBXML2_STUB="ON"
    SAVE_TEMPS="ON"
}

function build_cmake_generate ()
{
    cat > CMakeLists.txt <<EOF
# File generated by test_saml_sign.build.sh
# content manually adapted after a first generation by Callers's cmake_init.sh
cmake_minimum_required(VERSION 2.8)

# generate list of source files with related compilation command
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add debug symbols
set(CMAKE_BUILD_TYPE Debug)

#option(USE_CXX_EXCEPTIONS "Enable C++ exception support" ON)

# option to use the system library libxml2-dev
option(SYSTEM_LIB "Use the system library libxml2" ${SYSTEM_LIB})

# option to use a manual installed library libxml2-dev
option(DEV_LIB "Use the manual installed library libxml2" ON)

# option to build the integration test
option(INTG_TEST "Build the integration test" OFF)

# option to build the unitary test
option(UNIT_TEST "Build the unitary test" ON)

# option to use a wellformed call context
option(ADAPTED_CALL_CONTEXT "Use a wellformed call context" ON)

# option to activate Frama-C analysis
option(FRAMA_C "Option to activate Frama-C VA analysis" ON)

# option to stub the libxml2 library
option(LIBXML2_STUB "Option to stub the libxml2 library" OFF)

# save temporary files when needed
option(SAVE_TEMPS "Save temporary files" ON)

configure_file (
  "validate_saml_sign.h.in"
  "../validate_saml_sign.h"
  )

if(ADAPTED_CALL_CONTEXT)
  set(MANUAL_CALL_CONTEXT "try1" 1)
endif()

if(SAVE_TEMPS)
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -save-temps -C" )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps -C" )
endif()

if(FRAMA_C)
  set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS}   -DFRAMA_C -D__FC_MACHDEP_X86_64")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DFRAMA_C -D__FC_MACHDEP_X86_64")
endif()

# add subdirectories when needed
add_subdirectory(data)

# add path to header files
#include_directories(".")
if(FRAMA_C)
  # frama-c -print-share-path
  # include_directories("/home/hbalp/ocamlbrew/ocaml-4.02.3/.opam/system/share/frama-c/libc")
  include_directories("/opt/stance/share/frama-c/libc")
endif()
if(DEV_LIB)
  include_directories("/data/balp/src/tools/exec/include/libxml2")
  # include_directories("/tools/exec/include/libxml2")
endif()
if(SYSTEM_LIB)
  include_directories("/usr/include/libxml2")
endif()

# define by default a build target when source files are present
if(INTG_TEST)
  add_executable(xsw_test_saml_sign parse_saml_response.c validate_saml_sign.c test_saml_sign.it.c)
endif()
if(UNIT_TEST)
  #  add_executable(xsw_test_saml_sign validate_saml_sign.c test_saml_sign.ut.c)
  if(LIBXML2_STUB)
    include_directories("/home/hbalp/hugues/work/third_parties/src/libxml2")
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
      test_saml_sign.ut.c)
  else()
    if(FRAMA_C)
      add_executable(xsw_test_saml_sign validate_saml_sign.c stub/tree.c test_saml_sign.ut.c)
    else()
      add_executable(xsw_test_saml_sign validate_saml_sign.c test_saml_sign.ut.c)
    endif()
  endif()
endif()

# add path to external libraries
if (LIBXML2_STUB)
else()
  if(DEV_LIB)
    target_link_libraries(xsw_test_saml_sign /data/balp/src/tools/exec/lib/libxml2.so)
    # target_link_libraries(xsw_test_saml_sign /tools/exec/lib/libxml2.so)
  endif()
  if(SYSTEM_LIB)
    target_link_libraries(xsw_test_saml_sign /usr/lib/x86_64-linux-gnu/libxml2.so)
  endif()
endif()

EOF
}

function build_cmake_workflow ()
{
  # cmake_config_ut_gdb_lib-dev
  build_cmake_generate
}

