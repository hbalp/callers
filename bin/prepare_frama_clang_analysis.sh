#!/bin/bash
#set -x
# Copyright (C) 2015 Thales Communication & Security
#   - All Rights Reserved
# coded by Hugues Balp

source "common.sh" # get_file

function launch_frama_clang ()
{
    cpp_file=$1
    cabs_file=$2
    cabs_stderr=$3
    cabs_dir=`dirname $cabs_stderr`
    shift
    shift
    shift
    file_analysis_options="$@ -DFRAMA_CLANG"

    # localize frama-c
    frama_c=`which frama-c 2> /dev/null`

    # define frama-clang configuration options
    #frama_clang_options="-cxx-nostdinc -cxx-keep-mangling -fclang-msg-key clang,cabs -fclang-verbose 2 -machdep x86_64 -print -cxx-clang-command"
    #frama_clang_options="-machdep x86_64 -cxx-nostdinc -fclang-msg-key clang,cabs -cxx-clang-command" # for debug purposes
    frama_clang_options="-machdep x86_64 -cxx-nostdinc -val -lib-entry -rte -deps -print -ocode ${cabs_file} -cxx-clang-command"
    #frama_clang_options="-machdep x86_64 -cxx-nostdinc -print -ocode ${cabs_file} -cxx-clang-command"

    # add target source file specific analysis options
    frama_clang_analysis_options="${file_analysis_options}"

    framaCIRGen_options="-D__STRICT_ANSI__"

    # build the frama_clang analysis command
    # cpp_analysis="${frama_c} ${frama_clang_options} \"framaCIRGen \${system_includes} \${file_analysis_options}\" ${cpp_file} -cxx-keep-mangling -ocode ${cabs_file} -print"
    cpp_analysis="${frama_c} ${frama_clang_options} \"framaCIRGen ${framaCIRGen_options} \${system_includes} \${file_analysis_options}\" ${cpp_file} -cxx-keep-mangling > ${cabs_file}.stdout"

    # make sure the output directories are well created before calling the analysis ?

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch frama-clang analysis of file: ${cpp_file}\""
    echo "echo \"cppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcp\""
    echo "mkdir -p $cabs_dir"
    echo "touch $cabs_stderr"
    echo "file_analysis_options=\"${frama_clang_analysis_options}\""
    echo "#gdb --args "
    echo "#valgrind --tool=callgrind "
    echo "#valgrind "
    echo "${cpp_analysis}"
    #echo "if [ \$? -ne 0 ]; then" # WARNING: uncomment this line to really stop on Frama-clang frontend errors
    echo "if [ $? -ne 0 ]; then"   # WARNING: because by default, we do not stop on Frama-clang frontend errors
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $cabs_stderr"
    echo "    echo \"ERROR:launch_frama-clang:FAILED to analyze the file: $cpp_file\" >> $cabs_stderr"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $cabs_stderr"
    echo "    return 10"
    echo "fi"
    echo "gzip -f ${cabs_file}"
}

function launch_frama_c ()
{
    c_file=$1
    cabs_file=$2
    cabs_stderr=$3
    cabs_dir=`dirname $cabs_stderr`
    shift
    shift
    shift
    file_full_analysis_options="-I.. $@"

    # filter -g option unsupported by frama-c
    file_filtered_analysis_options=`echo ${file_full_analysis_options} | sed -e "s/ -g / /g"`

    # localize frama-c
    frama_c=`which frama-c 2> /dev/null`

    # define frama-c configuration options
    frama_c_options="-machdep x86_32 -no-cpp-gnu-like "

    # add target source file specific analysis options
    #frama_c_analysis_options="-cpp-extra-args=\\\"${file_filtered_analysis_options}\\\" -val -lib-entry -rte -deps -print -ocode ${cabs_file}"
    frama_c_analysis_options="-val -lib-entry -rte -deps -print -ocode ${cabs_file}"

    if [ "${file_filtered_analysis_options}" != "" ]
    then
        frama_c_file_args="-cpp-extra-args=\"${file_filtered_analysis_options}\""
    else
        frama_c_file_args=""
    fi

    # build the frama_c analysis command
    c_analysis="${frama_c} ${frama_c_options} ${frama_c_file_args} \${frama_c_analysis_options} ${c_file} > ${cabs_file}.stdout "
    # c_analysis="${frama_c} ${frama_c_options} \${frama_c_analysis_options} ${c_file} -ocode ${cabs_file} -print"

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch frama-c analysis of file: ${c_file}\""
    echo "echo \"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\""
    echo "mkdir -p $cabs_dir"
    echo "touch $cabs_stderr"
    echo "frama_c_analysis_options=\"${frama_c_analysis_options}\""
    echo "#gdb --args "
    echo "#valgrind --tool=callgrind "
    echo "#valgrind "
    echo "${c_analysis}"
    echo "if [ \$? -ne 0 ]; then"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $cabs_stderr"
    echo "    echo \"ERROR:launch_frama-c:FAILED to analyze the file: $c_file\" >> $cabs_stderr"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $cabs_stderr"
    echo "    return 11"
    echo "fi"
    echo "gzip -f ${cabs_file}"
}

