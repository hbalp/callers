#!/bin/bash
set -x

rm -f *.gen.dot
find . -name "*.gen.json" -exec rm {} \;
#rm -rf svg
rm -rf analysis
rm -rf build
rm -f defined_symbols.json

