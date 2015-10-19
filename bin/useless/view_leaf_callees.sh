#!/bin/bash

file=$1
leaf=$2

echo "dot file: $file"
echo "leaf name: $leaf"

nodes=`grep $leaf $1 | awk '{print $1}'`

for n in $nodes
do
    echo "node: $n"
    view_node_callees.sh $file $n
done