function launch_framaCIRGen ()
{
    src_file=$1
    fir_file=$2
    fir_stderr=$3
    fir_dir=`dirname $fir_stderr`
    shift
    shift
    shift
    file_analysis_options=$@

    # localize frama-c
    framaCIRGen=`which framaCIRGen 2> /dev/null`

    # define frama-clang configuration options
    framaCIRGen_options="-D__STRICT_ANSI__"

    # add target source file specific analysis options
    framaCIRGen_analysis_options="${file_analysis_options}"

    # build the framaCIRGen analysis command
    fir_analysis="${framaCIRGen} ${framaCIRGen_options} \${system_includes} ${framaCIRGen_analysis_options} -o ${fir_file} ${src_file}"
    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch framaCIRGen analysis of file: ${src_file}\""
    echo "echo \"ccccppccccppccccppccccppccccppccccppccccppccccppccccppccccppccccppccccppccccppcc\""
    echo "mkdir -p $fir_dir"
    echo "touch $fir_stderr"
    echo "#gdb --args "
    echo "#valgrind --tool=callgrind "
    echo "#valgrind "
    echo "${fir_analysis}"
    echo "if [ $? -ne 0 ]; then" # WARNING: fir retcode deactivated by default because it breaks the normal cmake process
    #echo "if [ \$? -ne 0 ]; then" # to activate the fir retcode
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $fir_stderr"
    echo "    echo \"ERROR:launch_framaCIRGen:FAILED to analyze the file: $src_file\" >> $fir_stderr"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $fir_stderr"
    echo "    return 12"
    #echo "    exit 12"
    echo "fi"
    echo "gzip -f ${fir_file}"
}

function launch_gcc_cpp ()
{
    cpp_file=$1
    gcc_cpp_stdout_file=$2
    gcc_cpp_stderr_file=$3
    stderr_dir=`dirname $gcc_cpp_stderr_file`
    shift
    shift
    shift
    file_build_options=$@

    # localize gcc_cpp
    gcc_cpp=`which g++ 2> /dev/null`

    # add some options when required
    gcc_cpp_options="-I. -I.. -c"

    # build the gcc_cpp build command
    gcc_cpp_build="${gcc_cpp} ${debug_options} ${gcc_cpp_options} ${file_build_options} -o ${gcc_cpp_stdout_file} ${cpp_file}"

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch g++ build of file: ${cpp_file}\""
    echo "echo \"cppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcp\""
    echo "mkdir -p $stderr_dir"
    echo "touch $gcc_cpp_stderr_file"
    echo "#gdb --args "
    echo "#valgrind --tool=callgrind "
    echo "#valgrind "
    echo "${gcc_cpp_build}"
    echo "if [ \$? -ne 0 ]; then"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $gcc_cpp_stderr_file"
    echo "    echo \"ERROR:launch_g++:FAILED to build the file: $cpp_file\" >> $gcc_cpp_stderr_file"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $gcc_cpp_stderr_file"
    echo "    return 13"
    echo "fi"
    #echo "gzip -f ${gcc_cpp_stdout_file}"
}

