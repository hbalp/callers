/****
     Copyright (C) 2015 Thales Communication & Security
       - All Rights Reserved
     coded by Hugues Balp
****/

//
// Description:
//   Callers's plugin c++ data structure
//   to be used with the ATDgen data structure defined in file callgraph.atd
//   for generating the callers's json files.
//

#define CALLERS_TYPE_DIR         "http://com.thalesgroup.fr/tcs/callers/1.0.0#//dir"
#define CALLERS_TYPE_DIR_SYMBOLS "http://com.thalesgroup.fr/tcs/callers/1.0.0#//dir_symbols"
#define CALLERS_TYPE_FILE        "http://com.thalesgroup.fr/tcs/callers/1.0.0#//file"
#define CALLERS_TYPE_RECORD      "http://com.thalesgroup.fr/tcs/callers/1.0.0#//record"
#define CALLERS_TYPE_FCT_DECL    "http://com.thalesgroup.fr/tcs/callers/1.0.0#//fct_decl"
#define CALLERS_TYPE_FCT_DEF     "http://com.thalesgroup.fr/tcs/callers/1.0.0#//fct_def"

#define CALLERS_ERROR_UNSUPPORTED_CASE 17

#define CALLERS_ROOTDIR_PREFIX     "/tmp/callers"
#define CALLERS_INCLUDES_FULL_DIR  "/tmp/callers/includes"
#define CALLERS_INCLUDES_DIR       "/includes"

#define CALLERS_DIR_JSON_EXT  ".dir.callers.gen.json"
#define CALLERS_FILE_JSON_EXT ".file.callers.gen.json"

#define CALLERS_DEFAULT_RECORD_NAME    "noRecordName"
#define CALLERS_DEFAULT_RECORD_PATH    "noRecordPath"
#define CALLERS_DEFAULT_NO_RECORD_NAME "C_code"
#define CALLERS_DEFAULT_NO_RECORD_PATH "none"
#define CALLERS_DEFAULT_NO_THREAD_ID   "noThread"

#define CALLERS_DEFAULT_BUILTIN_NAMESPACE "env"
// #define CALLERS_DEFAULT_BUILTIN_RECORD_NAME CALLERS_DEFAULT_BUILTIN_NAMESPACE
#define CALLERS_DEFAULT_BUILTIN_RECORD_NAME "builtin"
#define CALLERS_DEFAULT_BUILTIN_RECORD_PATH "builtin"

#define CALLERS_DEFAULT_NO_NAMESPACE_NAME "::"
#define CALLERS_DEFAULT_NO_QUALIFICATION_NAME ""

#define CALLERS_NO_FILE_PATH     "unknownFilePath"
#define CALLERS_NO_FILE_NAME     "unknownFileName"
#define CALLERS_NO_FCT_DEF_FILE  "unknownFctDefFile"
#define CALLERS_NO_FCT_DECL_FILE "unknownFctDeclFile"
#define CALLERS_LOCAL_FCT_DECL   "local"

#define CALLERS_NO_NB_LINES -1
