#!/bin/bash
# @author: Hugues Balp

################################################################################
#                     Loading of the required scripts
################################################################################

# help usage: usage_libxml2_install
# Usage of script libxml2_install.sh
# 3 features:
# 1) System install of libxml2:
#    > libxml2_workflow_system_install
# 2) Source install of libxml2:
#    > libxml2_workflow_sources_gdb <git|local>
# 3) Source Callers analysis of libxml2:
#    > libxml2_workflow_callers <git|local>
# 4) Frama-C builtin preprocessing of libxml2:
#    > libxml2_workflow_fc_va <git|local>
# 5) Frama-C update preprocessing of libxml2:
#    > libxml2_update_fc_preproc
   
source libxml2_install.sh

# test_saml_sign usage:
# either:
# build all config with: cmake_build_all
# or:
# 1: cmake_config_common
# 2: choose one of the cmake_config_* function
# 3: call script cmake_build_execute to generate and build the config
#    with the appropriate generated cmake file CMakeLists.txt

source test_saml_sign.launch.sh

################################################################################
#              To build and install all test and FC VA configuration
################################################################################

cmake_build_all

# To run each tests separately, look  at the details below:

################################################################################
#                     System install of libxml2 without debug symbols
################################################################################

# To install the libxml2 system library
libxml2_workflow_system_install

# To build the unitary test
cmake_config_common

cmake_config_it_gdb_lib-sys_with_xsw_countermeasure
cmake_build_execute

cmake_config_it_gdb_lib-sys_without_xsw_countermeasure
cmake_build_execute

# To execute the unitary test

# ... without the xsw countermeasure
./test_it_gdb_lib-sys_without_xsw_countermeasure.gen/xsw_test_saml_sign data/SAMLResponse.malicious_xsw.xml
./test_it_gdb_lib-sys_without_xsw_countermeasure.gen/xsw_test_saml_sign data/SAMLResponse.malicious_xsw.xml

# ... with the xsw countermeasure
./test_it_gdb_lib-sys_with_xsw_countermeasure.gen/xsw_test_saml_sign data/SAMLResponse.sane_xsw.xml
./test_it_gdb_lib-sys_with_xsw_countermeasure.gen/xsw_test_saml_sign data/SAMLResponse.malicious_xsw.xml

# To check potential memory vulnerabilities with valgrind
valgrind ./test_it_gdb_lib-sys.gen/xsw_test_saml_sign data/SAMLResponse.malicious_xsw.xml

# To get the execution path with valgrind
valgrind --tool=callgrind ./test_it_gdb_lib-sys.gen/xsw_test_saml_sign data/SAMLResponse.malicious_xsw.xml
kcachegrind callgrind.out.* &

################################################################################
#                     Build and Debug with gdb
################################################################################

# To build, fine tune configuration and install the libxml2 library without optimization and with debugging symbols
libxml2_workflow_sources_gdb local

# To build the unitary test
cmake_config_common

cmake_config_ut_gdb_lib-dev_without_xsw_countermeasure
cmake_build_execute

cmake_config_ut_gdb_lib-dev_with_xsw_countermeasure
cmake_build_execute

# To execute the unitary test
./test_ut_gdb_lib-dev_without_xsw_countermeasure.gen/xsw_test_saml_sign
./test_ut_gdb_lib-dev_with_xsw_countermeasure.gen/xsw_test_saml_sign

# To debug the unitary test with gdb
gdb --args ./test_ut_gdb_lib-dev_with_xsw_countermeasure.gen/xsw_test_saml_sign

################################################################################
#                     Frama-C Value Analysis
################################################################################

# To install and preprocess the sources of the libxml2 library
libxml2_workflow_fc_va local

# To modify some Frama-C VA parameters like the -slevel
# edit the parameters values in script test_saml_sign.launch.sh
emacs -nw test_saml_sign.launch.sh

# To Launch Frama-C Value Analysis on the unitary test
source test_saml_sign.launch.sh

# and then call
libxml2_workflow_fc_va local

# after successfull analysis completion, look at the results on the frama-c gui
frama-c-gui -load fc_analyzed.gen.sav &

# In case you need to edit some source files of the library,
# you should then update the preprocessing of libxml2 as follows:
[libxml2_update_fc_preproc]

