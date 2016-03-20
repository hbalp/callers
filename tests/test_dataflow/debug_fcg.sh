#!/bin/bash

common=`which common.sh`

source $common

## generate callee's tree from main entry point
#source extract_fcg.sh callees `pwd`/test_dataflow.c "main" "int main()" files
source extract_fcg.sh callees `pwd`/test_dataflow.cpp "main" "int main()" files

# ## generate caller's tree from main entry point
# #source extract_fcg.sh callers `pwd`/test_dataflow.cpp "main" "int main()" files

source callgraph_to_ecore.sh $callers_json_rootdir
source callgraph_to_dot.sh $callers_json_rootdir files

source process_dot_files.sh . analysis/${analysis_type}

source indent_jsonfiles.sh .
source indent_jsonfiles.sh $callers_json_rootdir

inkscape analysis/${analysis_type}/svg/main.fcg.callees.gen.dot.svg
#inkscape analysis/${analysis_type}/svg/main.fcg.callers.gen.dot.svg