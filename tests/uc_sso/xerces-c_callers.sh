#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

SSO_SP_SRC_DIR=/data/balp/src/stance/uc_sso/sp
XERCES_C_ARCH_NAME=xerces-c-3.1.2
XERCES_C_ARCH_PATH=${SSO_SP_SRC_DIR}/${XERCES_C_ARCH_NAME}

function xerces-c_distclean ()
{
    echo "xerces-c distclean..."
    cd ${SSO_SP_SRC_DIR}
    rm -rf ${XERCES_C_ARCH_NAME}
}

function xerces-c_install ()
{
    echo "xerces-c install..."
    cd ${SSO_SP_SRC_DIR}
    tar -zxf ../.svg/${XERCES_C_ARCH_NAME}.tar.gz
}

function xerces-c_clean ()
{
    echo "xerces-c clean..."
    cd ${XERCES_C_ARCH_PATH}
    make clean > /dev/null 2> /dev/null # Clean the current configuration away
}

function xerces-c_config ()
{
    echo "xerces-c config..."
    cd ${XERCES_C_ARCH_PATH}
    update_libtool_build
    ./configure CFLAGS=-g CXXFLAGS=-g --with-curl=/opt/stance --prefix=/opt/stance  > .xerces-c_callers.gen.config.stdout 2> .xerces-c_callers.gen.config.stderr
    #../${XERCES_C_ARCH_NAME}/Configure debug-linux-x86_64-clang shared --prefix=/opt/stance > .xerces-c_callers.config.stdout # 2> .xerces-c_callers.config.stderr
    #../${XERCES_C_ARCH_NAME}/Configure debug-linux-x86_64-callers shared --prefix=/opt/stance
}

function xerces-c_build ()
{
    echo "xerces-c build..."
    cd ${XERCES_C_ARCH_PATH}
    make > .xerces-c_build.gen.make.stdout 2> .xerces-c_build.gen.make.stderr
    # make test > .xerces-c_build.gen.test.stdout 2> .xerces-c_build.gen.test.stderr
}

function xerces-c_callers ()
{
    echo "xerces-c callers analysis..."
    export CALLERS_ANALYSIS_TYPE=callers
    # export CALLERS_ANALYSIS_TYPE=all
    cd ${XERCES_C_ARCH_PATH}
    scan-callers --use-analyzer `which clang` -o analysis make > .xerces-c_callers.gen.stdout 2> .xerces-c_callers.gen.stderr
    result=$?
    echo "retcode: ${result}"
}

function xerces-c_callers_analysis_workflow ()
{
    export CC=clang
    export CXX=clang++

    cd ${SSO_SP_SRC_DIR}
    xerces-c_distclean
    xerces-c_install
    xerces-c_clean
    xerces-c_config
    # time xerces-c_build
    time xerces-c_callers
}

