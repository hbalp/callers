#!/bin/bash
set -x

rm -f *.gen.callgraph
rm -f *.gen.dot
rm -f .debug_launch.std*
find . -name "*.debug.json" -exec rm {} \;
find . -name "*.gen.*" -exec rm {} \;
rm -rf analysis
rm -rf build
rm -f defined_symbols.json
rm -rf /tmp/callers
# when using script test_build.sh
rm -f .tmp.gen.*
rm test_threading
rm test_threading.o
rm -rf callers
rm -f *.gen.*.gz
rm -f *~
