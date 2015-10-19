#!/bin/bash

file=$1
leaf=$2
node=$3

echo "dot: $file"
echo "leaf: $leaf"
echo "node: $node"

# retrieve the label of the current leaf
ret=`egrep "${node}.*\[" ${file}.${leaf}.gen.callees.dot`
echo "ret is: ${ret}"

if [[ -z "$ret" ]]; then

    echo "Add node: $node"

    egrep "${node}.*\[" ${file} >> ${file}.${leaf}.gen.callees.dot

    # retrieve the direct callees of the current node
    egrep "\\->.*${node}" ${file} | tee .tmp.gen.callees.${node}.dot | awk '{print $1}' | grep -v $node > .tmp.gen.callees

    cat .tmp.gen.callees.${node}.dot >> ${file}.${leaf}.gen.callees.dot
    rm .tmp.gen.callees.${node}.dot

    # get their own callees
    for c in `cat .tmp.gen.callees`;
    do
	echo "callee: $c"
	filter_callees.sh $file $leaf $c
    done

else
    echo "Skip existing node: $node..."
fi





