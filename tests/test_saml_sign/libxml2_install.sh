#!/bin/bash
# Script managing the installation and build of the libxml2 library
# Two available configurations: libxml2_local_install and libxml2_git_install

#INSTALL_CONFIG="git"
INSTALL_CONFIG="local"

ici=`pwd`

# precondition: host dependant libxml2 config
function libxml2_config_shared ()
{
  #export libinstalldir="${librootdir}/exec"
  export libinstalldir="/tools/exec"
  export libxml2_local_gdb_dir="libxml2_gdb"
  export libxml2_src_gdb_config_args=""
  export libxml2_local_callers_dir="libxml2_callers"
  export libxml2_src_callers_config_args=""
  export libxml2_local_fc_dir="libxml2_fc"
  #export libxml2_install_cots="true"
  export libxml2_install_cots="false"
  export libxml2_autogen_config_filename=".libxml2.config.gen.sh"
}

# host dependant libxml2 config
function libxml2_config_host()
{
    host=`hostname`
    if [ ${host} == "devrte" ]; then
        libxml2_config_host_vm
    elif [ ${host} == "moriond" ]; then
        libxml2_config_host_moriond
    else
        echo "libxml2_install:ERROR: no available libxml2 configuration for unknown host ${host}"
    fi
}

# libxml2 config for host moriond
function libxml2_config_host_moriond ()
{
  export librootdir="/home/hbalp/hugues/work/third_parties/src"
  export libxml2_local_dir_name="libxml2"
  #export libxml2_local_archive_name="libxml2-2.9.4"
  export libxml2_local_archive_name="libxml2-2.9.4.1.stance"
  echo "HBDEBUG EXPORT:" libxml2_local_archive_fullname="${libxml2_local_archive_name}.tgz"
  export libxml2_local_archive_fullname="${libxml2_local_archive_name}.tgz"
}

# libxml2 config for host devrte
function libxml2_config_host_vm ()
{
  export librootdir="/data/balp/src/tools"
  export libxml2_local_dir_name="libxml2"
  export libxml2_local_archive_name="libxml2-2.9.3"
  export libxml2_local_archive_fullname="${libxml2_local_archive_name}.tar.gz"
}

# for install_config="git"
function libxml2_config_git_archive ()
{
  export libxml2_git_archive_url="git://git.gnome.org/libxml2"
}

# fc build option:
# CC=gcc CFLAGS="-save-temps -C -D__FC_MACHDEP_X86_64 -I /tools/exec/share/frama-c/libc" make
function libxml2_config_fc_va ()
{
  export frama_c_share_dir=`frama-c -print-share-path`
  export frama_c_libc_dir="${frama_c_share_dir}/libc"
  export libxml2_src_fc_config_args="CC=gcc CFLAGS=\"-save-temps -C -D__FC_MACHDEP_X86_64 -I ${frama_c_libc_dir}\""
}

function usage_libxml2_install()
{
    cat > /dev/stdout <<EOF
Usage of script libxml2_install.sh
5 features:
1) System install of libxml2:
   > libxml2_workflow_system_install
2) Source install of libxml2:
   > libxml2_workflow_sources_gdb <git|local>
3) Source Callers analysis of libxml2:
   > libxml2_workflow_sources_callers <git|local>
4) Frama-C builtin preprocessing of libxml2:
   > libxml2_workflow_fc_va <git|local>
5) Frama-C update preprocessing of libxml2:
   > libxml2_update_fc_preproc
    return 0;
EOF
}

# function libxml2_workflow ()
# {
#     # you should choose between a system install
#     libxml2_workflow_system_install
    
#     # ... or a build and install from sources
#     libxml2_workflow_sources_gdb
# }

function libxml2_workflow_system_install ()
{
    libxml2_config_common
    
    # Uninstall
    #libxml2_binary_uninstall
    
    # Install
    libxml2_binary_install
}

function libxml2_workflow_sources_gdb ()
{
    install_config=$1
    if [ -z ${install_config} ]; then
       echo "libxml2 source install error: expected one user parameter"
       echo "usage: libxml2_workflow_sources_gdb <install_config=git|local>"
       return 1;
    fi
    libxml2_config_common
    cd ${librootdir}

    # Uninstall
    #libxml2_source_uninstall
    
    # install COTS used by libxml2
    libxml2_cots_install &&

    # get the sources of libxml2
    libxml2_get_sources ${install_config} ${libxml2_local_gdb_dir} &&
    
    # Configure
    libxml2_source_config ${libxml2_src_gdb_config_args} > .libxml2_config.gen.stdout 2> .libxml2_config.gen.stderr &&
    (
      # Build
      libxml2_gdb_build > .libxml2_build.gen.stdout 2> .libxml2_build.gen.stderr &&

      # Install built library
      libxml2_git_master_install
      
    ) ||

    ( echo "libxml2 source config error; you need probably to install the COTS library libtool by using: libxml2_cots_force_install..."; return 3 ) 
}

