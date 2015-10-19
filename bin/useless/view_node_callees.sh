#!/bin/bash

file=$1
leaf=$2

echo "dot file: $file"
echo "leaf node: $leaf"

echo "digraph callees_${leaf} {" > ${file}.${leaf}.gen.callees.dot

filter_callees.sh $file $leaf $leaf

rm .tmp.gen.callees

echo "}" >> ${file}.${leaf}.gen.callees.dot
echo "" >> ${file}.${leaf}.gen.callees.dot
echo "// Local Variables:" >> ${file}.${leaf}.gen.callees.dot
echo "// compile-command: \"dot -Tsvg ${file}.${leaf}.gen.callees.dot > ${file}.${leaf}.gen.callees.svg\"" >> ${file}.${leaf}.gen.callees.dot
echo "// compile-command: \"dot -Tsvg ${file}.${leaf}.gen.callees.dot > ${file}.${leaf}.gen.callees.png\"" >> ${file}.${leaf}.gen.callees.dot
echo "// End:" >> ${file}.${leaf}.gen.callees.dot




