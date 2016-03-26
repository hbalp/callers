#!/bin/bash
#set -x
#     Copyright (C) 2015-2016 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

CALLERS_TMP_DIR=/tmp/callers

function clean_callers ()
{
    echo "clean callers..."
    rm -rf ${CALLERS_TMP_DIR}
}
