#!/bin/bash
#set -x
# Copyright (C) 2015 Thales Communication & Security
#   - All Rights Reserved
# author: Hugues Balp
# 
# This script generates a json file listing the defined symbols of each object file
# contained in ${rootdir} directory

progname=$0
version=0.0.1

# func_usage
# outputs to stdout the --help usage message.
function func_usage ()
{
    echo "################################################################################"
    echo "# shell script to generate a json file listing the defined symbols "
    echo "# of each object file included in <rootdir> directory."
    echo "# version $version"
    echo "################################################################################"
    echo "# Usage:"
    echo "${progname} <rootdir> [<defined_symbols.json>]"
    exit 0
}

# func_version
# outputs to stdout the --version message.
function func_version ()
{
    echo "################################################################################"
    echo "clang callers plugin v$version"
    echo "located at $progname"
    echo "Copyright (C) 2015 Thales Communication & Security, Commissariat à l'Energie Atomique"
    echo "Written by Hugues Balp and Franck Vedrine"
    echo "  - All Rights Reserved"
    echo "There is NO WARRANTY, to the extent permitted by law."
    echo "################################################################################"
    exit 0
}

function generate_defined_symbols_jsonfile ()
{
    rootdir=$1
    #jsonfile="build/defined_symbols.gen.json"

    echo "{\"defined_symbols\":["

    # List all object file included in rootdir and its subdirectories

    declare -i pos=0
    nb_obj_files=`find ${rootdir} -name "*.o" | wc -l`
    for o in `find ${rootdir} -name "*.o"`; 
    do
	# get the cpp source file related to the .o file
	bname=`basename $o .o`
	cpp_fullpath=`find ${rootdir}  -name "${bname}" -o -name "${bname}.c*" | grep -v callers.gen.json`
	if [ -z ${cpp_fullpath} ]; then
	    echo "IGNORE: Unable to find the source file related to the object file ${o}, so ignore it !" >&2
	    continue
	    #exit -1
	else
	    #echo "* Object file: ${o} is generated from source file: ${cpp_fullpath}" >&2
	    echo "* Source file(s): ${cpp_fullpath}" >&2
	fi
	# WARNING: several source files resiging in different directories may share the same filename !
	# so we have to loop over all the source files sharing the same name
	for fullpath in ${cpp_fullpath}
	do
	    cpp_file=`basename ${fullpath}`
	    cpp_dirpath=`dirname ${fullpath}`
	    if [ -z ${cpp_dirpath} ]; then
		echo "ERROR: empty path for source file ${fullpath} !!!"
		exit -2
	    fi
	    echo "{ \"file\": \"$cpp_file\", \"path\":\"$cpp_dirpath\", \"defined\":["
   	    # {\"sign\":\"\"}, {}, {} ] }"
	    nm -C $o | grep " T " | awk 'NR > 1 { printf(", ") } { print "{ \"sign\":\"" $3 "\"}" }'
            pos=($pos+1)
	    if [[ $pos == $nb_obj_files ]];then
		echo "]}"
	    else
		echo "]},"
	    fi
	done
    done
    echo "]}"
}

if test $# = 0; then
    func_usage; 
    exit 0
    
elif test $# = 1; then

    case "$1" in
	--help | --hel | --he | --h )
	    func_usage; exit 0 ;;
	--version | --versio | --versi | --vers | --ver | --ve | --v )
	    func_version ;;
	*)
	    # We assume here that the provided argument is the <roodir>
	    rootdir=$1
	    defined_symbols_jsonfilename="defined_symbols.json"
	    defined_symbols_tmpfilename="${defined_symbols_jsonfilename}.tmp"
	    echo "Generate list of defined symbols in json file named \"${defined_symbols_jsonfilename}\""
	    generate_defined_symbols_jsonfile ${rootdir} > ${defined_symbols_tmpfilename}
	    ydump ${defined_symbols_tmpfilename} > ${defined_symbols_jsonfilename}
	    rm ${defined_symbols_tmpfilename}
    esac

elif test $# = 2; then
    
    # WARNING: no input validation of the provided arguments
    # TODO: add some input validation
    rootdir=$1
    defined_symbols_jsonfilename=$2
    defined_symbols_tmpfilename="$2.tmp"
    #json_filename=`basename ${defined_symbols_jsonfilename}`
    generate_defined_symbols_jsonfile ${rootdir} ${defined_symbols_tmpfilename}
    ydump ${defined_symbols_tmpfilename} > ${defined_symbols_jsonfilename}
    rm ${defined_symbols_tmpfilename}
else
    func_usage
fi
