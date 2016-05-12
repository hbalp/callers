# download libxml2-2.9.3.tar.gz
ici=`pwd`
#libxml2=libxml2-2.9.3
libxml2=libxml2
#librootdir=${ici}
#librootdir="/data/balp/src/tools"
librootdir="/home/hbalp/hugues/work/third_parties/src"
libdir="${librootdir}/${libxml2}"
#libinstalldir="${librootdir}/exec"
libinstalldir="/tools/exec"

function libxml2_workflow ()
{
    # you should choose between a system install
    libxml2_binary_workflow
    
    # ... or a build and install from sources
    libxml2_source_workflow
}

function libxml2_binary_workflow ()
{
    cd ${librootdir}

    # Uninstall
    #libxml2_binary_uninstall
    
    # Install
    libxml2_binary_install
}

function libxml2_source_workflow ()
{
    cd ${librootdir}

    # Uninstall
    #libxml2_source_uninstall
    
    # Install
    libxml2_source_install
}

function libxml2_fc_va_prepare ()
{
    cd ${librootdir}

    # Uninstall
    #libxml2_source_uninstall

    # Install
    libxml2_fc_va_preproc
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

function libxml2_source_install ()
{
    # install COTS used by libxml2
    libxml2_cots_install

    # download libxml2 sources
    #libxml2_git_master_clone
    # or unzip a local archive when download is not possible
    tar -zxf libxml2-2.9.3.tar.gz

    # Configure
    libxml2_config "" > .libxml2_config.stdout 2> .libxml2_config.stderr

    # Build
    libxml2_build > .libxml2_build.stdout 2> .libxml2_build.stderr

    # Install built library
    libxml2_git_master_install
    
    # Prepare FC analysis
    # libxml2_fc_prepare > /dev/null 2> .libxml2_fc_prepare.stderr
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
    # install COTS used by libxml2
    #libxml2_cots_install

    # download libxml2 sources
    libxml2_git_master_clone
    # or unzip a local archive when download is not possible
    #tar -zxf libxml2-2.9.3.tar.gz

    # Configure
    #libxml2_config "-save-temps -C -D__FC_MACHDEP_X86_64 -I/opt/stance/share/frama-c/libc" > .libxml2_config.stdout 2> .libxml2_config.stderr
    libxml2_config "-save-temps -C -D__FC_MACHDEP_X86_64 -I/home/hbalp/ocamlbrew/ocaml-4.02.3/.opam/system/share/frama-c/libc" > .libxml2_config.stdout 2> .libxml2_config.stderr
    
    # Build
    libxml2_build > .libxml2_preproc.stdout 2> .libxml2_preproc.stderr

    # Install built library
    #libxml2_git_master_install
    
    # Prepare FC analysis
    # libxml2_fc_prepare > /dev/null 2> .libxml2_fc_prepare.stderr
}

function libxml2_cots_install ()
{
    # install COTS used by libxml2
    sudo apt-get install libtool
}

function libxml2_cots_uninstall ()
{
    # remove COTS used by libxml2
    sudo apt-get remove libtool
}

function libxml2_system_install ()
{
    sudo apt-get install libxml2-dev
}

function libxml2_system_uninstall ()
{
    sudo apt-get remove libxml2-dev
}

function libxml2_git_master_clone ()
{
    cd ${librootdir}
    git clone git://git.gnome.org/libxml2
}

function libxml2_git_master_delete ()
{
    cd ${librootdir}
    rm -rf ${libxml2}
}

function libxml2_git_master_install ()
{
    cd ${libdir}
    sudo make install
}

function libxml2_git_master_uninstall ()
{
    cd ${libdir}
    sudo make uninstall
}

function libxml2_config ()
{
    cflags=$1
    cd ${libdir}
    autogen.sh &&
    #autoreconf &&
    #CFLAGS+="-save-temps -C -DSTANCE_SSO -DFRAMA_C -D__FC_MACHDEP_X86_64" ./configure --without-ftp --without-http --without-zlib --with-run-debug --with-mem-debug --prefix=${libinstalldir}
    ##CFLAGS+="-save-temps -DSTANCE_SSO -D__FC_MACHDEP_X86_64 -I /opt/stance/share/frama-c/libc" ./configure --without-ftp --without-http --without-zlib --with-run-debug --with-mem-debug --prefix=${libinstalldir}
    CFLAGS+="${cflags}" ./configure \
	--prefix=/tools/exec \
	--enable-ipv6=no \
	--without-c14n \
	--without-catalog \
	--with-debug \
	--without-docbook \
	--without-fexceptions \
	--without-ftp \
	--without-history \
	--without-html \
	--without-http \
	--without-iconv \
	--without-icu \
	--with-iso8859x \
	--without-legacy \
	--with-mem-debug \
	--with-minimum \
	--without-output \
	--without-pattern \
	--without-push \
	--without-python \
	--without-reader \
	--without-regexps \
	--with-run-debug \
	--with-sax1 \
	--without-schemas \
	--without-schematron \
	--without-threads \
	--without-tree \
	--without-valid \
	--without-writer \
	--without-xinclude \
	--without-xpath \
	--without-xptr \
	--without-modules \
	--without-zlib \
	--without-lzma \
	--without-coverage
}

function libxml2_build ()
{
  cd ${libdir}
  make VERBOSE=yes
}

function libxml2_fc_prepare ()
{
  cd ${libdir}
  source fc_analysis.sh
  fc_parse
}
