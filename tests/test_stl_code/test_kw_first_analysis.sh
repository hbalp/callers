#!/bin/bash
set -x

test=$1

if [ -z $test ]
then
    echo "Usage: ./test_kw_first_analysis.sh <test_name>"
else
    #rm -rf /analysis/klocwork/tables/${test}
    mkdir -p build
    cd build
    cmake ..
    make clean
    kwinject -o ${test}.kwinject make
    kwbuildproject --url http://localhost:8090/${test} --tables-directory /analysis/klocwork/tables/${test} ${test}.kwinject
    chown -R user /analysis/klocwork/tables/${test}
    chgrp -R user /analysis/klocwork/tables/${test}
    cd ..
fi
