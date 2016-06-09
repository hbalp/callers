
clang_path=`which clang`
clang_bindir=`dirname $clang_path`
cp ../scan-callers $clang_bindir

## obscolete
# cd /tools/exec/bin
# git diff --no-prefix scan-build > ~/scan-build.patch
# patch -p0 < ~/scan-build.patch
