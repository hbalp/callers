#!/bin/bash
#set -x
#     Copyright (C) 2015 Commissariat à l'Energie Atomique, Thales Communication & Security
#       - All Rights Reserved
#     coded by Franck Vedrine, Hugues Balp

progname=$0
version=0.0.1

# to configure when needed
dot_file_max_nb_lines=3000
#dot_file_max_nb_lines=5000

# func_usage
# outputs to stdout the --help usage message.
short_description="shell script to convert dot files into svg files"
func_usage ()
{
    echo "################################################################################"
    echo "# ${short_description}"
    echo "# WARNING: the max number of lines accepted in the input dot files is ${dot_file_max_nb_lines}"
    echo "# any further lines will be ignored"
    echo "# version $version"
    echo "################################################################################"
    echo "# Usage:"
    echo "process_dot_files.sh <dot_files_root_dir>"
}

# func_version
# outputs to stdout the --version message.
func_version ()
{
    echo "################################################################################"
    echo "${short_description} v$version"
    echo "located at $progname"
    echo "Copyright (C) 2015 Thales Communication & Security, Commissariat à l'Energie Atomique"
    echo "Written by Hugues Balp and Franck Vedrine"
    echo "  - All Rights Reserved"
    echo "There is NO WARRANTY, to the extent permitted by law."
    echo "################################################################################"
}

dot_subgraph ()
{
    if [ -z $3 ]; then
	echo "HBDBG ERROR: dot_subgraph function expects 3 arguments !"
	echo "expected arguments: (dot_filepath, nb_lines, subgraph_nb_lines) "
	echo "provided arguments: (dot_filepath=$1, nb_lines=$2, subgraph_nb_lines=$3) "
	exit 17;
    else
	echo "HBDBG dot_subgraph(dot_filepath=$1, nb_lines=$2, subgraph_nb_lines=$3)"
    fi
    dot_filepath=$1
    dot_filename=`basename ${dot_filepath}`
    nb_lines=`wc -l ${dot_filepath} | awk '{ print $1 }'`
    subgraph_nb_lines=$2
    subgraph_dot_filepath=$3
    subgraph_dot_filename=`basename ${subgraph_dot_filepath}`
    echo "digraph \"${subgraph_dot_filename}\" {" > ${subgraph_dot_filepath}
    #echo "HBDBG: if [ $subgraph_nb_lines -ge $nb_lines ];"
    if [ $subgraph_nb_lines -ge $nb_lines ];
    then
	echo "ERROR:dot_subgraph: ${subgraph_nb_lines} >= ${nb_lines} and the subgraph cannot contain more lines than the input file \"${dot_filename}\""
	exit 0
    else
	echo "${subgraph_nb_lines} < ${nb_lines}, so we extract the subgraph \"${subgraph_dot_filename}\" from input file \"${dot_filename}\""	
	cat $dot_filepath | tail -${subgraph_nb_lines} >> ${subgraph_dot_filepath}
    fi
    #echo "}" >> ${subgraph_dot_filepath}
}

# convert the dot graph defined in input file into an image (svg by default, png commented)
convert_dot_file ()
{
    dot_filepath=$1
    dot_filename=`basename ${dot_filepath}`
    nb_lines=`wc -l ${dot_filepath} | awk '{ print $1 }'`
    echo "file: ${dot_filepath}"
    echo "nb_lines: ${nb_lines}"
    # launch dot only if the number of lines is lower then a given threshold
    if [ $nb_lines -le $dot_file_max_nb_lines ];
    then
	echo "${nb_lines} <= ${dot_file_max_nb_lines}, so we can convert the dot graph defined in file \"${dot_filename}\" into an image"
	#dot -Tpng ${dot_filepath} > callers-analysis-report/png/${dot_filename}.png
	dot -Tsvg ${dot_filepath} > callers-analysis-report/svg/${dot_filename}.svg
    else
	# build the subgraph containing the first ${dot_file_max_nb_lines}"
	echo "${nb_lines} > ${dot_file_max_nb_lines}, so we cannot convert the whole graph named defined in file \"${dot_filename}\" into an image"    
	subgraph_dot_filepath=`echo ${dot_filepath} | sed -e s/\\\\.dot/.subgraph.dot/g`
	subgraph_dot_filename=`basename ${subgraph_dot_filepath}`
	# echo "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
	# echo "HBDBG: dot_filepath=${dot_filepath}"
	# echo "HBDBG: subgraph_dot_filepath=${subgraph_dot_filepath}"
	# echo "HBDBG: subgraph_dot_filename=${subgraph_dot_filename}"
	# echo "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
	echo "however, we will build the subgraph \"${subgraph_dot_filename}\" containing the first ${dot_file_max_nb_lines}"
	dot_subgraph ${dot_filepath} ${dot_file_max_nb_lines} ${subgraph_dot_filepath}
	#dot -Tpng ${subgraph_dot_filepath} > callers-analysis-report/png/${subgraph_dot_filename}.png
	dot -Tsvg ${subgraph_dot_filepath} > callers-analysis-report/svg/${subgraph_dot_filename}.svg
    fi
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
	    dot_root_dirpath=$1 ;;
    esac
else
    func_usage; exit 0
fi

here=`pwd`
dot_root_dir=`basename ${dot_root_dirpath}`
dot_root_parentdir=`dirname ${dot_root_dirpath}`

echo "HBDBG: dot_root_dir=${dot_root_parentdir}/${dot_root_dir}"
echo "HBDBG cd ${dot_root_parentdir}"
cd ${dot_root_parentdir}

# sort the body of dot files to remove duplicated lines
echo "sort the body of dot files to remove duplicated lines..."
#mkdir -p callers-analysis-report/dot/sorted
unsorted_dot_files=`find ${dot_root_dir} -type f -name "*.unsorted.out.dot"`
for f in $unsorted_dot_files
do
b=`basename $f`
d=`dirname $f`
head -1 $f > .tmp.gen.header
tail -6 $f > .tmp.gen.footer
cat $f | egrep -v "{|}" | sort -u > .tmp.gen.body
sorted_filename=`echo $b | sed -e s/unsorted/sorted/g`
cat .tmp.gen.header .tmp.gen.body .tmp.gen.footer > $d/${sorted_filename}
done
rm -f .tmp.gen.header .tmp.gen.body .tmp.gen.footer

# concatenate all the sorted dot files into one unique dot file named all.dot
echo "concatenate all the sorted dot files into one unique dot file named all.unsorted.dot"
all_unsorted_dot_file=callers-analysis-report/dot/all.unsorted.dot
sorted_dot_files=`find ${dot_root_dir} -type f -name "*.sorted.out.dot"`
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
convert_dot_file ${all_sorted_dot_file}

# convert the generated sorted dot files into images
echo "try to convert the generated dot files into images only if the line number is lower than ${dot_file_max_nb_lines}..."
for f in $sorted_dot_files
do
    convert_dot_file $f
done

cd $here
