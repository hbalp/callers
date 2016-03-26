#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

SSO_SP_SRC_DIR=/data/balp/src/stance/uc_sso/sp
OPENSSL_ARCH_NAME=openssl-OpenSSL_1_0_2f
OPENSSL_ARCH_PATH=${SSO_SP_SRC_DIR}/${OPENSSL_ARCH_NAME}

function openssl_distclean ()
{
    echo "openssl distclean..."
    cd ${SSO_SP_SRC_DIR}
    rm -rf ${OPENSSL_ARCH_NAME}
}

function openssl_install ()
{
    echo "openssl install..."
    cd ${SSO_SP_SRC_DIR}
    tar -zxf ../.svg/${OPENSSL_ARCH_NAME}.tar.gz
}

function openssl_clean ()
{
    echo "openssl clean..."
    cd ${OPENSSL_ARCH_PATH}
    make clean > /dev/null 2> /dev/null # Clean the current configuration away
    #make clean > .openssl.gen.clean.config.stdout # 2> .openssl.clean.config.stderr
}

function openssl_config ()
{
    echo "openssl config..."
    cd ${OPENSSL_ARCH_PATH}
    #mkdir ../${OPENSSL_BUILD_DIR}
    #cd ../${OPENSSL_BUILD_DIR}
    #../${OPENSSL_DIR}/config --unified shared --prefix=/opt/stance
    ./Configure debug-linux-x86_64-clang shared --prefix=/opt/stance > .openssl.gen.callers.config.stdout # 2> .openssl.gen.callers.config.stderr
    #../${OPENSSL_ARCH_NAME}/Configure debug-linux-x86_64-callers shared --prefix=/opt/stance
}

function openssl_build ()
{
    echo "openssl build..."
    cd ${OPENSSL_ARCH_PATH}
    make
    make test
    # make install
}

function openssl_callers ()
{
    echo "openssl callers analysis..."
    export CALLERS_ANALYSIS_TYPE=callers
    cd ${OPENSSL_ARCH_PATH}
    scan-callers --use-analyzer `which clang` -o analysis make > .openssl.gen.callers.scan.stdout 2> .openssl.gen.callers.scan.stderr
    result=$?
    echo "retcode: ${result}"
}

function openssl_callers_analysis_workflow ()
{
    cd ${SSO_SP_SRC_DIR}
    openssl_distclean
    openssl_install
    openssl_clean
    openssl_config
    # time openssl_build
    time openssl_callers
    cd ${SSO_SP_SRC_DIR}
}

