#!/bin/bash
#set -x

source "common.sh" # get_file

# check whether the argument is present in input arguments of the script
function redirect_output_file ()
{
    args=$@
    new_args=""
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
	    echo "c_file: ${c_file}"
	fi
    fi

    if [ -z ${c_file} ]
    then
	if [ -n ${cpp_file} ]
	then
	    fileext="cpp"
	    src_file=${cpp_file}
	    echo "cpp_file: ${cpp_file}"
	fi
    fi

    if [ -z ${src_file} ]
    then
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
	echo "redirect_output_file::ERROR::Not Found .c or .cpp source file in args: ${args}" > /dev/stderr
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
	return 1
    fi

    if [ -z ${obj_file} ]
    then
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
	echo "redirect_output_file::ERROR::Not Found obj source file in args: ${args}" > /dev/stderr
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
	return 1
    fi

    # echo "src_file: $src_file"
    # echo "obj_file: $obj_file"

    cabs_file=`echo ${obj_file} | sed -e s/\\.o$/.cabs.c/g`

    for a in $args
    do
	if  [ ${a} != -c ]          && 
	    [ ${a} != -o ]          && 
	    [ ${a} != ${src_file} ] && 
	    [ ${a} != ${obj_file} ]
	then
	    new_args="${new_args} $a "
	fi
    done

    # echo "new_args: ${new_args}"
    # echo "redirection:"
    #echo "${new_args} \" ${src_file} > ${cabs_file}"
    echo "${new_args} \" ${src_file} > ${cabs_file}"
    echo "gzip ${cabs_file}"
}

redirect_output_file $@