function launch_gcc_c ()
{
    c_file=$1
    gcc_c_stdout_file=$2
    gcc_c_stderr_file=$3
    stderr_dir=`dirname $gcc_c_stderr_file`
    shift
    shift
    shift
    file_build_options=$@

    # localize gcc
    gcc_c=`which gcc 2> /dev/null`

    # add some options when required
    gcc_c_options="-I. -I.. -c"

    # build the gcc build command
    gcc_c_build="${gcc_c} ${debug_options} ${gcc_c_options} ${file_build_options} -o ${gcc_c_stdout_file} ${c_file}"

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch gcc build of file: ${c_file}\""
    echo "echo \"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\""
    echo "mkdir -p $stderr_dir"
    echo "touch $gcc_c_stderr_file"
    echo "#gdb --args "
    echo "#valgrind --tool=callgrind "
    echo "#valgrind "
    echo "${gcc_c_build}"
    echo "if [ \$? -ne 0 ]; then"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $gcc_c_stderr_file"
    echo "    echo \"ERROR:launch_gcc:FAILED to build the file: $c_file\" >> $gcc_c_stderr_file"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $gcc_c_stderr_file"
    echo "    return 14"
    echo "fi"
    #echo "gzip -f ${gcc_c_stdout_file}"
}

function launch_clang_cpp ()
{
    cpp_file=$1
    clang_cpp_stdout_file=$2
    clang_cpp_stderr_file=$3
    stderr_dir=`dirname $clang_cpp_stderr_file`
    shift
    shift
    shift
    file_build_options=$@

    # localize clang_cpp
    clang_cpp=`which clang++ 2> /dev/null`

    # add some options when required
    #clang_cpp_options="-std=c++11 -I. -I.."
    clang_cpp_build_options="-I. -I.. -c"

    # build the clang_cpp ast command
    #clang_cpp_ast_options="-I. -I.. -Xclang -ast-dump -fsyntax-only --disable-extern-template"
    clang_cpp_ast_options="-I. -I.. -Xclang -ast-dump -fsyntax-only "
    clang_cpp_astout_file="${clang_cpp_stdout_file}.ast"
    clang_cpp_ast="${clang_cpp} ${clang_cpp_ast_options} ${debug_options} ${file_build_options} \${system_includes} \${app_includes} ${cpp_file} > ${clang_cpp_astout_file}"

    # build the clang_cpp dump_cfg command
    clang_cpp_dump_cfg_options="-cc1 -analyze -analyzer-checker=debug.DumpCFG"
    clang_cpp_dump_cfg_file="${clang_cpp_stdout_file}.cfg.dump"
    clang_cpp_dump_cfg="${clang_cpp} ${clang_cpp_dump_cfg_options} ${file_build_options} \${system_includes} \${app_includes} ${cpp_file} 2> ${clang_cpp_dump_cfg_file}"

    # build the clang_cpp view_cfg command
    clang_cpp_view_cfg_options="-cc1 -analyze -analyzer-checker=debug.ViewCFG"
    clang_cpp_view_cfg_file="${clang_cpp_stdout_file}.cfg.view"
    clang_cpp_view_cfg1="${clang_cpp} ${clang_cpp_view_cfg_options} ${file_build_options} \${system_includes} \${app_includes} ${cpp_file}"
    #clang_cpp_view_cfg3="for f in `ls /tmp/CFG-*.dot`; do dot -Tsvg $f > $f.svg; done"
    clang_cpp_view_cfg2="tar -zcf ${clang_cpp_view_cfg_file}.tgz /tmp/CFG-*.dot; rm /tmp/CFG-*.*"
    #clang_cpp_view_cfg2="tar -zcf ${clang_cpp_view_cfg_file}.tgz /tmp/CFG-*.dot"

    # build the clang_cpp build command
    clang_cpp_build="${clang_cpp} ${debug_options} ${clang_cpp_build_options} ${file_build_options} \${system_includes} \${app_includes} -o ${clang_cpp_stdout_file} ${cpp_file}"

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch clang++ build of file: ${cpp_file}\""
    echo "echo \"cppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcp\""
    echo "mkdir -p $stderr_dir"
    echo "touch $clang_cpp_stderr_file"
    echo "#gdb --args "
    echo "#valgrind --tool=callgrind "
    echo "#valgrind "
    echo "app_includes=\"${file_build_options}\""
    echo "${clang_cpp_ast}"
    echo "# ${clang_cpp_dump_cfg}"
    echo "# ${clang_cpp_view_cfg1}"
    echo "# ${clang_cpp_view_cfg2}"
    echo "${clang_cpp_build}"
    echo "if [ \$? -ne 0 ]; then"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $clang_cpp_stderr_file"
    echo "    echo \"ERROR:launch_clang++:FAILED to build the file: $cpp_file\" >> $clang_cpp_stderr_file"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $clang_cpp_stderr_file"
    echo "    return 15"
    echo "fi"
    echo "gzip -f ${clang_cpp_astout_file}"
    echo "#gzip -f ${clang_cpp_stdout_file}"
}

