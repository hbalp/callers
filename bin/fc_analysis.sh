#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

################################################################################
#           GENERATION OF FC MAIN ENTRY POINT                                  #
################################################################################
# You need to build the input source files with debug options

# some global definitions
fc_parsed_data="fc_parsed.gen.sav"

fc_shared_libc_pp_annot_macros="-DEOF=-1 -D__FC_FOPEN_MAX=512 -D__FC_MAX_OPEN_FILES=1024 -DF_DUPFD=1 -DF_GETFD=2 -DF_SETFD=3 -DF_GETFL=4 -DF_SETFL=5 -DF_GETLK=6 -DF_SETLK=7 -DF_SETLKW=8 -DF_GETOWN=9 -DF_SETOWN=10 -DO_CREAT=0x0200 -DFRAMA_C_MALLOC_INDIVIDUAL"
#"-D__FC_USE_BUILTIN__"
# WARNING: do not try to use the Frama_C_memcpy builtin function
# cf http://lists.gforge.inria.fr/pipermail/frama-c-discuss/2009-December/001644.html

FRAMA_C_SHARE_PATH=`frama-c -print-share-path`
FRAMA_C_LIBC_DIR="${FRAMA_C_SHARE_PATH}/libc"
# to be used when correct
#FRAMA_C_BUILTIN_FILES="${FRAMA_C_SHARE_PATH}/libc.c"
FRAMA_C_BUILTIN_FILES="${FRAMA_C_SHARE_PATH}/libc/stdlib.c ${FRAMA_C_SHARE_PATH}/libc/string.c ${FRAMA_C_SHARE_PATH}/libc/__fc_builtin.c"
# replaced by a local and modified libc.c 
#FRAMA_C_BUILTIN_FILES=""

# usage: fc_main_gen ${cmd_args}
function fc_main_gen ()
{
  cmd_args=$@
  fc_main_filename="fc_main.gen.i"
  fc_main_gen_args="fc_main.gen.args.c"
  fc_main_args_gdb_run $fc_main_gen_args $cmd_args

  cat > ${fc_main_filename} <<EOF
/* File generated by Callers's generate_fc_main.sh */
#define fc_entrypoint main
int fc_entrypoint(int argc, char **argv);

int fc_va_entrypoint()
{
`cat fc_main.gen.args.c`
  fc_entrypoint(argc, argv);
}
EOF
  rm ${fc_main_gen_args}
}

function fc_main_args_gdbinit_gen ()
{
  fc_main_gen_args=$1
  gdbinit_filename=$2
  program=$3
  shift
  shift
  shift
  cmd_args=$@
  cat > ${gdbinit_filename} <<EOF
# File generated by Callers's generate_fc_main.sh

shell rm -f ${fc_main_gen_args}
set pagination off
set logging file ${fc_main_gen_args}
set logging overwrite on
set logging overwrite off

# specific loading of executable
# file ${program}

# init of some local variables
# tbc

b main

commands
set logging on
printf "  int argc=%d;\n", argc
printf "  char* argv[] = {\n"

# naive printing of the program arguments
# printf "argv[0]=%s,\n", argv[0]
# printf "argv[1]=%s\n", argv[1]

# generic printing of the program arguments
# p *argv@argc

# or

# generic pretty printing the program arguments
set \$arg=0
while (\$arg < argc)
# printf "argv[%d]=%s\n", $arg, argv[\$arg++]
printf "    \"%s\",\n", argv[\$arg++]
end
printf "  };\n"
set logging off
c
end

r ${cmd_args}
quit
EOF
}

function fc_main_args_gdb_run ()
{
  fc_main_gen_args=$1
  shift
  program=$1
  cmd_args=$@
  gdbinit_filename="fc_main.gen.gdbinit"
  fc_main_args_gdbinit_gen ${fc_main_gen_args} ${gdbinit_filename} $cmd_args
  gdb ${program} -x ${gdbinit_filename}
  rm ${gdbinit_filename}
}

################################################################################
#                    PARSING OF PREPROCESSED FILES                             #
################################################################################

# This function renames the ".i" extension of gcc preprocessed src files by ".i.c"
# to enable a second preprocessing step of annotations by frama-c
function fc_rename_preproc_files ()
{
    preproc_dirs=$@
    # list preprocessed files in each preproc dir
    for preproc_dir in ${preproc_dirs};
    do
	fc_main_entryppoint_files=`find ${preproc_dir} -name "*.i"`
	# rename all preprocessed files
	for f in ${fc_main_entryppoint_files};
	do
	    mv $f $f.c
	done
    done
}

# This function generates a script for parsing with frama-c some preprocessed src files
function fc_parse ()
{
    target_main_entrypoint=$1
    if [ -z $target_main_entrypoint ]; then
	echo "fc_parse usage error: expected at least one argument"
	echo "fc_parse <target_main_entrypoint> <fc_preproc_dirs>"
	return 17
    fi
    shift
    fc_preproc_dirs=$@
    echo "HHHHHHHHHHHHHHHHHHHHHHHHHHHH: fc_rename_preproc_files ${fc_preproc_dirs}" > /dev/stderr
    fc_rename_preproc_files ${fc_preproc_dirs}
    fc_filter_files_with_main_entrypoints "fc_entrypoints.gen.preproc_files" ${target_main_entrypoint} ${fc_preproc_dirs}
    fc_select_preproc_files ${fc_preproc_dirs}
}

