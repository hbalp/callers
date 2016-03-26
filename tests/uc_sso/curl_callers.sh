#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

SSO_SP_SRC_DIR=/data/balp/src/stance/uc_sso/sp
CURL_ROOT_NAME=curl-7.35.0
CURL_ARCH_NAME=curl_7.35.0.orig
CURL_ARCH_PATH=${SSO_SP_SRC_DIR}/${CURL_ROOT_NAME}

function curl_clean ()
{
    echo "curl clean..."
    cd ${SSO_SP_SRC_DIR}
    rm -rf ${CURL_ROOT_NAME}
}

function curl_install ()
{
    echo "curl install..."
    cd ${SSO_SP_SRC_DIR}
    tar -zxf ../.svg/${CURL_ARCH_NAME}.tar.gz
}

function curl_clean ()
{
    echo "curl clean..."
    cd ${CURL_ARCH_PATH}
    make clean > /dev/null 2> /dev/null # Clean the current configuration away
}

function curl_config ()
{
    echo "curl config..."
    cd ${CURL_ARCH_PATH}
    update_libtool_build
    ./configure --enable-debug --with-ssl=/opt/stance --prefix=/opt/stance  > .curl_build.gen.config.stdout 2> .curl_build.gen.config.stderr
}

function curl_build ()
{
    echo "curl build..."
    cd ${CURL_ARCH_PATH}
    make > .curl_build.gen.make.stdout 2> .curl_build.gen.make.stderr
    # make test > ../.curl_build.gen.test.stdout 2> ../.curl_build.gen.test.stderr
}

function curl_callers ()
{
    echo "curl callers analysis..."
    export CALLERS_ANALYSIS_TYPE=callers
    cd ${CURL_ARCH_PATH}
    scan-callers --use-analyzer `which clang` -o analysis make > ../.curl_callers.gen.stdout 2> ../.curl_callers.gen.stderr
    result=$?
    echo "retcode: ${result}"
}

function curl_callers_analysis_workflow ()
{
    export CC=clang
    export CXX=clang++

    cd ${SSO_SP_SRC_DIR}
    curl_distclean
    curl_install
    curl_clean
    curl_config
    # time curl_build
    time curl_callers
    cd ${SSO_SP_SRC_DIR}
}
