#!/bin/bash
#set -x

source "common.sh" # get_file

function launch_frama_clang ()
{
    cpp_file=$1
    cabs_file=$2
    shift
    shift
    file_analysis_options=$@

    # localize frama-c
    frama_c=`which frama-c`

    # define frama-clang configuration options
    frama_clang_options="-cxx-nostdinc -cxx-keep-mangling -fclang-msg-key clang,cabs -fclang-verbose 2 -machdep x86_32 -print -cxx-clang-command"

    # add target source file specific analysis options
    frama_clang_analysis_options="${file_analysis_options}"

    # build the frama_clang analysis command
    cpp_analysis="${frama_c} ${frama_clang_options} \"framaCIRGen \${system_includes} ${frama_clang_analysis_options}\" ${cpp_file} > ${cabs_file}"

    # make sure the output directories are well created before calling the analysis ?

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch frama-clang analysis of file: ${cpp_file}\""
    echo "echo \"cppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcp\""
    echo "#gdb --args "
    echo "${cpp_analysis}"
    echo "gzip -f ${cabs_file}"    
}

function launch_frama_c ()
{
    c_file=$1
    cabs_file=$2
    shift
    shift
    file_analysis_options=$@

    # localize frama-c
    frama_c=`which frama-c`

    # define frama-c configuration options
    frama_c_options="-machdep x86_32 -print -no-cpp-gnu-like "

    # add target source file specific analysis options
    frama_c_analysis_options="-cpp-extra-args=\"${file_analysis_options}\""

    # build the frama_c analysis command
    c_analysis="${frama_c} ${frama_c_options} ${frama_c_analysis_options} ${c_file} > ${cabs_file}"

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch frama-c analysis of file: ${c_file}\""
    echo "echo \"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\""
    echo "#gdb --args "
    echo "${c_analysis}"
    echo "gzip -f ${cabs_file}"
}

function launch_framaCIRGen ()
{
    src_file=$1
    fir_file=$2
    shift
    shift
    file_analysis_options=$@

    # localize frama-c
    framaCIRGen=`which framaCIRGen`

    # define frama-clang configuration options
    framaCIRGen_options=""

    # add target source file specific analysis options
    framaCIRGen_analysis_options="${file_analysis_options}"

    # build the framaCIRGen analysis command
    fir_analysis="${framaCIRGen} ${framaCIRGen_options} \${system_includes} ${framaCIRGen_analysis_options} -o ${fir_file} ${src_file}"
    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch framaCIRGen analysis of file: ${src_file}\""
    echo "echo \"ccccppccccppccccppccccppccccppccccppccccppccccppccccppccccppccccppccccppccccppcc\""
    echo "#gdb --args "
    echo "${fir_analysis}"
    echo "gzip -f ${fir_file}"    
}

function launch_callers_cpp ()
{
    cpp_file=$1
    callers_stdout_file=$2
    shift
    shift
    file_analysis_options=$@

    # localize callers
    callers=`which callers++`

    # add some options when required
    callers_options="-std=c++11 -I."

    # build the callers analysis command    
    callers_analysis="${callers} ${callers_options} \${system_includes} ${file_analysis_options} -o ${callers_stdout_file} ${cpp_file}"

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch callers++ analysis of file: ${cpp_file}\""
    echo "echo \"cppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcppcp\""
    echo "#gdb --args "
    echo "${callers_analysis}"
    echo "gzip -f ${callers_stdout_file}"
}

function launch_callers_c ()
{
    c_file=$1
    callers_stdout_file=$2
    shift
    shift
    file_analysis_options=$@

    # localize callers
    callers=`which callers`

    # add some options when required
    callers_options="-I."

    # build the callers analysis command    
    callers_analysis="${callers} ${callers_options} \${system_includes} ${file_analysis_options} -o ${callers_stdout_file} ${c_file}"

    echo "echo \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\""
    echo "echo \"launch callers analysis of file: ${c_file}\""
    echo "echo \"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\""
    echo "#gdb --args "
    echo "${callers_analysis}"
    echo "gzip -f ${callers_stdout_file}"
}

