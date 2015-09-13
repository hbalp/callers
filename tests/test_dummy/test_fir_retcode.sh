#!/bin/bash
set -x
# This test enable to debug the fir retcode when it is activated in file prepare_frama_clang_analysis.sh at line 115
cp testCCompiler.c analysis/CMakeFiles/CMakeTmp

cd analysis
export CALLERS_ANALYSIS_TYPE=all

/tools/llvm/trunk/tools/clang/tools/extra/callers/bin/ccc-analyzer -o CMakeFiles/cmTryCompileExec695589015.dir/testCCompiler.c.o -c /tools/llvm/trunk/tools/clang/tools/extra/callers/tests/test_dummy/analysis/CMakeFiles/CMakeTmp/testCCompiler.c

source .testCCompiler.c.gen.analysis.launch.ccc-analyzer.sh
retcode=$?
if [ $retcode -ne 0 ]; then
    echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
    echo "ERROR:testons:FAILED to analyze the file: /tools/llvm/trunk/tools/clang/tools/extra/callers/tests/test_dummy/analysis/CMakeFiles/CMakeTmp/testCCompiler.c" 
    echo "retcode: $retcode"
    echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
fi
