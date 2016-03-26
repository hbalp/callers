#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

SSO_SP_SRC_DIR=/data/balp/src/stance/uc_sso/sp
CPP_OPENSAML_ARCH_NAME=cpp-opensaml
CPP_OPENSAML_ARCH_PATH=${SSO_SP_SRC_DIR}/${CPP_OPENSAML_ARCH_NAME}

function cpp-opensaml_distclean ()
{
    echo "cpp-opensaml distclean..."
    cd ${SSO_SP_SRC_DIR}
    rm -rf ${CPP_OPENSAML_ARCH_NAME}
}

function cpp-opensaml_download ()
{
    echo "cpp-opensaml download..."
    cd ${SSO_SP_SRC_DIR}
    git clone https://git.shibboleth.net/git/cpp-opensaml
    tar -zcf ../.svg/${CPP_OPENSAML_ARCH_NAME}.tar.gz cpp-opensaml
}

function cpp-opensaml_install ()
{
    echo "cpp-opensaml install..."
    cd ${SSO_SP_SRC_DIR}
    tar -zxf ../.svg/${CPP_OPENSAML_ARCH_NAME}.tar.gz
}

function cpp-opensaml_clean ()
{
    echo "cpp-opensaml clean..."
    cd ${CPP_OPENSAML_ARCH_PATH}
    #make clean          # Clean the current configuration away
    make clean > /dev/null 2> /dev/null # Clean the current configuration away
}

# Optional Packages:
#   --with-PACKAGE[=ARG]    use PACKAGE [ARG=yes]
#   --without-PACKAGE       do not use PACKAGE (same as --with-PACKAGE=no)
#   --with-pic[=PKGS]       try to use only PIC/non-PIC objects [default=use
#                           both]
#   --with-gnu-ld           assume the C compiler uses GNU ld [default=no]
#   --with-sysroot=DIR Search for dependent libraries within DIR
#                         (or the compiler's sysroot if not specified).
#   --with-zlib=PATH        where zlib is installed
#   --with-openssl=PATH     where openssl is installed
#   --with-boost=DIR        prefix of Boost [guess]
#   --with-log4shib=PATH    where log4shib-config is installed
#   --with-log4cpp=PATH     where log4cpp-config is installed
#   --with-xerces=PATH      where xerces-c is installed
#   --with-xmlsec=PATH      where xmlsec is installed
#   --with-xmltooling=PATH  where xmltooling is installed
#   --with-cxxtest=PATH     where cxxtest is installed
function cpp-opensaml_config ()
{
    echo "cpp-opensaml config..."
    cd ${CPP_OPENSAML_ARCH_PATH}
    update_libtool_build
    ./configure --enable-debug --with-xmltooling=/opt/stance --with-xmlsec=/opt/stance --with-xerces=/opt/stance --with-log4shib=/opt/stance --with-openssl=/opt/stance --prefix=/opt/stance > cpp-opensaml_build.gen.config.stdout 2> cpp-opensaml_build.gen.config.stderr
}

function cpp-opensaml_build ()
{
    echo "cpp-opensaml build..."
    cd ${CPP_OPENSAML_ARCH_PATH}
    make > cpp-opensaml_build.gen.make.stdout 2> cpp-opensaml_build.gen.make.stderr
    #make test > cpp-opensaml_build.gen.test.stdout 2> cpp-opensaml_build.gen.test.stderr
}

function cpp-opensaml_callers ()
{
    echo "cpp-opensaml callers analysis..."
    export CALLERS_ANALYSIS_TYPE=callers
    cd ${CPP_OPENSAML_ARCH_PATH}
    scan-callers --use-analyzer `which clang` -o analysis make > ../.cpp-opensaml_callers.gen.stdout 2> ../.cpp-opensaml_callers.gen.stderr
    result=$?
    echo "retcode: ${result}"
}

function cpp-opensaml_callers_analysis_workflow ()
{
    export CC=clang
    export CXX=clang++

    cd ${SSO_SP_SRC_DIR}
    cpp-opensaml_distclean
    # cpp-opensaml_download
    cpp-opensaml_install
    cpp-opensaml_clean
    cpp-opensaml_config
    # time cpp-opensaml_build
    time cpp-opensaml_callers
}