function prepare_frama_clang_analysis_from_compile_command()
{
    # ignore the path to the build tool
    shift
    args=$@
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
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"  > /dev/stderr
	echo "prepare_frama_clang_analysis::ERROR::Not Found .c or .cpp source file in args: ${args}" > /dev/stderr
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"  > /dev/stderr
	return 1
    fi

    if [ -z ${obj_file} ]
    then
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" > /dev/stderr
	echo "prepare_frama_clang_analysis::ERROR::Not Found obj source file in args: ${args}"  > /dev/stderr
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" > /dev/stderr
	return 1
    fi

    # echo "src_file: $src_file"
    # echo "obj_file: $obj_file"

    cabs_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.cabs.c/g`
    fir_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.fir/g`
    callers_stdout_file=`echo ${obj_file} | sed -e s/\\.o$/.gen.callers.stdout/g`

    run_callers="false"
    run_frama_clang="false"
    run_framaCIRGen="false"

    # get the analysis_type = callers | frama-clang | framaCIRGen | all
    analysis_type=${CALLERS_ANALYSIS_TYPE}
    #echo "c++-analysis type is: ${analysis_type}" 

    case $analysis_type in

	"callers" )
	    #echo "activates callers analysis";
	    run_callers="true"
	    ;;

	"frama-c" )
	    #echo "activates frama-c analysis";
	    run_frama_clang="true"
	    ;;

	"frama-clang" )
	    #echo "activates frama-clang analysis";
	    run_frama_clang="true"
	    ;;

	"framaCIRGen" )
	    #echo "activates framaCIRGen analysis";
	    run_framaCIRGen="true"
	    ;;

	"all" )
	    #echo "activates all kind of analysis: callers, frama_clang and framaCIRGen";
	    run_callers="true"
	    run_frama_clang="true"
	    # run_framaCIRGen="true"
	    ;;

	*)
	    #echo "builds the input file without any analysis..."
	    ;;
    esac

    # get source file's specific build options
    file_build_options=""
    for a in $args
    do
	if  [ ${a} != -c ]          && 
	    [ ${a} != -o ]          && 
	    [ ${a} != ${src_file} ] && 
	    [ ${a} != ${obj_file} ]
	then
	    if  [ ${a} != "-nostdinc" ]
	    then
		file_build_options="${file_build_options} $a "
	    fi
	fi
    done

    if [ $fileext == "cpp" ]
    then
	if [ $run_frama_clang == "true" ] 
	then
	    launch_frama_clang ${src_file} ${cabs_file} ${file_build_options}
	fi
	if [ $run_framaCIRGen == "true" ] 
	then
	    launch_framaCIRGen ${src_file} ${fir_file} ${file_build_options}
	fi
	if [ $run_callers == "true" ] 
	then
	    launch_callers_cpp ${src_file} ${callers_stdout_file} ${file_build_options}
	fi
    elif [ $fileext == "c" ]
    then
	if [ $run_frama_clang == "true" ] 
	then
	    launch_frama_c ${src_file} ${cabs_file} ${file_build_options}
	fi
	if [ $run_framaCIRGen == "true" ] 
	then
	    launch_framaCIRGen ${src_file} ${fir_file} ${file_build_options}
	fi
	if [ $run_callers == "true" ] 
	then
	    launch_callers_c ${src_file} ${callers_stdout_file} ${file_build_options}
	fi
    else
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
	echo "prepare_frama_clang_analysis::ERROR::interbal error: unreachable state !"
	echo "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
    fi
}

function prepare_analysis_from_cmake_compile_commands()
{
    compile_commands_json=$1

    echo "#!/bin/bash"
    echo "#set -x"

    # get the absolute path to the first file to be analyzed
    file=`grep \"file\" ${compile_commands_json} | tail -1 | cut -d '"' -f4`

    # get the system includes that are required o launch frama_clang
    system_includes $file

    # build the analysis command from the build one listed in file compile_commands.json
    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | while read command_line; do prepare_frama_clang_analysis_from_compile_command ${command_line}; done
}

function prepare_analysis_from_scan_build_command()
{
    src_file=$1
    shift
    build_command=$@

    echo "#!/bin/bash"
    echo "#set -x"
    system_includes ${src_file}

    # prepare the analysis launch script from the build command
    prepare_frama_clang_analysis_from_compile_command ${build_command}
}
