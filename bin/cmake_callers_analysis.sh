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
    echo "# shell script to launch the clang \"callers\" analysis plugin"
    echo "# version $version"
    echo "################################################################################"
    echo "# Usage:"
    echo "# cmake_callers_analysis.sh <cmake_compile_commands.json> (all|<specific_file>) <callers_analysis_report_dir>"
    echo "# Provided command was: ${provided_cmds}"
    exit -1
}

# func_version
# outputs to stdout the --version message.
function func_version ()
{
    echo "################################################################################"
    echo "# clang callers plugin v$version"
    echo "# located at $progname"
    echo "# Copyright (C) 2015 Thales Communication & Security, Commissariat à l'Energie Atomique"
    echo "# Written by Hugues Balp and Franck Vedrine"
    echo "#  - All Rights Reserved"
    echo "# There is NO WARRANTY, to the extent permitted by law."
    echo "################################################################################"
    exit 0
}

callers_launch_script=launch.gen.sh

# system_includes
# retrieve the system include files required by clang
function system_includes ()
{
    compile_commands_json=$1

    # get the absolute path to the first file to be analyzed
    file=`grep \"file\" ${compile_commands_json} | tail -1 | cut -d '"' -f4`
    clang=`which clang`
    
    system_includes=`strace -f -e verbose=all -s 256 -v ${clang} -std=c++11 $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g|& sed -e s/'"-internal-externc-isystem", "'/'-I"'/g |& sed -e s/", "/"\n"/g |& grep "\-I\"" | sed -e s/\"//g | awk '{print}' ORS=' ' `

    echo "system_includes: $system_includes"

    echo "system_includes=\"$system_includes\"" >> $callers_launch_script
}

function dump_gdbinit ()
{
    compile_commands_json=$1

    # get the absolute path to the first file to be analyzed
    file=`grep \"file\" ${compile_commands_json} | tail -1 | cut -d '"' -f4`
    clang=`which clang`
    
    system_includes=`strace -f -e verbose=all -s 256 -v ${clang} -std=c++11 $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g|& sed -e s/'"-internal-externc-isystem", "'/'-I"'/g |& sed -e s/", "/"\n"/g |& grep "\-I\"" | sed -e s/\"//g | awk '{print}' ORS=' ' `

    gdb_launch_script=gdbinit
    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4  | sed -e s/^[^\ ]*//g > .build_cmds
    while read -r line
    do
	file=`echo $line | awk '{print $NF}' | xargs basename`
	printf "# $file\n" >> $gdb_launch_script
	printf "# set args -s ${system_includes} $line\n" >> $gdb_launch_script
	printf "\n" >> $gdb_launch_script
    done < .build_cmds
}

function launch_script_header ()
{
    compile_commands_json=$1
    echo "#!/bin/bash" > $callers_launch_script
    echo "#set -x" >> $callers_launch_script
    system_includes $compile_commands_json;
    echo "# files to analyze: ${files}" >> $callers_launch_script
    echo "echo \"Begin function call graph analysis...\" \\" >> $callers_launch_script
}

function launch_script_footer ()
{
    echo "&& echo" >> $callers_launch_script
    echo "echo \"End function call graph analysis.\"" >> $callers_launch_script
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
    callers_analysis_report=$3

    json_filename=`basename ${compile_commands_json}`
    
    case $json_filename in
	"compile_commands.json" )
	    echo "json_file: ${json_filename}";
	    launch_script_header $compile_commands_json;;
	*)
	    func_usage $provided_cmds;;
    esac

    case $files in
	"all" )
	    echo "analyze all files...";
	    # make sure the output directories are well created before calling the analysis
	    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e "s/.*-o //g" | awk '{ print $1 }' | sort -u | xargs dirname | awk '{ print "&& mkdir -p " $N " \\" }' >> $callers_launch_script
	    # build the analysis command from the build one listed in file compile_commands.json
	    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e s/^[^\ ]*/callers\ $\{system_includes\}/g | sed -e s/-c\ //g | sed -e s/\\.o\ /\.gen.callers.stdout\ /g | awk '{ print "&& " $N " \\" }' >> $callers_launch_script
	    # prepare command arguments for gdbinit script
	    dump_gdbinit $compile_commands_json
	    ;;
	*)
	    echo "analyze file $files..."; 
	    # make sure the output directories are well created before calling the analysis
	    cat $compile_commands_json | grep \"command\" | grep $files | cut -d '"' -f4 | sed -e "s/.*-o //g" | awk '{ print $1 }' | sort -u | xargs dirname | awk '{ print "&& mkdir -p " $N " \\" }' >> $callers_launch_script
	    # build the analysis command from the build one listed in file compile_commands.json
	    cat $compile_commands_json | grep \"command\" | grep $files | cut -d '"' -f4 | sed -e s/^[^\ ]*/callers\ \$\{system_includes\}/g | sed -e s/-c\ //g | sed -e s/\\.o\ /\.gen.callers.stdout\ /g | awk '{ print "&& " $N " \\" }' >> $callers_launch_script
	    # prepare command arguments for gdbinit script
	    dump_gdbinit $compile_commands_json
	    ;;
    esac

    launch_script_footer;
else
    func_usage $provided_cmds
fi

chmod +x ${callers_launch_script}

echo "generated launcher script: ${callers_launch_script}"

mkdir -p ${callers_analysis_report}

echo "launch the analysis..."

./${callers_launch_script} 2>&1 | tee ${callers_analysis_report}/callers.analysis.gen.log

