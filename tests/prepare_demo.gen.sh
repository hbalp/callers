#!/bin/bash
function usage_prepare_demo()
{
    cat > /dev/stdout <<EOF

usage_prepare_demo usage:
either:

  build all config with: 
     > prepare_all_tests
or 
  choose among the following configurations:

  1. Integration tests with or without countermeasure (with libxml2):
     > prepare_it_with_system_lib && execute_it_with_system_lib

  2. Unit tests with or without countermeasure:
     > prepare_ut_with_gdb &&
     > execute_ut_without_xsw_countermeasure && 
     > execute_ut_with_xsw_countermeasure ||
     > debug_ut_with_xsw_countermeasure

  3. Callers analysis with or without libxml2:
     > prepare_callers_with_libxml2
     > prepare_callers_without_libxml2

  4. Frama-c Value Analysis:
     > prepare_fc_va ||
     > update_fc_va
EOF
}

source libxml2_install.sh

source test_saml_sign.launch.sh

function prepare_all_tests()
{
  cmake_build_all
}

function prepare_it_with_system_lib()
{
    # To install the libxml2 system library
    libxml2_workflow_system_install

    # To build the unitary test
    cmake_config_common

    cmake_config_it_gdb_lib-sys_with_xsw_countermeasure
    cmake_build_execute

    cmake_config_it_gdb_lib-sys_without_xsw_countermeasure
    cmake_build_execute
}

function execute_it_with_system_lib()
{
    # ... without the xsw countermeasure
    ./test_it_gdb_lib-sys_without_xsw_countermeasure.gen/xsw_test_saml_sign data/SAMLResponse.sane.xml
    ./test_it_gdb_lib-sys_without_xsw_countermeasure.gen/xsw_test_saml_sign data/SAMLResponse.malicious_xsw.xml

    # ... with the xsw countermeasure
    ./test_it_gdb_lib-sys_with_xsw_countermeasure.gen/xsw_test_saml_sign data/SAMLResponse.sane.xml
    ./test_it_gdb_lib-sys_with_xsw_countermeasure.gen/xsw_test_saml_sign data/SAMLResponse.malicious_xsw.xml

    # To check potential memory vulnerabilities with valgrind
    valgrind ./test_it_gdb_lib-dev_with_xsw_countermeasure.gen/xsw_test_saml_sign data/SAMLResponse.malicious_xsw.xml

    # To get the execution path with valgrind
    valgrind --tool=callgrind ./test_it_gdb_lib-dev_with_xsw_countermeasure.gen/xsw_test_saml_sign data/SAMLResponse.malicious_xsw.xml
    kcachegrind callgrind.out.* &
}

function prepare_ut_with_gdb()
{
   # To build, fine tune configuration and install the libxml2 library without optimization and with debugging symbols
   libxml2_workflow_sources_gdb local

   # To build the unitary test
   cmake_config_common

   cmake_config_ut_gdb_lib-dev_without_xsw_countermeasure
   cmake_build_execute

   cmake_config_ut_gdb_lib-dev_with_xsw_countermeasure
   cmake_build_execute
}

function execute_ut_without_xsw_countermeasure()
{
   # To execute the unitary test
   ./test_ut_gdb_lib-dev_without_xsw_countermeasure.gen/xsw_test_saml_sign
}

function execute_ut_with_xsw_countermeasure()
{
   ./test_ut_gdb_lib-dev_with_xsw_countermeasure.gen/xsw_test_saml_sign
}

function debug_ut_with_xsw_countermeasure()
{
   # To debug the unitary test with gdb
   gdb --args ./test_ut_gdb_lib-dev_with_xsw_countermeasure.gen/xsw_test_saml_sign
}

function prepare_callers_without_libxml2()
{
   cmake_build_all_callers_without_libxml2
}

function prepare_callers_with_libxml2()
{
   cmake_build_all_callers_with_libxml2
}

function prepare_fc_va()
{
   # To Launch Frama-C Value Analysis on the unitary test
   # source test_saml_sign.launch.sh

   # and then call to install and preprocess the sources of the libxml2 library
   libxml2_workflow_fc_va local

   # frama-c Value Analysis
   fc_parse_prepare

   # after successfull analysis completion, look at the results on the frama-c gui
   frama-c-gui -load fc_analyzed.gen.sav &
}

function update_fc_va()
{
   libxml2_update_fc_preproc
}
