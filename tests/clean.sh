#!/bin/bash
here=`pwd`

cd $here/test_dummy
source test_clean.sh

cd $here/test_external_callcycle
source test_clean.sh

cd $here/test_frama-clang
source clean.sh

cd $here/test_local_callcycle
source test_clean.sh

cd $here/test_local_callgraph_several_files
source clean.sh

cd $here/test_shared_header_impl
source test_clean.sh

cd $here
