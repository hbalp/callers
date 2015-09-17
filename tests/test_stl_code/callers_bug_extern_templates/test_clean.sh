#!/bin/bash

rm -rf analysis
rm -f defined_symbols.json
rm -f debug.gen.stdout
find . -type f -name "*.gen.json.gz" -exec rm -f {} \;
find . -type f -name "*.gen.json" -exec rm -f {} \;
find . -type f -name "*.gen.dot" -exec rm -f {} \;
find /usr/include/c++/4.8 -type f -name "*.gen.json" -exec rm {} \;

