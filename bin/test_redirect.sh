#!/bin/bash
#set -x

build_options="-DHAVE_CONFIG_H -DXERCES_BUILDING_LIBRARY -I. -I.. -I../src/xercesc/util -g -msse2 -MT xercesc/util/BinMemInputStream.lo -MD -MP -MF xercesc/util/.deps/BinMemInputStream.Tpo -c xercesc/util/BinMemInputStream.cpp -o xercesc/util/BinMemInputStream.o"

echo "args: ${build_options}"
echo "redirecion: "
echo $build_options | { args=$(< /dev/stdin); redirect_output_file.sh .cpp $args; }
