#!/bin/bash
#set -x
#     Copyright (C) 2015 Commissariat à l'Energie Atomique, Thales Communication & Security
#       - All Rights Reserved
#     coded by Franck Vedrine, Hugues Balp

export CC=`which clang`
export CXX=`which clang++`

cmake -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug .

compile_commands_json="compile_commands.json"

callers_launch_script=launch.gen.sh
dot2png_script=dot2png.gen.sh
dot2svg_script=dot2svg.gen.sh

get the absolute path to the first file to be analyzed
file=`grep \"file\" compile_commands.json | tail -1 | cut -d '"' -f4`
clang=`which clang`

system_includes=`strace -f -e verbose=all -s 256 -v ${clang} -std=c++11 $file |& grep execve |& grep "bin/clang" |& grep cc1 |& sed -e s/'"-internal-isystem", "'/'-I"'/g|& sed -e s/'"-internal-externc-isystem", "'/'-I"'/g |& sed -e s/", "/"\n"/g |& grep "\-I\"" | sed -e s/\"//g | awk '{print}' ORS=' ' `

echo "system_includes: $system_includes"

echo "#!/bin/bash -x" > $callers_launch_script

echo "system_includes=\"$system_includes\"" >> $callers_launch_script

echo "echo \"Begin function call graph analysis...\" \\" >> $callers_launch_script

cat $compile_commands_json | grep \"command\" | cut -d '"' -f4 | sed -e s/^[^\ ]*/callers\ \$\{system_includes\}/g | sed -e s/-c\ //g | sed -e s/\\.o\ /\.out\ /g | awk '{ print "&& " $N " \\" }'  >> $callers_launch_script

echo "echo" >> $callers_launch_script
echo "echo \"End function call graph analysis.\"" >> $callers_launch_script

chmod +x $callers_launch_script

echo "generated launcher script: ${callers_launch_script}"

echo "launch the analysis..."

./launch.gen.sh

echo "convert the generated dot files into png and svg images"

# convert the generated dot files into png and svg images
dot_files=`find CMakeFiles -type f -name "*.dot"`
for f in $dot_files
do
dot -Tpng $f > $f.png
dot -Tsvg $f > $f.svg
done

# generate a script to convert dot files into png and svg images
# echo "mkdir -p callers-report/png" > $dot2png_script
# grep \"file\" compile_commands.json | cut -d '"' -f4 | awk '{ printf( "dot -Tpng %s > callers-report/png/", $1 ); system("basename " $1) }' | sed -e s/$/.png/g >> $dot2png_script
# chmod +x $dot2png_script

# generate a script to convert dot files into svg images
# echo "mkdir -p callers-report/svg" > $dot2svg_script
# grep \"file\" compile_commands.json | cut -d '"' -f4 | awk '{ printf( "dot -Tsvg %s > callers-report/svg/", $1 ); system("basename " $1) }' | sed -e s/$/.svg/g >> $dot2svg_script
# chmod +x $dot2svg_script
