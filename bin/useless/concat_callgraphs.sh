#!/bin/bash

echo "digraph full_callgraph {" > callgraph.gen.full.dot

for f in `ls dot/CallGraph-*.dot`
do
    echo "file: $f"
    cat $f | tail -n+2 | head -n-1 >> callgraph.gen.full.dot
done

echo "}" >> callgraph.gen.full.dot

