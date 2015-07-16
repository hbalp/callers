#!/bin/bash
#set -x
#     Copyright (C) 2015 Commissariat à l'Energie Atomique, Thales Communication & Security
#       - All Rights Reserved
#     coded by Franck Vedrine, Hugues Balp

progname=$0
version=0.0.1

# func_usage
# outputs to stdout the --help usage message.
func_usage ()
{
    echo "################################################################################"
    echo "# shell script to launch the clang \"callers\" analysis plugin"
    echo "# version $version"
    echo "################################################################################"
    echo "# Usage:"
    echo "launch_callers_analysis.sh <cmake_compile_commands.json> (all|<specific_file>)"
}

# func_version
# outputs to stdout the --version message.
func_version ()
{
    echo "################################################################################"
    echo "clang callers plugin v$version"
    echo "located at $progname"
    echo "Copyright (C) 2015 Thales Communication & Security, Commissariat à l'Energie Atomique"
    echo "Written by Hugues Balp and Franck Vedrine"
    echo "  - All Rights Reserved"
    echo "There is NO WARRANTY, to the extent permitted by law."
    echo "################################################################################"
}

#echo "nb_params: " $#

callers_launch_script=launch.gen.sh
# # dot2png_script=dot2png.gen.sh
# # dot2svg_script=dot2svg.gen.sh

# system_includes
# retrieve the system include files required by clang
system_includes ()
{
    compile_commands_json=$1

    # get the absolute path to the first file to be analyzed
    file=`grep \"file\" ${compile_commands_json} | tail -1 | cut -d '"' -f4`
    clang=`which clang`
    
    system_includes=`strace -f -e verbose=all -s 256 -v ${clang} -std=c++11 $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g|& sed -e s/'"-internal-externc-isystem", "'/'-I"'/g |& sed -e s/", "/"\n"/g |& grep "\-I\"" | sed -e s/\"//g | awk '{print}' ORS=' ' `

    echo "system_includes: $system_includes"

    echo "system_includes=\"$system_includes\"" >> $callers_launch_script
}

launch_script_header ()
{
    compile_commands_json=$1
    echo "#!/bin/bash" > $callers_launch_script
    echo "#set -x" >> $callers_launch_script
    system_includes $compile_commands_json;
    echo "echo \"Begin function call graph analysis...\" \\" >> $callers_launch_script
}

launch_script_footer ()
{
    echo "&& echo" >> $callers_launch_script
    echo "echo \"End function call graph analysis.\"" >> $callers_launch_script
}

if test $# = 0; then
    func_usage; 
    exit 0
    
elif test $# = 1; then

    case "$1" in
	--help | --hel | --he | --h )
	    func_usage; exit 0 ;;
	--version | --versio | --versi | --vers | --ver | --ve | --v )
	    func_version; exit 0 ;;
	*)
	    func_usage; exit 0 ;;
    esac

elif test $# = 2; then

    compile_commands_json=$1
    json_filename=`basename ${compile_commands_json}`
    
    case $json_filename in
	"compile_commands.json" )
	    echo "json_file: ${json_filename}";
	    launch_script_header $compile_commands_json;;
	*)
	    func_usage; exit 0 ;;
    esac

    case $2 in
	"all" )
	    echo "analyze all files...";
	    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e s/^[^\ ]*/callers\ \$\{system_includes\}/g | sed -e s/-c\ //g | sed -e s/\\.o\ /\.out\ /g | awk '{ print "&& " $N " \\" }' | sed -e s#-o\ CMakeFiles[^\ ]*/#-o\ callers-analysis-report/dot/#g  >> $callers_launch_script
	    ;;
	*)
	    echo "analyze file $2..."; 
	    cat $compile_commands_json | grep \"command\" | grep $2 | cut -d '"' -f4 | sed -e s/^[^\ ]*/callers\ \$\{system_includes\}/g | sed -e s/-c\ //g | sed -e s/\\.o\ /\.out\ /g | awk '{ print "&& " $N " \\" }' | sed -e s#-o\ CMakeFiles[^\ ]*/#-o\ callers-analysis-report/dot/#g >> $callers_launch_script
	    ;;
    esac

    launch_script_footer;

else
    func_usage; exit 0
fi

chmod +x $callers_launch_script

echo "generated launcher script: ${callers_launch_script}"

mkdir -p callers-analysis-report/dot

echo "launch the analysis..."

./launch.gen.sh

echo "convert the generated dot files into png and svg images"

# convert the generated dot files into png and svg images
dot_files=`find callers-analysis-report -type f -name "*.dot"`
mkdir -p callers-analysis-report/png
mkdir -p callers-analysis-report/svg
for f in $dot_files
do
b=`basename $f`
dot -Tpng $f > callers-analysis-report/png/$b.png
dot -Tsvg $f > callers-analysis-report/svg/$b.svg
done

# # generate a script to convert dot files into png and svg images
# # echo "mkdir -p callers-report/png" > $dot2png_script
# # grep \"file\" compile_commands.json | cut -d '"' -f4 | awk '{ printf( "dot -Tpng %s > callers-analysis-report/png/", $1 ); system("basename " $1) }' | sed -e s/$/.png/g >> $dot2png_script
# # chmod +x $dot2png_script

# # generate a script to convert dot files into svg images
# # echo "mkdir -p callers-report/svg" > $dot2svg_script
# # grep \"file\" compile_commands.json | cut -d '"' -f4 | awk '{ printf( "dot -Tsvg %s > callers-analysis-report/svg/", $1 ); system("basename " $1) }' | sed -e s/$/.svg/g >> $dot2svg_script
# # chmod +x $dot2svg_script
