#!/bin/bash
#set -x

test=$1

if [ -z $test ]
then
    echo "Usage: ./test_kw_publish_table.sh <test_name>"
else
    kwadmin --url http://localhost:8090 load ${test} /analysis/klocwork/tables/${test}
fi