function launch_clang_c ()
{
    c_file=$1
    clang_c_stdout_file=$2
    clang_c_stderr_file=$3
    stderr_dir=`dirname $clang_c_stderr_file`
    shift
    shift
    shift
    file_build_options=$@

    # localize clang
    clang_c=`which clang 2> /dev/null`

    # add some options when required
    clang_c_build_options="-I. -I.. -c"

    # build the clang ast command
    #clang_c_ast_options="-I. -I.. -Xclang -ast-dump -fsyntax-only --disable-extern-template"
    clang_c_ast_options="-I. -I.. -Xclang -ast-dump -fsyntax-only "
    clang_c_astout_file="${clang_c_stdout_file}.ast"
    clang_c_ast="${clang_c} ${debug_options} ${clang_c_ast_options} ${file_build_options} \${system_includes} \${app_includes} ${c_file} > ${clang_c_astout_file}"

    # build the clang_cpp dump_cfg command
    clang_c_dump_cfg_options="-cc1 -analyze -analyzer-checker=debug.DumpCFG"
    clang_c_dump_cfg_file="${clang_c_stdout_file}.cfg.dump"
    clang_c_dump_cfg="${clang_cpp} ${debug_options} ${clang_c_dump_cfg_options} ${file_build_options} \${system_includes} \${app_includes} ${cpp_file} 2> ${clang_c_dump_cfg_file}"

    # build the clang_cpp view_cfg command
    clang_c_view_cfg_options="-cc1 -analyze -analyzer-checker=debug.ViewCFG"
    clang_c_view_cfg_file="${clang_c_stdout_file}.cfg.view"
    clang_c_view_cfg1="${clang_cpp} ${debug_options} ${clang_c_view_cfg_options} ${file_build_options} \${system_includes} \${app_includes} ${cpp_file}"
    #clang_c_view_cfg3="for f in `ls /tmp/CFG-*.dot`; do dot -Tsvg $f > $f.svg; done"
    clang_c_view_cfg2="tar -zcf ${clang_c_view_cfg_file}.tgz /tmp/CFG-*.dot; rm /tmp/CFG-*.*"

    # build the clang build command
    clang_c_build="${clang_c} ${debug_options} ${clang_c_build_options} ${file_build_options} \${system_includes} \${app_includes} -o ${clang_c_stdout_file} ${c_file}"

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch clang build of file: ${c_file}\""
    echo "echo \"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\""
    echo "mkdir -p $stderr_dir"
    echo "touch $clang_c_stderr_file"
    echo "#gdb --args "
    echo "#valgrind --tool=callgrind "
    echo "#valgrind "
    echo "app_includes=\"${file_build_options}\""
    echo "${clang_c_ast}"
    echo "# ${clang_cpp_dump_cfg}"
    echo "# ${clang_cpp_view_cfg1}"
    echo "# ${clang_cpp_view_cfg2}"
    echo "${clang_c_build}"
    echo "if [ \$? -ne 0 ]; then"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $clang_c_stderr_file"
    echo "    echo \"ERROR:launch_clang:FAILED to build the file: $c_file\" >> $clang_c_stderr_file"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $clang_c_stderr_file"
    echo "    return 16"
    echo "fi"
    echo "gzip -f ${clang_c_astout_file}"
    echo "#gzip -f ${clang_c_stdout_file}"
}

