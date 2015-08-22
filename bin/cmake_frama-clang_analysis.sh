#!/bin/bash
set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

progname=$0
version=0.0.1

# func_usage
# outputs to stdout the --help usage message.
function func_usage ()
{
    provided_cmds=$@
    echo "################################################################################"
    echo "# shell script to launch the clang \"frama-clang\" analysis plugin"
    echo "# version $version"
    echo "################################################################################"
    echo "# Usage:"
    echo "# cmake_frama-clang_analysis.sh <cmake_compile_commands.json> (all|<specific_file>) <frama-clang_analysis_report_dir>"
    echo "# Provided command was: ${provided_cmds}"
    exit -1
}

# func_version
# outputs to stdout the --version message.
function func_version ()
{
    echo "################################################################################"
    echo "# clang frama-clang plugin v$version"
    echo "# located at $progname"
    echo "# Copyright (C) 2015 Thales Communication & Security"
    echo "# Written by Hugues Balp"
    echo "#  - All Rights Reserved"
    echo "# There is NO WARRANTY, to the extent permitted by law."
    echo "################################################################################"
    exit 0
}

frama_clang_launch_script=frama-clang.launch.gen.sh
framaCIRGen_launch_script=framaCIRGen.launch.gen.sh

# system_includes
# retrieve the system include files required by clang
function system_includes ()
{
    compile_commands_json=$1

    # get the absolute path to the first file to be analyzed
    file=`grep \"file\" ${compile_commands_json} | tail -1 | cut -d '"' -f4`
    clang=`which clang`
    
    system_includes=`strace -f -e verbose=all -s 256 -v ${clang} -std=c++11 $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g|& sed -e s/'"-internal-externc-isystem", "'/'-I"'/g |& sed -e s/", "/"\n"/g |& grep "\-I\"" | sed -e s/\"//g | awk '{print}' ORS=' ' `

    echo "system_includes: $system_includes" >&2

    echo "system_includes=\"$system_includes\""
}

function launch_framaCIRGen ()
{
    compile_commands_json=$1
    
    # make sure the output directories are well created before calling the analysis
    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e "s/.*-o //g" | awk '{ print $1 }' | sort -u | xargs dirname | awk '{ print "&& mkdir -p " $N " \\" }'

    # build the analysis command from the build one listed in file compile_commands.json
    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e s/^[^\ ]*/framaCIRGen\ \$\{system_includes\}/g | sed -e s/-c\ //g | sed -e s/\\.o\ /\.gen.fir\ /g | awk '{ print "&& " $N " \\" }'    
}

function launch_frama_clang ()
{
    compile_commands_json=$1
    
    # make sure the output directories are well created before calling the analysis
    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e "s/.*-o //g" | awk '{ print $1 }' | sort -u | xargs dirname | awk '{ print "&& mkdir -p " $N " \\" }'

    # build the analysis command from the build one listed in file compile_commands.json
    #cat $compile_commands_json | grep \"command\" | cut -d '"' -f4  | awk '{ print "&& " $1 " " $4 " " $2 " " $3 " \\" }'

    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e s/^[^\ ]*/\$\{frama_clang\}\ \"framaCIRGen\ \$\{system_includes\}\"\ -machdep\ x86_32\ -print/g | sed -e s/-c\ //g | sed -e s/-o\ /\>\ /g | sed -e s/\\.o\ /\.gen.cabs.c\ /g | awk '{ print "&& " $1 " " $2 " " $3 " " $4 " " $5 " " $6 " " $9 " " $7 " " $8 " \\" }'

    #cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e s/^[^\ ]*/\$\{frama_clang\}\ framaCIRGen \$\{system_includes\}\ -machdep\ x86_32\ -print/g | sed -e s/-c\ //g | sed -e s/-o\ /\>\ /g | sed -e s/\\.o\ /\.gen.cabs.c\ /g | awk '{ print "&& " $1 " " $4 " " $2 " " $3 " \\" }'

#    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e s/^[^\ ]*/\$\{frama_clang\}\ \"framaCIRGen \$\{system_includes\}\"\ -machdep x86_32 -print/g | sed -e s/-c\ //g | sed -e s/-o\ /\>\ /g | sed -e s/\\.o\ /\.gen.cabs.c\ /g | awk '{ print "&& " $1 $2 $3 $4  $5 " \\" }'

#    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e s/^[^\ ]*/\$\{frama_clang\}\ \"framaCIRGen \$\{system_includes\}\"\ -machdep x86_32 -print/g | sed -e s/-c\ //g | sed -e s/-o\ /\>\ /g | sed -e s/\\.o\ /\.gen.cabs.c\ /g | awk '{ print "&& " " :1: " $1 " :2: " $2 " :3: " $3 " :4: " $4 " :5: " $5 " :6: " $6 " :7: " $7 " :8: " $8 " :9: " $9 " :10: " $10 " \\" }'
    #echo "frama_clang=\"frama-c -cxx-nostdinc -cxx-keep-mangling -fclang-msg-key clang,cabs -fclang-verbose 2 -cxx-clang-command \"framaCIRGen ${system_includes}\" -machdep x86_32 -print \""
}

