#!/bin/bash
#set -x
#     Copyright (C) 2015 Thales Communication & Security
#       - All Rights Reserved
#     coded by Hugues Balp

# exename=$1

function cmakelists_gen ()
{
  target=$1
  src_dir=$2
  cmakelists_filename="CMakeLists.txt"

  ici=`pwd`
  #echo "ici: ${ici}"
  cd $src_dir
  echo "src_dir: ${src_dir}"
  src_files=`ls *.c* 2> /dev/null`
  if [ "${src_files}" != "" ]; then
    add_executable="add_executable(${target} ${src_files})"
  else
    add_executable="#add_executable(${target} tbc)"
  fi
  # list subdirectories
  #subdirs=`ls -l | awk '/^d/ { print $9 }'`
  # add all subdirectories even if not really required
  add_subdirs=`ls -l | awk '/^d/ { print "add_subdirectory(" $9 ")" }'`

  cd $ici

  cat > ${cmakelists_filename} <<EOF
# File generated by Callers's cmake_init.sh
cmake_minimum_required(VERSION 2.8)

# generate list of source files with related compilation command
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add debug symbols
set(CMAKE_BUILD_TYPE Debug)

#option(USE_CXX_EXCEPTIONS "Enable C++ exception support" ON)
# save temporary files when needed
option(SAVE_TEMPS "Save temporary files" ON)
#option(SAVE_TEMPS "Save temporary files" OFF)

if(SAVE_TEMPS)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -save-temps")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps")
endif()

# add subdirectories when needed
${add_subdirs}

# add path to header files
include_directories(".")

# define by default a build target when source files are present
${add_executable}

# add path to external libraries
#target_link_libraries(${target} /path/to/lib*.so)
EOF
}

function cmake_clean ()
{
  src_dir=$PWD
  cd ${src_dir}
  rm -rf build
  find ${src_dir} -type f -name "CMakeLists.txt" -exec rm -f {} \;
}

function cmake_init ()
{
  exename=$1
  src_dir=$PWD
  cd ${src_dir}
  ici="$PWD"
  echo "Generate default CMakeLists.txt build files in directories:"
  subdirs=`find ${ici} -type d`
  for subdir in ${subdirs};
  do
    cd ${subdir}
    subdir_basename=`basename ${subdir}`
    echo "subdir: ${subdir}"
    echo "base_subdir: ${subdir_basename}"
    cmakelists_gen "${exename}_${subdir_basename}" ${subdir};
  done
  cd ${ici}
  cd ..
}

function cmake_build ()
{
  src_dir=$PWD
  cd ${src_dir}
  echo "CMake build..."
  mkdir -p build
  cd build
  cmake ..
  make
  cd ${src_dir}
}

#cmake_init ${exename}