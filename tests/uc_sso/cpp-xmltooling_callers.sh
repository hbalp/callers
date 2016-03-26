#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

SSO_SP_SRC_DIR=/data/balp/src/stance/uc_sso/sp
CPP_XML_TOOLING_ARCH_NAME=cpp-xmltooling
CPP_XML_TOOLING_ARCH_PATH=${SSO_SP_SRC_DIR}/${CPP_XML_TOOLING_ARCH_NAME}

function cpp-xmltooling_distclean ()
{
    echo "cpp-xmltooling distclean..."
    cd ${SSO_SP_SRC_DIR}
    rm -rf ${CPP_XML_TOOLING_ARCH_NAME}
}

function cpp-xmltooling_download ()
{
    echo "cpp-xmltooling download..."
    cd ${SSO_SP_SRC_DIR}
    git clone https://git.shibboleth.net/git/cpp-xmltooling
    tar -zcf ../.svg/${CPP_XML_TOOLING_ARCH_NAME}.tar.gz cpp-xmltooling
}

function cpp-xmltooling_install ()
{
    echo "cpp-xmltooling install..."
    cd ${SSO_SP_SRC_DIR}
    tar -zxf ../.svg/${CPP_XML_TOOLING_ARCH_NAME}.tar.gz
}

function cpp-xmltooling_clean ()
{
    echo "cpp-xmltooling clean..."
    cd ${CPP_XML_TOOLING_ARCH_PATH}
    make clean > /dev/null 2> /dev/null # Clean the current configuration away
}

# Try 'grep --help' for more information.
# balp@devrte:/data/balp/src/stance/uc_sso/sp/cpp-xmltooling$ ./configure --help | grep with
#   --disable-option-checking  ignore unrecognized --enable/--with options
#   --enable-debug          Have GCC compile with symbols (Default = no)
#   --with-PACKAGE[=ARG]    use PACKAGE [ARG=yes]
#   --without-PACKAGE       do not use PACKAGE (same as --with-PACKAGE=no)
#   --with-pic[=PKGS]       try to use only PIC/non-PIC objects [default=use
#   --with-gnu-ld           assume the C compiler uses GNU ld [default=no]
#   --with-sysroot=DIR Search for dependent libraries within DIR
#   --with-boost=DIR        prefix of Boost [guess]
#   --with-log4shib=PATH    where log4shib-config is installed
#   --with-log4cpp=PATH     where log4cpp-config is installed
#   --with-xerces=PATH      where xerces-c is installed
#   --with-xmlsec=PATH      where xmlsec is installed
#   --with-openssl=PATH     where openssl is installed
#   --with-curl=PATH        where curl-config is installed
#   --with-cxxtest=PATH     where cxxtest is installed
function cpp-xmltooling_config ()
{
    echo "cpp-xmltooling config..."
    cd ${CPP_XML_TOOLING_ARCH_PATH}
    update_libtool_build
    ./configure --enable-debug --with-curl=/opt/stance --with-xmlsec=/opt/stance --with-xerces=/opt/stance --with-log4shib=/opt/stance --with-openssl=/opt/stance --prefix=/opt/stance > cpp-xmltooling_build.gen.config.stdout 2> cpp-xmltooling_build.gen.config.stderr
}

function cpp-xmltooling_build ()
{
    echo "cpp-xmltooling build..."
    cd ${CPP_XML_TOOLING_ARCH_ARCH}
    make > cpp-xmltooling_build.gen.make.stdout 2> cpp-xmltooling_build.gen.make.stderr
    #make test > cpp-xmltooling_build.gen.test.stdout 2> cpp-xmltooling_build.gen.test.stderr
}

function cpp-xmltooling_callers ()
{
    echo "cpp-xmltooling callers analysis..."
    export CALLERS_ANALYSIS_TYPE=callers
    cd ${CPP_XML_TOOLING_ARCH_PATH}
    scan-callers --use-analyzer `which clang` -o analysis make > ../.cpp-xmltooling_callers.gen.stdout 2> ../.cpp-xmltooling_callers.gen.stderr
    result=$?
    echo "retcode: ${result}"
}

function cpp-xmltooling_callers_analysis_workflow ()
{
    export CC=clang
    export CXX=clang++

    cd ${SSO_SP_SRC_DIR}
    cpp-xmltooling_distclean
    # cpp-xmltooling_download
    cpp-xmltooling_install
    cpp-xmltooling_clean
    cpp-xmltooling_config
    # time cpp-xmltooling_build
    time cpp-xmltooling_callers
    cd ${SSO_SP_SRC_DIR}
}
