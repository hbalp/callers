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
    compile_commands_json=$1

    # get the absolute path to the first file to be analyzed
    file=`grep \"file\" ${compile_commands_json} | tail -1 | cut -d '"' -f4`
    clang=`which clang`
    
    system_includes=`strace -f -e verbose=all -s 256 -v ${clang} -std=c++11 $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g|& sed -e s/'"-internal-externc-isystem", "'/'-I"'/g |& sed -e s/", "/"\n"/g |& grep "\-I\"" | sed -e s/\"//g | awk '{print}' ORS=' ' `

    # echo "system_includes: $system_includes" >&2
    echo "system_includes=\"$system_includes\""
}