function dump_gdbinit ()
{
    compile_commands_json=$1

    # get the absolute path to the first file to be analyzed
    file=`grep \"file\" ${compile_commands_json} | tail -1 | cut -d '"' -f4`
    clang=`which clang`
    
    system_includes=`strace -f -e verbose=all -s 256 -v ${clang} -std=c++11 $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g|& sed -e s/'"-internal-externc-isystem", "'/'-I"'/g |& sed -e s/", "/"\n"/g |& grep "\-I\"" | sed -e s/\"//g | awk '{print}' ORS=' ' `

    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e s/^[^\ ]*//g | sed -e s/-c\ //g | sed -e s/\\.o\ /\.gen.fir\ /g > .build_cmds

    while read -r line
    do
	file=`echo $line | awk '{print $NF}' | xargs basename`
	printf "# $file\n"
	printf "# set args ${system_includes} $line\n"
	printf "\n"
    done < .build_cmds
    #rm .build_cmds
}

function launch_script_header ()
{
    analysis=$1
    compile_commands_json=$2
    echo "#!/bin/bash"
    echo "#set -x"
    system_includes $compile_commands_json

    if [ $analysis == "frama-clang" ]
    then
	echo "frama_clang=\"frama-c -cxx-nostdinc -cxx-keep-mangling -fclang-msg-key clang,cabs -fclang-verbose 2 -cxx-clang-command \""
    fi
    echo "# files to analyze: ${files}"
    echo "echo \"Begin function ${analysis} analysis...\" \\"
}

function launch_script_footer ()
{
    analysis=$1
    echo "&& echo"
    echo "echo \"End function ${analysis} analysis.\""
}

provided_cmds=$@

if test $# = 0; then
    func_usage $provided_cmds; 
    exit 0
    
elif test $# = 1; then

    case "$1" in
	--help | --hel | --he | --h )
	    func_usage; exit 0 ;;
	--version | --versio | --versi | --vers | --ver | --ve | --v )
	    func_version ;;
	*)
	    func_usage $provided_cmds;;
    esac

elif test $# = 3; then

    compile_commands_json=$1
    files=$2
    frama_clang_analysis_report=$3

    json_filename=`basename ${compile_commands_json}`
    
    case $json_filename in
	"compile_commands.json" )
	    echo "json_file: ${json_filename}";
	    launch_script_header frama-clang $compile_commands_json > $frama_clang_launch_script
	    launch_script_header framaCIRGen $compile_commands_json > $framaCIRGen_launch_script;;
	*)
	    func_usage $provided_cmds;;
    esac

    case $files in
	"all" )
	    echo "analyze all files...";

	    # prepare command arguments for framaCIRGen analysis
	    launch_framaCIRGen $compile_commands_json >> $framaCIRGen_launch_script

	    # prepare command arguments for frama-clang analysis
	    launch_frama_clang $compile_commands_json >> $frama_clang_launch_script

	    #frama-c -cxx-nostdinc -cxx-keep-mangling -fclang-msg-key clang,cabs -fclang-verbose 2 -cxx-clang-command "framaCIRGen ${INCLUDES}" -machdep x86_32 -print simple.cpp > simple.gen.fclang.cabs.c

	    # prepare command arguments for gdbinit script
	    gdb_launch_script=gdbinit
	    dump_gdbinit $compile_commands_json > $gdb_launch_script
	    ;;
	*)
	    echo "analyze file $files..."; 
	    # make sure the output directories are well created before calling the analysis
	    cat $compile_commands_json | grep \"command\" | grep $files | cut -d '"' -f4 | sed -e "s/.*-o //g" | awk '{ print $1 }' | sort -u | xargs dirname | awk '{ print "&& mkdir -p " $N " \\" }' >> $frama_clang_launch_script
	    # build the analysis command from the build one listed in file compile_commands.json
	    cat $compile_commands_json | grep \"command\" | grep $files | cut -d '"' -f4 | sed -e s/^[^\ ]*/framaCIRGen\ \$\{system_includes\}/g | sed -e s/-c\ //g | sed -e s/\\.o\ /\.gen.fir\ /g | awk '{ print "&& " $N " \\" }' >> $frama_clang_launch_script
	    # prepare command arguments for gdbinit scrip
	    gdb_launch_script=gdbinit
	    dump_gdbinit $compile_commands_json > $gdb_launch_script
	    ;;
    esac
    launch_script_footer framaCIRGen >> $framaCIRGen_launch_script;
    launch_script_footer frama-clang >> $frama_clang_launch_script;
else
    func_usage $provided_cmds
fi

echo "generated launcher script: ${framaCIRGen_launch_script}"
echo "generated launcher script: ${frama_clang_launch_script}"

mkdir -p ${frama_clang_analysis_report}

echo "launch the analysis..."
source ${framaCIRGen_launch_script} 2>&1 | tee ${frama_clang_analysis_report}/framaCIRGen.analysis.gen.log
source ${frama_clang_launch_script} 2>&1 | tee ${frama_clang_analysis_report}/frama-clang.analysis.gen.log

