#!/bin/bash
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp
# WARNING: We assume here that only one input parameter is present and correspond to a valid directory.
dir=$1

echo "Try to indent all json files present in directory \"${dir}\""

for json in `find $dir -name "*.json"`
do
tmp="${json}.tmp"
echo "* indent json file: ${json}"
cp ${json} ${tmp}
ydump ${tmp} > ${json}
rm ${tmp}
done
