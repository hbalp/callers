# File generated by Callers's cmake_init.sh
cmake_minimum_required(VERSION 2.8)

# generate list of source files with related compilation command
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add debug symbols
set(CMAKE_BUILD_TYPE Debug)

# option to activate Frama-C analysis
option(FRAMA_C "Option to activate Frama-C VA analysis" OFF)
#option(FRAMA_C "Option to activate Frama-C VA analysis" ON)

# save temporary files when needed
if(FRAMA_C)
  #option(SAVE_TEMPS "Save temporary files" ON)
  # WARNING: do not try to use the Frama_C_memcpy builtin function
  # cf http://lists.gforge.inria.fr/pipermail/frama-c-discuss/2009-December/001644.html
  #set(CMAKE_C_FLAGS " -save-temps -C -DFRAMA_C -D__FC_USE_BUILTIN__ -D__FC_MACHDEP_X86_64")
set(CMAKE_C_FLAGS " -save-temps -C -DFRAMA_C -D__FC_MACHDEP_X86_64")
set(CMAKE_CXX_FLAGS " -save-temps -C -DFRAMA_C ")
endif()

# add path to header files
if(FRAMA_C)
  # frama-c -print-share-path
  include_directories("/tools/exec/share/frama-c/libc")
  # include_directories("/opt/stance/share/frama-c/libc")
  # include_directories("/home/hbalp/ocamlbrew/ocaml-4.02.3/.opam/system/share/frama-c/libc")
else()
  include_directories("/usr/include")
endif()

# define by default a build target when source files are present
#add_executable(test_assert test_assert.c)
#add_executable(test_stdio test_stdio.c)
#add_executable(test_pp-annot test_pp-annot.c)
#add_executable(test_malloc test_malloc.c stdlib.c)
add_executable(test_memcpy test_memcpy.c)
