#!/bin/bash

OPENSSL_ARCH_NAME=openssl-OpenSSL_1_0_2f
#OPENSSL_BUILD_DIR=_openssl-build_linux-debug_x86_64_clang

# rm -rf ${OPENSSL_ARCH_NAME}
# #rm -rf ${OPENSSL_BUILD_DIR}
# tar -zxvf ../.svg/${OPENSSL_ARCH_NAME}.tar.gz

cd ${OPENSSL_ARCH_NAME}
# make clean          # Clean the current configuration away

#mkdir ../${OPENSSL_BUILD_DIR}
#cd ../${OPENSSL_BUILD_DIR}
#../${OPENSSL_DIR}/config --unified shared --prefix=/opt/stance
#../${OPENSSL_ARCH_NAME}/Configure debug-linux-x86_64-clang shared --prefix=/opt/stance

export CALLERS_ANALYSIS_TYPE=callers

../${OPENSSL_ARCH_NAME}/Configure debug-linux-x86_64-clang shared --prefix=/opt/stance

scan-callers --use-analyzer `which clang` -o analysis make

#make test