function libxml2_workflow_sources_callers ()
{
    install_config=$1
    if [ -z ${install_config} ]; then
       echo "libxml2 source callers analysis error: expected one user parameter"
       echo "usage: libxml2_workflow_sources_callers <install_config=git|local>"
       return 1;
    fi
    libxml2_config_common
    cd ${librootdir}

    # Uninstall
    #libxml2_source_uninstall
    
    # install COTS used by libxml2
    libxml2_cots_install &&

    # get the sources of libxml2
    libxml2_get_sources ${install_config} ${libxml2_local_callers_dir} &&
    
    # Configure
    libxml2_source_config ${libxml2_src_callers_config_args} > .libxml2_config.gen.stdout 2> .libxml2_config.gen.stderr &&
    (
      # Callers analysis
      libxml2_callers_analysis > .libxml2_callers.gen.stdout 2> .libxml2_callers.gen.stderr
      
    ) ||

    ( echo "libxml2 source callers analysis config error; you need probably to install the COTS library libtool by using: libxml2_cots_force_install..."; return 3 ) 
}

function libxml2_fc_va_prepare ()
{
    install_config=$1
    if [ -z ${install_config} ]; then
       echo "libxml2 fc va error: expected one user parameter"
       echo "usage: libxml2_fc_va_prepare <install_config=git|local>"
       return 1;
    fi
    libxml2_config_common
    cd ${librootdir}

    # Uninstall
    #libxml2_source_uninstall

    # Install
    libxml2_fc_va_preproc ${install_config}
}

function libxml2_update_fc_preproc ()
{
    libxml2_config_common
    cd ${librootdir}
    cd ${libxml2_local_fc_dir}
    CC=gcc CFLAGS="-save-temps -C -D__FC_MACHDEP_X86_64 -I ${frama_c_libc_dir}" make
}

function libxml2_config_common ()
{
    libxml2_config_host
    libxml2_config_shared
    libxml2_config_git_archive
    libxml2_config_fc_va
}

function libxml2_binary_uninstall ()
{
    libxml2_system_uninstall

    # uninstall COTS used by libxml2 only when needed
    libxml2_cots_uninstall
}

function libxml2_binary_install ()
{
    libxml2_cots_install
    libxml2_system_install
}

# download or unzip a local archive according to the install_config and dest_dir parameters
function libxml2_get_sources ()
{
    install_config=$1
    dest_dir=$2
    libdir="${librootdir}/${dest_dir}"
    if [ ${install_config} == "git" ]; then
      # download libxml2 sources
      libxml2_git_master_clone ${dest_dir}
    elif [ ${install_config} == "local" ]; then
      # or unzip a local archive when download is not possible
      libxml2_local_archive ${dest_dir}
    else
      return 1;
    fi
}

# unzip a local archive when download is not possible
function libxml2_local_archive ()
{
    dest_dir=$1
    cd ${librootdir}
    if [ -d ${dest_dir} ]; then
       echo "WARNING: already existing dest_dir=${dest_dir}"
       echo "We will overwrite it !"
       #echo "Do you really want to overwrite it ?"
       rm -rf ${dest_dir}
    fi
    if [ ! -f ${libxml2_local_archive_fullname} ]; then
	(
	    echo "libxml2_install ERROR: Not found tar archive \"${libxml2_local_archive_fullname}\" in \"${librootdir}\"";
	    return 7;
	)
    fi
    tar -zxf ${libxml2_local_archive_fullname}
    mv ${libxml2_local_dir_name} ${dest_dir}
}

function libxml2_source_uninstall ()
{
    # remove local source only when needed
    libxml2_git_master_uninstall
    
    libxml2_git_master_delete

    # uninstall COTS used by libxml2 only when needed
    libxml2_cots_uninstall
}

function libxml2_fc_va_preproc ()
{
    install_config=$1
    libxml2_config_fc_va
    
    # install COTS used by libxml2
    libxml2_cots_install

    # get the sources of libxml2
    libxml2_get_sources ${install_config} ${libxml2_local_fc_dir}

    # Configure
    libxml2_source_config ${libxml2_src_fc_config_args} > .libxml2_config.gen.stdout 2> .libxml2_config.gen.stderr
    
    # Build
    libxml2_fc_build > .libxml2_preproc.gen.stdout 2> .libxml2_preproc.gen.stderr

    # Install built library
    #libxml2_git_master_install
    
    # Prepare FC analysis
    # libxml2_fc_prepare > /dev/null 2> .libxml2_fc_prepare.gen.stderr
}

