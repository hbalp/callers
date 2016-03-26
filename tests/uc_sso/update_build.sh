#!/bin/bash

function update_libtool_build ()
{
    set -x
    libtoolize --force
    aclocal
    autoheader
    automake --force-missing --add-missing
    autoconf
    set +x
}


