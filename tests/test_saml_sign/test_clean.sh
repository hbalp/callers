#!/bin/bash

#rm -rf build
rm -rf analysis
rm -f defined_symbols.json
rm -f *.gen.callgraph
rm -f *.gen.aird
find . -type f -name "*.gen.json.gz" -exec rm -f {} \;
find . -type f -name "*.gen.json" -exec rm -f {} \;
find . -type f -name "*.gen.dot" -exec rm -f {} \;
#rm -rf /tmp/callers/

