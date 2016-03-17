#!/bin/bash

rm -rf install_dir
rm -f defined_symbols.json
rm -f *.gen.callgraph
find . -type d -name "analysis" -exec rm -rf {} \;
find . -type f -name "*.gen.json.gz" -exec rm -f {} \;
find . -type f -name "*.gen.json" -exec rm -f {} \;
find . -type f -name "*.gen.dot" -exec rm -f {} \;
#rm -rf /tmp/callers/

