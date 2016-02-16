#!/bin/bash

if [ -z ${CC} ]
then
export CC=`which clang`
fi

${CC} -g -I/data/balp/src/tests/test_threading  -o test_threading.o   -c /data/balp/src/tests/test_threading/test_threading.c
${CC} test_threading.o -pthread -o test_threading

#${CC} -g -I/data/balp/src/tests/test_threading -pthread -o test_threading /data/balp/src/tests/test_threading/test_threading.c
