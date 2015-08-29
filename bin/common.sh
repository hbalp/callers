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
