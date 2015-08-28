#!/bin/bash
#set -x

# check whether the argument is present in input arguments of the script
function redirect_output_file()
{
    option=$1
    shift 1
    args=$@
    output=""

    # echo "option: ${option}"
    # echo "args: ${args}"

    # initializes the first new arg
    # check whether the first args is the output option
    if [ ${option} == -o ]
    then
	new_args=""
    else
	new_args="${option} "
    fi

    for a in $args
    do
	#echo "check whether ${option} == \"-o\":"
	if [ ${option} == -o ]
	then
	    #echo "output file option is: ${a}"
	    output=${a}
	elif [ ${a} != -o ]
	then
	    new_args="${new_args} $a "
	fi
	option=$a
    done

    if [ -z ${output} ]
    then
	echo "ERROR_NO_OUTPUT_FILE_IN_ARGS: ${output}"
	return 1
    else
	printf "${new_args} > ${output}"
	return 0
    fi
}

redirect_output_file $@
