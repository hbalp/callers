#!/bin/bash
#set -x
system_includes="-I/usr/lib/gcc/i486-linux-gnu/4.7/../../../../include/c++/4.7 -I/usr/lib/gcc/i486-linux-gnu/4.7/../../../../include/c++/4.7/i486-linux-gnu -I/usr/lib/gcc/i486-linux-gnu/4.7/../../../../include/c++/4.7/backward -I/usr/local/include -I/media/sf_users_3u/tools/llvm/build/bin/../lib/clang/3.7.0/include -I/usr/include/i386-linux-gnu -I/include -I/usr/include "
echo "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
echo "launch callers++ analysis of file: test_std_map.cpp"
echo "cppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcp"
mkdir -p analysis
touch analysis/.test_std_map.cpp.gen.analysis.launch.c++-analyzer.sh.stderr
#gdb --args 
callers++ -I. -I..  ${system_includes} -g -o debug.gen.stdout test_std_map.cpp
if [ $? -ne 0 ]; then
    echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" >> analysis/.test_std_map.cpp.gen.analysis.launch.c++-analyzer.sh.stderr
    echo "ERROR:launch_callers++:FAILED to analyze the file: test_std_map.cpp" >> analysis/.test_std_map.cpp.gen.analysis.launch.c++-analyzer.sh.stderr
    echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" >> analysis/.test_std_map.cpp.gen.analysis.launch.c++-analyzer.sh.stderr
    return 17
fi