function launch_callers_cpp ()
{
    cpp_file=$1
    callers_cpp_stdout_file=$2
    callers_cpp_stderr_file=$3
    stderr_dir=`dirname $callers_cpp_stderr_file`
    shift
    shift
    shift
    file_analysis_options=$@

    # localize callers
    callers_cpp=`which callers++ 2> /dev/null`

    # add some options when required
    #callers_cpp_options="-std=c++11 -I. -I.."
    #callers_cpp_options="-I. -I.. --disable-extern-template"
    callers_cpp_options="-I. -I.. "

    # build the callers analysis command
    callers_cpp_analysis="${callers_cpp} ${callers_cpp_options} \${system_includes} \${app_includes} -o ${callers_cpp_stdout_file} ${cpp_file}"

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch callers++ analysis of file: ${cpp_file}\""
    echo "echo \"cppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcp\""
    echo "mkdir -p ${stderr_dir}"
    echo "touch $callers_cpp_stderr_file"
    stderr_file_dir=`dirname ${callers_cpp_stderr_file}`
    echo "app_includes=\"${file_analysis_options}\""
    echo "#gdb --args "
    echo "#valgrind --tool=callgrind "
    echo "#valgrind "
    echo "${callers_cpp_analysis}"
    echo "if [ \$? -ne 0 ]; then"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $callers_cpp_stderr_file"
    echo "    echo \"ERROR:launch_callers++:FAILED to analyze the file: $cpp_file\" >> $callers_cpp_stderr_file"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $callers_cpp_stderr_file"
    echo "    return 17"
    echo "fi"
    echo "gzip -f ${callers_cpp_stdout_file}"
    #echo "gzip -f /tmp/callers\${file_dir}/${cpp_file}.file.callers.gen.json"
    # if [ ${stderr_file_dir} == "." ]
    # then
    #     echo "gzip -f /tmp/callers${cpp_file}.file.callers.gen.json"
    # else
    #     echo "gzip -f /tmp/callers${stderr_file_dir}/${cpp_file}.file.callers.gen.json"
    # fi
}

function launch_callers_c ()
{
    c_file=$1
    callers_c_stdout_file=$2
    callers_c_stderr_file=$3
    stderr_dir=`dirname $callers_c_stderr_file`
    shift
    shift
    shift
    file_analysis_options=$@

    # localize callers
    callers_c=`which callers 2> /dev/null`

    # add some options when required
    #callers_c_options="-I. -I.. --disable-extern-template"
    callers_c_options="-I. -I.. "

    # build the callers analysis command
    callers_c_analysis="${callers_c} ${callers_c_options} \${system_includes} \${app_includes} -o ${callers_c_stdout_file} ${c_file}"

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch callers analysis of file: ${c_file}\""
    echo "echo \"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\""
    echo "mkdir -p ${stderr_dir}"
    echo "touch $callers_c_stderr_file"
    #echo "file_dir=`dirname ${c_file}`"
    stderr_file_dir=`dirname ${callers_c_stderr_file}`
    echo "app_includes=\"${file_analysis_options}\""
    echo "#gdb --args "
    echo "#valgrind --tool=callgrind "
    echo "#valgrind "
    echo "${callers_c_analysis}"
    echo "if [ \$? -ne 0 ]; then"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $callers_c_stderr_file"
    echo "    echo \"ERROR:launch_callers:FAILED to analyze the file: $c_file\" >> $callers_c_stderr_file"
    echo "    echo \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\" >> $callers_c_stderr_file"
    echo "    return 18"
    echo "fi"
    echo "gzip -f ${callers_c_stdout_file}"
    # if [ ${stderr_file_dir} == "." ]
    # then
    #     echo "gzip -f /tmp/callers${c_file}.file.callers.gen.json"
    # else
    #     echo "gzip -f /tmp/callers${stderr_file_dir}/${c_file}.file.callers.gen.json"
    # fi
}

