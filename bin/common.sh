#!/bin/bash
#set -x

# check whether the argument is present in input arguments of the script
function has_arg()
{
    arg=$1
    shift 1
    args=$@
    echo "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ"
    echo "arg: ${arg}"
    echo "args: ${args}"
    for a in $args
    do
	echo "check whether ${arg} == ${a}:"
	if [ ${arg} == ${a} ]
	then
	    echo "YES"
	    return 0
	fi
    done
    echo "NO"
    return 1
}

function is_option()
{
    id=$1
    if [ ${id:0:1} == "-" ]
    then
	#echo "id: ${id} is an option"
	return 0
    else
	#echo "id: ${id} is not an option"
	return 1
    fi
}

function has_extension()
{
    file=$1
    ext=$2
    bname=`basename $file`
    fname=`basename $bname $ext`
    if [ $fname == $bname ]
    then
	#echo "ext of file ${file} is not ${ext}"
	return 1
    else
	#echo "ext of file ${file} is ${ext}"
	return 0
    fi
}

function get_file()
{
    file_ext=$1
    shift 1
    args=$@
    #echo "file_ext: ${file_ext}"
    #echo "args: ${args}"
    for a in $args
    do
	#echo "check whether ${a} is an option:"
	is_option ${a}
	is_opt=$?
	if [ $is_opt -eq 1 ]
	then
	    #echo "check whether ${a} is a file with ext=${file_ext}:"
	    has_extension ${a} ${file_ext}
	    has_ext=$?
	    if [ $has_ext -eq 0 ]
	    then
	        #echo "file of ext ${file_ext} is ${a}"
		echo "${a}"
		return 0
	    fi
	fi
    done
    #echo "No file of ext ${file_ext} has been found in $@"
    return 1
}

# system_includes
# retrieve the system include files required by clang
function system_includes ()
{
    # path to the first file to be analyzed
    file=$1
    clang=`which clang`
    
    system_includes=`strace -f -e verbose=all -s 256 -v ${clang} -std=c++11 $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g|& sed -e s/'"-internal-externc-isystem", "'/'-I"'/g |& sed -e s/", "/"\n"/g |& grep "\-I\"" | sed -e s/\"//g | awk '{print}' ORS=' ' `

    # echo "system_includes: $system_includes" >&2
    echo "system_includes=\"$system_includes\""
}

function list_files_in_dirs()
{
    root_dirpath=$1
    fileext=$2
    jsondir_fileext=$3
    ignored_directories=$4

    list_json_files_in_dirs.native ${root_dirpath} ${fileext} ${jsondir_fileext} ${ignored_directories}
    if [ $? -ne 0 ]; then
	echo "################################################################################"
	echo "# ERROR in list_json_files_in_dirs.native error $@. Stop here !"
	echo "################################################################################"
	exit -1
    fi
}

function list_defined_symbols()
{
    defined_symbols_jsonfilename=$1
    current_dir=$2
    jsondir_fileext=$3

    list_defined_symbols.native ${defined_symbols_jsonfilename} ${current_dir} ${jsondir_fileext}
    if [ $? -ne 0 ]; then
	echo "################################################################################"
	echo "# ERROR in list_defined_symbols.native $@. Stop here !"
	echo "################################################################################"
	exit -1
    fi
}
