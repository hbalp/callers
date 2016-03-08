#!/bin/bash
set -x

rm -f *.gen.callgraph
rm -f *.gen.dot
find . -name "*.debug.json" -exec rm {} \;
find . -name "*.gen.json" -exec rm {} \;
find . -name "*.gen.aird" -exec rm {} \;
find . -name "*.gen.callgraph" -exec rm {} \;
find . -name "*.gen.json.gz" -exec rm {} \;
rm -rf analysis
rm -rf build
rm -f defined_symbols.json
#rm -rf /tmp/callers/