function prepare_frama_clang_analysis_from_compile_command()
{
    stderr_file=$1
    shift
    args=$@
    fileext="unknownFileExt"
    src_file="noSrcFile"

    c_file=`get_file .c ${args}`
    cpp_file=`get_file .cpp ${args}`
    obj_file=`get_file .o ${args}`

    if [ -z ${cpp_file} ]
    then
	if [ -n ${c_file} ]
	then
	    fileext="c"
	    src_file=${c_file}
	    echo "# c_file: ${c_file}"
	fi
    fi

    if [ -z ${c_file} ]
    then
	if [ -n ${cpp_file} ]
	then
	    fileext="cpp"
	    src_file=${cpp_file}
	    echo "# cpp_file: ${cpp_file}"
	fi
    fi

    if [ -z ${src_file} ]
    then
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"  >> $stderr_file
	echo "prepare_frama_clang_analysis::ERROR::Not Found .c or .cpp source file in args: ${args}" >> $stderr_file
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"  >> $stderr_file
	return 1
    fi

    if [ -z ${obj_file} ]
    then
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" >> $stderr_file
	echo "prepare_frama_clang_analysis::ERROR::Not Found obj source file in args: ${args}"  >> $stderr_file
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" >> $stderr_file
	return 1
    fi

    # echo "src_file: $src_file"
    # echo "obj_file: $obj_file"

    #gcc_stdout_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.gcc.out/g`
    gcc_stdout_file=${obj_file}
    clang_stdout_file=${obj_file}
    #clang_stdout_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.clang.out/g`
    callers_stdout_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.callers.stdout/g`
    cabs_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.cabs.c/g`
    fir_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.fir/g`

    # gcc_stderr_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.gcc.stderr/g`
    # clang_stderr_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.clang.stderr/g`
    # callers_stderr_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.callers.stderr/g`
    # cabs_stderr_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.cabs.stderr/g`
    # fir_stderr_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.fir.stderr/g`

    gcc_stderr_file=$stderr_file
    clang_stderr_file=$stderr_file
    callers_stderr_file=$stderr_file
    cabs_stderr_file=$stderr_file
    fir_stderr_file=$stderr_file

    debug="true"
    #debug="false"

    run_gcc="false"
    #run_gcc="true"
    run_clang="true"
    run_callers="false"
    run_frama_clang="false"
    run_framaCIRGen="false"

    if [ $debug == "true" ]
    then
	debug_options="-g"
    else
	debug_options=""
    fi

    # check whether this command is a link edition one ore not
    # precondition:
    # if the option -c is present, we consider it is a compilation command
    # else we consider it is a link edition command and we do not try to launch the callers analysis
    file_build_options=""
    is_build_command="false"

    # get source file's specific build options
    file_build_options=""
    for a in $args
    do
	if [ ${a} == -c ]
	then
	    is_build_command="true"
	fi
	if  [ ${a} != -c ]          &&
	    [ ${a} != -o ]          &&
	    [ ${a} != "-nostdinc" ] &&
	    [ ${a} != ${src_file} ] &&
	    [ ${a} != ${obj_file} ]
	then
	    file_build_options="${file_build_options} $a "
	fi
    done

    # get the analysis_type = callers | frama-clang | framaCIRGen | all
    analysis_type=${CALLERS_ANALYSIS_TYPE}
    #echo "c++-analysis type is: ${analysis_type}"

    # echo "DEBUG: is_build_command: ${is_build_command}" >> $stderr_file
    if [ ${is_build_command} == "true" ]
    then
        case $analysis_type in

	    "callers" )
	        #echo "activates callers analysis";
	        run_callers="true"
	        ;;

	    "frama-c" )
	        #echo "activates frama-c analysis";
	        run_frama_clang="true"
	        ;;

	    "frama-clang" )
	        #echo "activates frama-clang analysis";
	        run_frama_clang="true"
	        ;;

	    "framaCIRGen" )
	        #echo "activates framaCIRGen analysis";
	        run_framaCIRGen="true"
	        ;;

	    "all" )
	        #echo "activates all kind of analysis: callers, frama_clang and framaCIRGen";
	        #run_gcc="true"
	        run_clang="true"
	        run_callers="true"
	        run_frama_clang="true"
	        run_framaCIRGen="true"
	        ;;

	    *)
	        echo "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" >> $stderr_file
	        echo "WARNING: unknown analysis_type: $analysis_type" >> $stderr_file
	        echo "WARNING: builds the input file without any analysis..." >> $stderr_file
	        echo "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" >> $stderr_file
	        ;;
        esac
    else
	echo "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG" >> $stderr_file
	echo "prepare_frama_clang_analysis::IGNORE:: ignore this link edition step during ${analysis_type} analyzis : ${file_build_options}" >> $stderr_file
	echo "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG" >> $stderr_file
    fi

    if [ $fileext == "cpp" ]
    then
	if [ $run_gcc == "true" ]
	then
	    launch_gcc_cpp ${src_file} ${gcc_stdout_file} ${gcc_stderr_file} ${file_build_options}
	fi
	if [ $run_clang == "true" ]
	then
	    launch_clang_cpp ${src_file} ${clang_stdout_file} ${clang_stderr_file} ${file_build_options}
	fi
	if [ $run_callers == "true" ]
	then
	    launch_callers_cpp ${src_file} ${callers_stdout_file} ${callers_stderr_file} ${file_build_options}
	fi
	if [ $run_framaCIRGen == "true" ]
	then
	    launch_framaCIRGen ${src_file} ${fir_file} ${fir_stderr_file} ${file_build_options}
	fi
	if [ $run_frama_clang == "true" ]
	then
	    launch_frama_clang ${src_file} ${cabs_file} ${cabs_stderr_file} ${file_build_options}
	fi
    elif [ $fileext == "c" ]
    then
	if [ $run_gcc == "true" ]
	then
	    launch_gcc_c ${src_file} ${gcc_stdout_file} ${gcc_stderr_file} ${file_build_options}
	fi
	if [ $run_clang == "true" ]
	then
	    launch_clang_c ${src_file} ${clang_stdout_file} ${clang_stderr_file} ${file_build_options}
	fi
	if [ $run_callers == "true" ]
	then
	    launch_callers_c ${src_file} ${callers_stdout_file} ${callers_stderr_file} ${file_build_options}
	fi
	if [ $run_framaCIRGen == "true" ]
	then
	    launch_framaCIRGen ${src_file} ${fir_file} ${fir_stderr_file} ${file_build_options}
	fi
	if [ $run_frama_clang == "true" ]
	then
	    launch_frama_c ${src_file} ${cabs_file} ${cabs_stderr_file} ${file_build_options}
	fi
    else
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" >> $stderr_file
	echo "prepare_frama_clang_analysis::ERROR::internal error: unreachable state !" >> $stderr_file
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" >> $stderr_file
    fi
    #echo "# DEBUG: output file build options: ${file_build_options}"
}

function prepare_analysis_from_cmake_compile_commands()
{
    stderr_file=$1
    stderr_dir=`dirname $stderr_file`
    mkdir -p $stderr_dir
    touch $stderr_file
    compile_commands_json=$2

    echo "#!/bin/bash"
    echo "#set -x"

    # get the absolute path to the first file to be analyzed
    file=`grep \"file\" ${compile_commands_json} | tail -1 | cut -d '"' -f4`

    # get the system includes that are required o launch frama_clang
    system_includes $file

    # build the analysis command from the build one listed in file compile_commands.json
    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | while read command_line; do prepare_frama_clang_analysis_from_compile_command ${stderr_file} ${command_line}; done
}

function prepare_analysis_from_scan_build_command()
{
    stderr_file=$1
    stderr_dir=`dirname $stderr_file`
    mkdir -p $stderr_dir
    touch $stderr_file
    src_file=$2
    shift
    shift
    build_command=$@

    echo "#!/bin/bash"
    echo "#set -x"
    system_includes ${src_file}

    # prepare the analysis launch script from the build command
    prepare_frama_clang_analysis_from_compile_command ${stderr_file} ${build_command}
}