# you should first call function fc_filter_files_with_main_entrypoints
# to avoid conflicts between potential main entrypoint functions reported by frama-c during parsing (see below)
function fc_select_preproc_files ()
{
    fc_preproc_dirs=$@
    fc_parse_prepro_files="fc_parse_preproc_files.gen.sh"
    echo "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
    echo "WW: pwd=${PWD}"
    echo "WW: fc_parse_prepro_files = ${fc_parse_prepro_files} "
    echo "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
    cat > ${fc_parse_prepro_files} <<EOF
#!/bin/bash
# Generated by fc_parse.sh in dir ${curdir}

frama-c -machdep x86_64 -cpp-gnu-like -cpp-command 'gcc -C -E -I. -DFRAMA_C ${fc_shared_libc_pp_annot_macros}' \\
    ${FRAMA_C_BUILTIN_FILES} \\
EOF

    # list preprocessed files in each preproc dir
    for preproc_dir in ${fc_preproc_dirs};
    do
	#create the directory if not existing
	if [ ! -d ${preproc_dir} ]; then
	    echo "fc_analysis:fc_select_preproc_files:WARNING: unexisting preproc_dir=\"${preproc_dir}\", so we will create it"
	    mkdir -p ${preproc_dir}
	fi
	preproc_files=`find ${preproc_dir} -type f -name "*.i.c" | awk '{ print $1 " \\\" }'`
	#fc_parsed_data="fc_parsed.gen.sav"
	fc_parse_args="-pp-annot -save ${fc_parsed_data}"
	cat >> ${fc_parse_prepro_files} <<EOF
${preproc_files}
EOF
    done
    cat >> ${fc_parse_prepro_files} <<EOF
${fc_parse_args}
EOF
}

################################################################################
#           FILTERING OF PREPROCESSED FILES WITH MAIN ENTRYPOINTS              #
################################################################################

# filter all potential main entrypoint functions except the target one to avoid conflicts reported by frama-c during parsing
function fc_filter_files_with_main_entrypoints ()
{
  fc_filtered_entrypoints_preproc_files=$1
  target_main_entrypoint_srcfile=$2
  shift
  shift
  fc_preproc_dirs=$@
  target_main_entrypoint_basename=`basename ${target_main_entrypoint_srcfile} .c`
  curdir=${PWD}
  cat > ${fc_filtered_entrypoints_preproc_files} <<EOF
#!/bin/bash
# Potential main entrypoint files filtered by fc_parse.sh
EOF
  # list preprocessed files in each preproc dir
  for preproc_dir in ${fc_preproc_dirs};
  do
      # list preprocessed files containing "main(" word
      fc_main_entryppoint_files=`find ${preproc_dir} -name "*.i.c" -exec egrep -l "\bmain\(" {} \;`
      # rename preprocessed files containing "main(" word
      for f in ${fc_main_entryppoint_files};
      do
	  fb=`basename $f .i.c`
	  if [ $fb == ${target_main_entrypoint_basename} ]; then
	      echo "MAIN_ENTRY_POINT_FILE: $f" >> /dev/stderr
	  else
	      echo "WARNING: found main() entry point declaration in file $f, so we rename it into $f.main to avoid conflicts reported by frama-c during parsing" >> /dev/stderr
	      mv $f $f.main
	  fi
      done
      fc_renamed_main_entrypoint_files=`find ${preproc_dir} -name "*.i.c.main" | awk '{ print $1 " \\\" }' | sed -e s/\.main//g`
      cat >> ${fc_filtered_entrypoints_preproc_files} <<EOF
${fc_renamed_main_entrypoint_files}
EOF
  done
}

################################################################################
#                    FRAMA-C VALUE ANALYSIS                                    #
################################################################################

# usage: fc_va <main_entrypoint_fct> <slevel>
# 
# if end-user main entry point:
#   Unprecise begin of analysis using the user main entry_point function
#   and without knowing the program arguments
# if fc_main entrypoint generated by function "fc_main_gen" of script "fc_analysis.sh"
#   More precise begin of analysis using the fc_main entry_point function generated by fc_main_gen
#   that captures thanks to gdb the user-defined arguments
#
function fc_va ()
{
  main=$1
  slevel=$2
  if [ -z ${slevel} ]; then
      echo "fc_va usage: fc_va <slevel>"
  else
      fc_analyzis_stdout="fc_analysis.gen.stdout"
      #fc_analyzis_stdout="/dev/null"
      fc_analyzis_stderr="fc_analysis.gen.stderr"
      fc_analyzed_data="fc_analyzed.gen.sav"
      #fc_parsed_data="fc_parsed.gen.sav"
      #fc_va_builtins_args="-val-builtin assert:Frama_C_assert"
      fc_va_builtins_args="-val-builtin malloc:Frama_C_alloc_size"
      #fc_va_builtins_args="-val-builtin __FC_assert"
      #fc_va_builtins_args=""
      fc_va_optional_args="-rte-all"
      # WARNING: Cf frama-c/kernel/cmdline.ml: 
      # Wrong Error Report "ignoring source files specified on the command line while loading a global initial context"
      # when some parameters are found after a -load
      fc_va_cmd="time frama-c -val -main ${main} -slevel ${slevel} ${fc_va_optional_args} ${fc_va_builtins_args} -load ${fc_parsed_data} -save ${fc_analyzed_data}"
      echo "fc_va: ${fc_va_cmd}"
      echo "fc_analyzis_stdout=${fc_analyzis_stdout}"
      echo "fc_analyzis_stderr=${fc_analyzis_stderr}"
      ${fc_va_cmd} > ${fc_analyzis_stdout} 2> ${fc_analyzis_stderr}
      grep "warning: " ${fc_analysis_gen_stdout}
  fi
}
