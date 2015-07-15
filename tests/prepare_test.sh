#!/bin/bash
file=$1

echo "#!/bin/sh
callers -DNOT_USE_BOOST -std=c++11 \\" > launch.gen.sh

# #strace -f -e verbose=all -s 256 -v `which clang` $file |& grep execve |& grep "bin/clang" |& grep cc1 | sed -e s/", "/"\n"/g > .debug.log
# #strace -f -e verbose=all -s 256 -v `which clang` $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g |& sed -e s/'"-internal-externc-isystem", "'/'-I"'/g |& sed -e s/", "/"\n"/g |& grep " -I\"" > .debug.log

# #strace -f -e verbose=all -s 256 -v `which clang` $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g  > .debug.log

echo "-I/user/local/include \\" >> launch.gen.sh

strace -f -e verbose=all -s 256 -v `which clang` -std=c++11 $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g |& sed -e s/'"-internal-externc-isystem", "'/'-I"'/g  |& sed -e s/", "/"\n"/g |& grep "\-I\"" |& sed -e s/"$"/" \\\\"/g >> launch.gen.sh

echo "${file} -o ${file}.gen.out" >> launch.gen.sh

chmod +x launch.gen.sh


