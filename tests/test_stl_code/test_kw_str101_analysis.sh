#!/bin/bash
set -x

test=$1

if [ -z $test ]
then
    echo "Usage: ./test_kw_str101_analysis.sh <test_name>"
else
    mkdir -p /analysis/structure101/tables/${test}
    kwstruct101 --tables-directory /analysis/klocwork/tables/${test} -H 10.0.2.15 -P 27000 --output /analysis/structure101/tables/${test}/${test}.kw
    #ltrace kwstruct101 --tables-directory /analysis/klocwork/tables/${test} -H 10.0.2.15 -P 27000 --output /analysis/structure101/tables/${test}/${test}.kw 2&> debug.ltrace
    #strace kwstruct101 --tables-directory /analysis/klocwork/tables/${test} -H 10.0.2.15 -P 27000 --output /analysis/structure101/tables/${test}/${test}.kw 2&>1 > strace.log
fi
