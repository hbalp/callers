#!/bin/bash
#set -x

source "common.sh" # get_file

# check whether the argument is present in input arguments of the script

function prepare_frama_clang_analysis_from_compile_command()
{
    build_tool=$1
    shift
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
	    echo "# c_file: ${c_file}"
	fi
    fi

    if [ -z ${c_file} ]
    then
	if [ -n ${cpp_file} ]
	then
	    fileext="cpp"
	    src_file=${cpp_file}
	    echo "# cpp_file: ${cpp_file}"
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

    # localize frama-c
    frama_c=`which frama-c`

    if [ $fileext == "cpp" ]
    then

        # define frama-clang configuration options
	frama_clang_options="-cxx-nostdinc -cxx-keep-mangling -fclang-msg-key clang,cabs -fclang-verbose 2 -machdep x86_32 -print -cxx-clang-command"

        # add target source file specific analysis options
	frama_clang_analysis_options="${new_args}"

	# build the frama_clang analysis command
	cpp_analysis="${frama_c} ${frama_clang_options} \"framaCIRGen \${system_includes} ${frama_clang_analysis_options}\" ${src_file} > ${cabs_file}"
	echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
	echo "echo \"launch frama-clang analysis of file: ${cpp_file}\""
	echo "echo \"cppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcp\""
	echo "${cpp_analysis}"
	echo "gzip -f ${cabs_file}"

    elif [ $fileext == "c" ]
    then

        # define frama-c configuration options
	frama_c_options="-machdep x86_32 -print -no-cpp-gnu-like "

        # add target source file specific analysis options
	frama_c_analysis_options="-cpp-extra-args=\"${new_args}\""

	# build the frama_c analysis command
	c_analysis="${frama_c} ${frama_c_options} ${frama_c_analysis_options} ${src_file} > ${cabs_file}"

	echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
	echo "echo \"launch frama-clang analysis of file: ${c_file}\""
	echo "echo \"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\""
	echo "${c_analysis}"
	echo "gzip -f ${cabs_file}"

    else
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
	echo "prepare_frama_clang_analysis::ERROR::interbal error: unreachable state !"
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
    fi
}

function prepare_frama_clang_analysis_from_cmake_compile_commands()
{
    compile_commands_json=$1

    echo "#!/bin/bash"
    echo "#set -x"
    system_includes $compile_commands_json

    # make sure the output directories are well created before calling the analysis
    #cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e "s/.*-o //g" | awk '{ print $1 }' | sort -u | xargs dirname | awk '{ print "&& mkdir -p " $N " \\" }'

    # build the analysis command from the build one listed in file compile_commands.json
    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | while read command_line; do prepare_frama_clang_analysis_from_compile_command ${command_line}; done
}

compile_commands_json=$1

prepare_frama_clang_analysis_from_cmake_compile_commands $compile_commands_json > .tmp.gen.frama-clang.launch.sh

cat .tmp.gen.frama-clang.launch.sh
