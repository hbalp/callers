#!/bin/bash

rm -rf callers_analysis
#rm -f defined_symbols.json
rm -f *.gen.callgraph
find . -type f -name "*.gen.*" -exec rm -f {} \;
#rm -rf /tmp/callers/

