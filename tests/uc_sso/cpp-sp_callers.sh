#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

SSO_SP_SRC_DIR=/data/balp/src/stance/uc_sso/sp
CPP_SP_ARCH_NAME=cpp-sp
CPP_SP_ARCH_PATH=${SSO_SP_SRC_DIR}/${CPP_SP_ARCH_NAME}

function cpp-sp_distclean ()
{
    echo "cpp-sp distclean..."
    cd ${SSO_SP_SRC_DIR}
    rm -rf ${CPP_SP_ARCH_NAME}
}

function cpp-sp_download ()
{
    echo "cpp-sp download..."
    cd ${SSO_SP_SRC_DIR}
    git clone https://git.shibboleth.net/git/cpp-sp
    tar -zcf ../.svg/${CPP_SP_ARCH_NAME}.tar.gz cpp-sp
}

function cpp-sp_install ()
{
    echo "cpp-sp install..."
    cd ${SSO_SP_SRC_DIR}
    tar -zxf ../.svg/${CPP_SP_ARCH_NAME}.tar.gz
}

function cpp-sp_clean ()
{
    echo "cpp-sp clean..."
    cd ${CPP_SP_ARCH_PATH}
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
#   --with-saml=PATH        where opensaml is installed
#   --with-cxxtest=PATH     where cxxtest is installed
function cpp-sp_config ()
{
    echo "cpp-sp config..."
    cd ${CPP_SP_ARCH_PATH}
    update_libtool_build
    ./configure --enable-debug --with-saml=/opt/stance --with-xmltooling=/opt/stance --with-xmlsec=/opt/stance --with-xerces=/opt/stance --with-log4shib=/opt/stance --with-openssl=/opt/stance --prefix=/opt/stance > cpp-sp_build.gen.config.stdout 2> cpp-sp_build.gen.config.stderr
}

function cpp-sp_build ()
{
    echo "cpp-sp build..."
    cd ${CPP_SP_ARCH_PATH}
    make > cpp-sp_build.gen.make.stdout 2> cpp-sp_build.gen.make.stderr
    #make test > cpp-sp_build.gen.test.stdout 2> cpp-sp_build.gen.test.stderr
}

function cpp-sp_callers ()
{
    echo "cpp-sp callers analysis..."
    export CALLERS_ANALYSIS_TYPE=callers
    cd ${CPP_SP_ARCH_PATH}
    scan-callers --use-analyzer `which clang` -o analysis make > ../.cpp-sp_callers.gen.stdout 2> ../.cpp-sp_callers.gen.stderr
    result=$?
    echo "retcode: ${result}"
}

function cpp-sp_callers_analysis_workflow ()
{
    export CC=clang
    export CXX=clang++

    cd ${SSO_SP_SRC_DIR}
    cpp-sp_distclean
    # cpp-sp_download
    cpp-sp_install
    cpp-sp_clean
    cpp-sp_config
    # time cpp-sp_build
    time cpp-sp_callers
}
