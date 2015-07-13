#!/bin/sh
file=$1
../../../../../../../build/bin/callers -I/usr/lib/gcc/x86_64-linux-gnu/4.8/include -I/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/c++/4.8 -I/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/c++/4.8/x86_64-linux-gnu -I/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/c++/4.8/backward -I/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/x86_64-linux-gnu/c++/4.8 -I/usr/local/include $file -o ${file}.out

