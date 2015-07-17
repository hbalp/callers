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
	    cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e s/^[^\ ]*/callers\ \$\{system_includes\}/g | sed -e s/-c\ //g | sed -e s/\\.o\ /\.out\ /g | awk '{ print "&& " $N " \\" }' | sed -e s#-o\ CMakeFiles[^\ ]*/#-o\ callers-analysis-report/dot/unsorted/#g  >> $callers_launch_script
	    ;;
	*)
	    echo "analyze file $2..."; 
	    cat $compile_commands_json | grep \"command\" | grep $2 | cut -d '"' -f4 | sed -e s/^[^\ ]*/callers\ \$\{system_includes\}/g | sed -e s/-c\ //g | sed -e s/\\.o\ /\.out\ /g | awk '{ print "&& " $N " \\" }' | sed -e s#-o\ CMakeFiles[^\ ]*/#-o\ callers-analysis-report/dot/unsorted/#g >> $callers_launch_script
	    ;;
    esac

    launch_script_footer;

else
    func_usage; exit 0
fi

chmod +x $callers_launch_script

echo "generated launcher script: ${callers_launch_script}"

mkdir -p callers-analysis-report/dot/unsorted

echo "launch the analysis..."

./launch.gen.sh

# sort the body of dot files to remove duplicated lines
echo "sort the body of dot files to remove duplicated lines..."
mkdir -p callers-analysis-report/dot/sorted
unsorted_dot_files=`find callers-analysis-report/dot/unsorted -type f -name "*.dot"`
for f in $unsorted_dot_files
do
b=`basename $f`
head -1 callers-analysis-report/dot/unsorted/$b > .tmp.gen.header
tail -6 callers-analysis-report/dot/unsorted/$b > .tmp.gen.footer
cat callers-analysis-report/dot/unsorted/$b | egrep -v "{|}" | sort -u > .tmp.gen.body
cat .tmp.gen.header .tmp.gen.body .tmp.gen.footer > callers-analysis-report/dot/sorted/$b
done
rm -f .tmp.gen.header .tmp.gen.body .tmp.gen.footer

# concatenate all the sorted dot files into one unique dot file named all.dot
echo "concatenate all the sorted dot files into one unique dot file named all.unsorted.dot"
all_unsorted_dot_file=callers-analysis-report/dot/all.unsorted.dot
sorted_dot_files=`find callers-analysis-report/dot/sorted -type f -name "*.dot"`
for f in $sorted_dot_files
do
    b=`basename $f`
    cat $f | egrep -v "{|}">> $all_unsorted_dot_file
done

# sort the body of the all.unsorted.dot file to remove duplicated lines
echo "sort the body of the all.unsorted.dot file to remove duplicated lines"
all_sorted_dot_file=callers-analysis-report/dot/all.sorted.dot
all_dot_file=`basename $all_sorted_dot_file`
echo "digraph all {" > $all_sorted_dot_file
cat $all_unsorted_dot_file | sort -u >> $all_sorted_dot_file
echo "}" >> $all_sorted_dot_file
echo "generated ${all_dot_file} file: ${all_sorted_dot_file}"

# convert when possible the resulting all.sorted.dot file into an image
echo "convert when possible the resulting ${all_dot_file} file into an image"
#mkdir -p callers-analysis-report/png
mkdir -p callers-analysis-report/svg
dot_file_max_nb_lines=5000
nb_lines=`wc -l ${all_sorted_dot_file} | awk '{ print $1 }'`
echo "nb_lines: ${nb_lines}"
# launch dot only if the number of lines is lower then a given threshold
if [ $nb_lines -le $dot_file_max_nb_lines ];
then
    echo "${nb_lines} <= ${dot_file_max_nb_lines}, so we can convert the ${all_dot_file} graph into an image"
    #dot -Tpng $f > callers-analysis-report/png/$b.png
    dot -Tsvg ${all_sorted_dot_file} > callers-analysis-report/${all_dot_file}.svg
else
    echo "${nb_lines} > ${dot_file_max_nb_lines}, so we cannot convert the ${all_dot_file} graph into an image"    
fi

# convert the generated sorted dot files into images
echo "try to convert the generated dot files into images only if the line number is lower than ${dot_file_max_nb_lines}..."
for f in $sorted_dot_files
do
    b=`basename $f`
    nb_lines=`wc -l $f | awk '{ print $1 }'`
    echo "file: $f"
    echo "nb_lines: ${nb_lines}"
    # launch dot only if the number of lines is lower then a given threshold
    if [ $nb_lines -le $dot_file_max_nb_lines ];
    then
	echo "${nb_lines} <= ${dot_file_max_nb_lines}, so we can convert the dot graph into an image"
	#dot -Tpng $f > callers-analysis-report/png/$b.png
	dot -Tsvg $f > callers-analysis-report/svg/$b.svg
    else
	echo "${nb_lines} > ${dot_file_max_nb_lines}, so we cannot convert the dot graph into an image"    
    fi
done
