#!/bin/bash
# Extract nb functions of each cots used by the stance sso use case
#set +x

function cots_list_functions ()
{
    cots_name=$1
    cots_dirpath=$2
    cots_functions_filename="${cots_name}.functions.gen.all"
    #cots_functions_tmpfilename="${cots_name}.functions.gen.tmp"
    find ${cots_dirpath} -type f -exec egrep -1 "\"sign\":" {} \; | egrep -v "\"decl\":" | egrep -v "\"def\":" | egrep -v "::operator" | egrep -v "operator new"  | egrep -v "operator delete" | egrep -v "gnu_cxx" | grep -v "boost::algorithm" | sed -e "s#^ *\"#\"#g" | sort -u | egrep "\"" > ${cots_functions_filename}
}

function cots_nb_loc ()
{
    cots_name=$1
    cots_dirpath=$2
    cots_cloc_filename="${cots_name}.gen.cloc"
    cloc_path="/data/balp/src/tools/cloc"
    cloc="${cloc_path}/cloc"
    echo ${cloc} ${cots_dirpath}
    ${cloc} ${cots_dirpath} > ${cots_cloc_filename}
}

function cots_nb_functions ()
{
    cots_name=$1
    cots_functions_filename="${cots_name}.functions.gen.all"
    cots_nb_fcts=`wc -l ${cots_functions_filename} | awk '{print $1}'`
    echo "| ${cots_name} | ${cots_nb_fcts} |"
}

function uc_sso_nb_loc ()
{
    cots_dirpath=$1
    for cots_path in `find ${cots_dirpath} -maxdepth 1 -type d`;
    do
        cots_name=`basename ${cots_path}`
        #echo "${cots_name}: ${cots_path}"
        #echo cots_nb_loc ${cots_name} ${cots_path}
        cots_nb_loc ${cots_name} ${cots_path}
    done
}

function uc_sso_nb_functions ()
{
    cots_dirpath=$1
    for cots_path in `find ${cots_dirpath} -maxdepth 1 -type d`;
    do
        cots_name=`basename ${cots_path}`
        #echo "${cots_name}: ${cots_path}"
        cots_list_functions ${cots_name} ${cots_path}
        cots_nb_functions ${cots_name}
    done
}

uc_sso_nb_loc /data/balp/src/stance/uc_sso/sp

#uc_sso_nb_functions /tmp/callers/data/balp/src/stance/uc_sso/sp
#uc_sso_nb_functions /tmp/callers/data/balp/src/stance/uc_sso/sp/curl-7.35.0
