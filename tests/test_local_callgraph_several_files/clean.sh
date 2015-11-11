#!/bin/bash
set -x

rm -f *.gen.dot
find . -name "*.debug.json" -exec rm {} \;
find . -name "*.gen.json" -exec rm {} \;
find . -name "*.gen.json.gz" -exec rm {} \;
rm -rf analysis
#rm -rf build
rm -f defined_symbols.json

