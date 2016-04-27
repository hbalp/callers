#!/bin/bash

#canonical_pwd="$PWD"
canonical_pwd="/net/alpha.sc2.theresis.org$PWD"

common=`which common.sh`

source $common

## generate callee's tree from main entry point
source extract_fcg.sh callees ${canonical_pwd}/test_saml_sign.cpp "main" "int main()" files

# ## generate caller's tree from main entry point
# #source extract_fcg.sh callers `pwd`/test_saml_sign.cpp "main" "int main()" files

source callgraph_to_ecore.sh $callers_json_rootdir
source callgraph_to_dot.sh $callers_json_rootdir files

source process_dot_files.sh . analysis/${analysis_type}

source indent_jsonfiles.sh .
source indent_jsonfiles.sh $callers_json_rootdir

inkscape analysis/${analysis_type}/svg/main.fcg.callees.gen.dot.svg
#inkscape analysis/${analysis_type}/svg/main.fcg.callers.gen.dot.svg