# install COTS used by libxml2 when configured
function libxml2_cots_install ()
{
    if [ ${libxml2_install_cots} == "true" ]; then
    	libxml2_cots_force_install
    fi
}

function libxml2_cots_force_install ()
{
    echo "WARNING: We need root privileges to install COTS used by libxml2..."
    sudo apt-get install libtool
}

# uninstall COTS used by libxml2 when configured
function libxml2_cots_uninstall ()
{
    if [ ${libxml2_install_cots} == "true" ]; then
        echo "WARNING: We need root privileges to uninstall COTS used by libxml2..."
	sudo apt-get remove libtool
    fi
}

function libxml2_system_install ()
{
    echo "WARNING: We need root privileges to install the system library libxml2"
    sudo apt-get install libxml2-dev
}

function libxml2_system_uninstall ()
{
    echo "WARNING: We need root privileges to uninstall the system library libxml2"
    sudo apt-get remove libxml2-dev
}

function libxml2_git_master_clone ()
{
    cd ${librootdir}
    clone_dir=$1
    git clone ${libxml2_git_archive_url} ${clone_dir}
}

function libxml2_git_master_delete ()
{
    cd ${librootdir}
    rm -rf ${libxml2_local_clone_dir}
}

function libxml2_git_master_install ()
{
    cd ${libdir}
    echo "WARNING: We need root privileges to install in dir ${libinstalldir} the library libxml2 built in ${libdir}"
    sudo make install
}

function libxml2_git_master_uninstall ()
{
    cd ${libdir}
    echo "WARNING: We need root privileges to uninstall from dir ${libinstalldir} the library libxml2 built in ${libdir}"
    sudo make uninstall
}

function libxml2_source_config ()
{
    cflags=$@
    if [ -d ${libdir} ]; then
	(
	    echo "INFO: libdir=${libdir}" > /dev/stdout;
	    cd ${libdir}
	    #autogen.sh
	    #autogen.sh &&
	    #autoreconf &&
	    #CFLAGS+="${cflags}" ./configure \
	    cat > ${libxml2_autogen_config_filename} <<EOF
#!/bin/bash \\
${cflags} ./autogen.sh \\
          --prefix=/tools/exec \\
          --disable-shared \\
          --disable-fast-install \\
          --enable-ipv6=no \\
          --without-c14n \\
          --without-catalog \\
          --with-debug \\
          --without-docbook \\
          --without-fexceptions \\
          --without-ftp \\
          --without-history \\
          --without-html \\
          --without-http \\
          --without-iconv \\
          --without-icu \\
          --with-iso8859x \\
          --without-legacy \\
          --with-mem-debug \\
          --with-minimum \\
          --without-output \\
          --without-pattern \\
          --without-push \\
          --without-python \\
          --without-reader \\
          --without-regexps \\
          --with-run-debug \\
          --with-sax1 \\
          --without-schemas \\
          --without-schematron \\
          --without-threads \\
          --without-tree \\
          --without-valid \\
          --without-writer \\
          --without-xinclude \\
          --without-xpath \\
          --without-xptr \\
          --without-modules \\
          --without-zlib \\
          --without-lzma \\
          --without-coverage
EOF
	    source ${libxml2_autogen_config_filename} || ( echo "libxml2 source config error" > /dev/stderr; return 2 )
	)
    else
	( echo "libxml2_install:ERROR: Not found libdir=${libdir}" > /dev/stderr; return 5 )
	return
    fi

}

function libxml2_callers_analysis ()
{
    echo "libxml2 callers analysis..."
    export CC=clang
    export CXX=clang++
    export CFLAGS="-g -O0"
    export LDFLAGS="-g -O0"
    export CALLERS_ANALYSIS_TYPE=callers
    cd ${libdir}
    scan-callers --use-analyzer `which clang` -o callers make > /dev/stdout 2> /dev/stderr
    result=$?
    echo "retcode: ${result}"
}

function libxml2_gdb_build ()
{
  cd ${libdir}
  CC=clang CLFAGS="-g -O0" LDFLAGS="-g -O0" make
}

function libxml2_fc_build ()
{
  cd ${libdir}
  CC=gcc CFLAGS="-O0 -save-temps -C -D__FC_MACHDEP_X86_64 -I ${frama_c_libc_dir}" make
}

# function libxml2_fc_prepare ()
# {
#   cd ${libdir}
#   source fc_analysis.sh
#   fc_parse
# }
