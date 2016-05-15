#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

SSO_SP_SRC_DIR=/data/balp/src/stance/uc_sso/sp
CPP_LOG4SHIB_ARCH_NAME=cpp-log4shib
CPP_LOG4SHIB_ARCH_PATH=${SSO_SP_SRC_DIR}/${CPP_LOG4SHIB_ARCH_NAME}

function cpp-log4shib_distclean ()
{
    echo "cpp-log4shib distclean..."
    cd ${SSO_SP_SRC_DIR}
    rm -rf ${CPP_LOG4SHIB_ARCH_NAME}
}

function cpp-log4shib_download ()
{
    echo "cpp-log4shib download..."
    cd ${SSO_SP_SRC_DIR}
    git clone https://git.shibboleth.net/git/cpp-log4shib
    tar -zcf ../.svg/${CPP_LOG4SHIB_ARCH_NAME}.tar.gz cpp-log4shib
}

function cpp-log4shib_install ()
{
    echo "cpp-log4shib install..."
    cd ${SSO_SP_SRC_DIR}
    tar -zxf ../.svg/${CPP_LOG4SHIB_ARCH_NAME}.tar.gz
}

function cpp-log4shib_clean ()
{
    echo "cpp-log4shib clean..."
    cd ${CPP_LOG4SHIB_ARCH_NAME}
    make clean > /dev/null 2> /dev/null # Clean the current configuration away
}

function cpp-log4shib_config ()
{
    echo "cpp-log4shib config..."
    cd ${CPP_LOG4SHIB_ARCH_PATH}
    update_libtool_build
    ./configure --prefix=/opt/stance --enable-debug > cpp-log4shib_callers.gen.config.stdout 2> cpp-log4shib_callers.gen.config.stderr
}

function cpp-log4shib_build ()
{
    echo "cpp-log4shib build..."
    cd ${CPP_LOG4SHIB_ARCH_PATH}
    make > cpp-log4shib_build.gen.make.stdout 2> cpp-log4shib_build.gen.make.stderr
    #make test > cpp-log4shib_build.gen.test.stdout 2> cpp-log4shib_build.gen.test.stderr
}

function cpp-log4shib_callers ()
{
    echo "cpp-log4shib callers analysis..."
    export CALLERS_ANALYSIS_TYPE=callers
    cd ${CPP_LOG4SHIB_ARCH_PATH}
    scan-callers --use-analyzer `which clang` -o analysis make > ../.cpp-log4shib_callers.gen.stdout 2> ../.cpp-log4shib_callers.gen.stderr
    result=$?
    echo "retcode: ${result}"
}

function cpp-log4shib_callers_analysis_workflow ()
{
    export CC=clang
    export CXX=clang++

    cd ${SSO_SP_SRC_DIR}
    cpp-log4shib_distclean
    #cpp-log4shib_download
    cpp-log4shib_install
    cpp-log4shib_clean
    cpp-log4shib_config
    # time cpp-log4shib_build
    time cpp-log4shib_callers
    cd ${SSO_SP_SRC_DIR}
}
