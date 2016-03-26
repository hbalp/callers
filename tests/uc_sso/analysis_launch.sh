#!/bin/bash
#set -x
#     Copyright (C) 2015-2016 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

source clean_callers.sh
source update_build.sh

source openssl_callers.sh
# source openssl_fcg.sh
source curl_callers.sh
source xerces-c_callers.sh
source xml-security-c_callers.sh
source cpp-log4shib_callers.sh
source cpp-xmltooling_callers.sh
source cpp-opensaml_callers.sh
source cpp-sp_callers.sh
# source test_launch.sh

function launch_stance_sso_uc_analysis ()
{
  # clean the /tmp/callers repository
  echo "Are you sure you want to clean the /tmp/callers respository ? (y)"
  read
  echo "Launch STANCE SSO UC analysis..."
  # clean
  clean_callers
  # OpenSSL
  openssl_callers_analysis_workflow
  # curl
  curl_callers_analysis_workflow
  # xerces
  xerces-c_callers_analysis_workflow
  # xml-security-c
  xml-security-c_callers_analysis_workflow
  # cpp-log4shib
  cpp-log4shib_callers_analysis_workflow
  # cpp-xmltooling
  cpp-xmltooling_callers_analysis_workflow
  # opensaml
  cpp-opensaml_callers_analysis_workflow
  # shibboleth-sp
  cpp-sp_callers_analysis_workflow
}

function launch_stance_sso_uc_complete_analysis ()
{
  # clean the /tmp/callers repository
  echo "Complete STANCE SSO UC analysis..."
  # # clean
  # clean_callers
  # # OpenSSL
  # openssl_callers_analysis_workflow
  # # curl
  # curl_callers_analysis_workflow
  # # xerces
  # xerces-c_callers_analysis_workflow
  # # xml-security-c
  # xml-security-c_callers_analysis_workflow
  # # cpp-log4shib
  # cpp-log4shib_callers_analysis_workflow
  # cpp-xmltooling
  cpp-xmltooling_callers_analysis_workflow
  # opensaml
  cpp-opensaml_callers_analysis_workflow
  # shibboleth-sp
  cpp-sp_callers_analysis_workflow
}

launch_stance_sso_uc_analysis

#launch_stance_sso_uc_complete_analysis
