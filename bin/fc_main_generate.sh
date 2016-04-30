#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

cmd_args=$@

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
# File generated by generate_fc_main.sh

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
printf "  int argc=%d\n", argc
printf "  char* argv[] = {\n"

# naive printing of the program arguments
# printf "argv[0]=%s\n", argv[0]
# printf "argv[1]=%s\n", argv[1]

# generic printing of the program arguments
# p *argv@argc

# or

# generic pretty printing the program arguments
set \$arg=0
while (\$arg < argc)
# printf "argv[%d]=%s\n", $arg, argv[\$arg++]
printf "    \"%s\"\n", argv[\$arg++]
end
printf "  }\n"
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

function fc_main_gen ()
{
  cmd_args=$@
  fc_main_gen_args="fc_main.gen.args.c"
  fc_main_args_gdb_run $fc_main_gen_args $cmd_args

  cat > fc_main.gen.c <<EOF
/* File generated by generate_fc_main.sh */
int fc_main()
{
`cat fc_main.gen.args.c`
  main(argc, argv);
}
EOF
  rm ${fc_main_gen_args}
}

fc_main_gen ${cmd_args}
