#!/bin/bash
set -x

rm -f *.gen.callgraph
rm -f *.gen.dot
find . -name "*.gen.*" -exec rm {} \;
rm -rf analysis
rm -rf build
rm -f defined_symbols.json
rm -rf /tmp/callers
