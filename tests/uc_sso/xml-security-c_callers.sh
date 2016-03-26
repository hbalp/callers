#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

SSO_SP_SRC_DIR=/data/balp/src/stance/uc_sso/sp
XML_SECURITY_C_ARCH_NAME=xml-security-c-1.7.3
XML_SECURITY_C_ARCH_PATH=${SSO_SP_SRC_DIR}/${XML_SECURITY_C_ARCH_NAME}

function xml-security-c_distclean ()
{
    echo "xml-security-c distclean..."
    cd ${SSO_SP_SRC_DIR}
    rm -rf ${XML_SECURITY_C_ARCH_NAME}
}

function xml-security-c_install ()
{
    echo "xml-security-c install..."
    cd ${SSO_SP_SRC_DIR}
    tar -zxf ../.svg/${XML_SECURITY_C_ARCH_NAME}.tar.gz
}

function xml-security-c_clean ()
{
    echo "xml-security-c clean..."
    cd ${XML_SECURITY_C_ARCH_PATH}
    make clean > /dev/null 2> /dev/null # Clean the current configuration away
}

function xml-security-c_config ()
{
    echo "xml-security-c config..."
    cd ${XML_SECURITY_C_ARCH_PATH}
    update_libtool_build
    ./configure --enable-debug --with-openssl=/opt/stance --with-xerces=/opt/stance --prefix=/opt/stance  > .xml-security-c_callers.gen.config.stdout 2> .xml-security-c_callers.gen.config.stderr
}

function xml-security-c_build ()
{
    echo "xml-security-c build..."
    cd ${XML_SECURITY_C_ARCH_PATH}
    make > .xml-security-c_build.gen.make.stdout 2> .xml-security-c_build.gen.make.stderr
    #make test > .xml-security-c_build.gen.test.stdout 2> .xml-security-c_build.gen.test.stderr
}

function xml-security-c_callers ()
{
    export CALLERS_ANALYSIS_TYPE=callers
    cd ${XML_SECURITY_C_ARCH_PATH}
    echo "xml-security-c callers analysis..."
    scan-callers --use-analyzer `which clang` -o analysis make > .xml-security-c_callers.gen.stdout 2> .xml-security-c_callers.gen.stderr
    result=$?
    echo "retcode: ${result}"
    #make test
}

function xml-security-c_callers_analysis_workflow ()
{
    export CC=clang
    export CXX=clang++

    cd ${SSO_SP_SRC_DIR}
    xml-security-c_distclean
    xml-security-c_install
    xml-security-c_clean
    xml-security-c_config
    # time xml-security-c_build
    time xml-security-c_callers
    cd ${SSO_SP_SRC_DIR}
}
