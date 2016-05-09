# download libxml2-2.9.3.tar.gz
ici=`pwd`
libxml2=libxml2-2.9.3
#librootdir=${ici}
librootdir="/data/balp/src/tools"
libdir="${librootdir}/${libxml2}"
libinstalldir="${librootdir}/exec"

function libxml2_install ()
{
  sudo apt-get install libxml2-dev
  #cd ${librootdir}
  #tar -zxvf libxml2-2.9.3.tar.gz
  #tar -zxvf libxml2-2.9.3.stance_sso.tar.gz
}

function libxml2_config ()
{
  cd ${libdir}
  CFLAGS+="-save-temps -DSTANCE_SSO -DFRAMA_C -D__FC_MACHDEP_X86_64" ./configure --without-ftp --without-http --without-zlib --with-run-debug --with-mem-debug --prefix=${libinstalldir}
  #CFLAGS+="-save-temps -DSTANCE_SSO -D__FC_MACHDEP_X86_64 -I /opt/stance/share/frama-c/libc" ./configure --without-ftp --without-http --without-zlib --with-run-debug --with-mem-debug --prefix=${libinstalldir}
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
  fc_parse libxml2
}

function libxml2_workflow ()
{
#  libxml2_install
  libxml2_config > /dev/null 2> .libxml2_config.stderr
  libxml2_build > /dev/null 2> .libxml2_build.stderr
  libxml2_fc_prepare > /dev/null 2> .libxml2_fc_prepare.stderr
}
