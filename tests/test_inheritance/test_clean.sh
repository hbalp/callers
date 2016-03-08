#!/bin/bash

rm -rf analysis
rm -f defined_symbols.json
rm -f *.fct.callees.gen.*
rm -f *.fcg.callees.gen.*
find . -type f -name "*.gen.json.gz" -exec rm -f {} \;
find . -type f -name "*.gen.json" -exec rm -f {} \;
find . -type f -name "*.gen.dot" -exec rm -f {} \;
#rm -rf /tmp/callers/
rm -rf build

