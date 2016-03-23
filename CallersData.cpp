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

//#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#ifndef NOT_USE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
#endif
#ifndef NOT_USE_BOOST_REGEX
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#endif
#ifndef NOT_USE_BOOST_STRING
#include <boost/algorithm/string.hpp>
#endif
#include <boost/algorithm/string/predicate.hpp>

#include "assert.h"
#include "clang/AST/Decl.h"
#include "CallersConfig.hpp"
#include "CallersData.hpp"

extern std::string getCanonicalAbsolutePath(const std::string& path);

/***************************************** shared utilities ****************************************/

const std::string gc_root_prefix(CALLERS_ROOTDIR_PREFIX);

// return true if the prefix is well present and false otherwise
bool id_has_prefix(std::string prefix, std::string identifier)
{
  // Check whether the identifier does well begins with prefix
  bool has_prefix = boost::algorithm::contains(identifier, prefix);
  return has_prefix;
}

// stops the execution if the input path does not begins with the expected root dir prefix
void assert_rootdir_prefix(std::string path)
{
  // Check whether the path does well begins with root dir prefix
  bool has_prefix = id_has_prefix(gc_root_prefix, path);
  assert(has_prefix == true);
  return;
}

void assert_homedir_prefix(std::string path)
{
  // Check whether the path does not begins with home dir prefix
  std::string home_prefix ("/tmp/callers/home");
  bool has_prefix = id_has_prefix(home_prefix, path);
  if(has_prefix == true)
  {
    std::cerr << "DEBUG: Bad Directory prefix: /tmp/callers/home" << std::cerr;
    assert(0);
  }
  return;
}

// add the root dir prefix to the input path when it is not present
// similar to the ocaml shared function check_root_dir in file common.ml of the callgraph plugin
std::string check_rootdir_prefix(std::string path)
{
  // Check whether the path dir does well begins with root dir prefix
  bool has_prefix = id_has_prefix(gc_root_prefix, path);
  if( has_prefix )
    {
      // the post-condition is ok, so nothing to do
      return path;
    }
  else
    // add the root dir prefix to the input path
    {
      std::cout << "check_root_dir: adds the rootdir prefix \"" << gc_root_prefix << "\" to the path \"" << path << "\"" << std::endl;
      path = gc_root_prefix + path;
      return path;
    }
}

// remove the prefix when present in the input string
// similar to the ocaml shared function filter_root_dir in file common.ml of the callgraph plugin
std::string id_filter_prefix(std::string prefix, std::string identifier)
{
  // Check whether the string does well begins with the prefix
  bool has_prefix = id_has_prefix(prefix, identifier);
  if( has_prefix )
    // remove the prefix from the input identifier
    {
      std::vector<std::string> parts;
      boost::algorithm::split_regex(parts, identifier, boost::regex(prefix));
      std::vector<std::string>::iterator part = parts.end();
      std::string filtered_identifier = *part;
      return filtered_identifier;
    }
  else
    {
      // the post-condition is ok, so nothing to do
      return identifier;
    }
}

#include <boost/lexical_cast.hpp>

bool decode_function_location(std::string fct_pos, std::string& fct_file, int& fct_line)
{
  std::vector<std::string> loc;
  boost::algorithm::split_regex(loc, fct_pos, boost::regex(":"));
  std::vector<std::string>::iterator l = loc.begin();
  fct_file = *l++;
  // std::cout << "decode_function_location: fct_file=" << fct_file << std::endl;
  if( l != loc.end())
  {
    std::string line = *l++;
    try
    {
      fct_line = boost::lexical_cast<int>(line);
    }
    catch( boost::bad_lexical_cast const& )
    {
      std::cerr << "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" << std::endl;
      std::cerr << "Error: input string line=\"" << line << "\" was not valid"<< std::endl;
      std::cerr << "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" << std::endl;
      exit(9137);
    }
    // std::cout << "decode_function_location: fct_line=" << line << std::endl;
  }
  return true;
}

/***************************************** class JsonFileWriter ****************************************/

CallersData::JsonFileWriter::JsonFileWriter(std::string jsonLogicalFilePath, std::string jsonPhysicalFilePath) :
    jsonLogicalFilePath(jsonLogicalFilePath),
    jsonPhysicalFilePath(jsonPhysicalFilePath),
    out()
{
  // Check whether the input path does well begins with the expected rootdir path
  assert_rootdir_prefix(jsonPhysicalFilePath);

  std::cout << "Try to open source file \"" << jsonPhysicalFilePath << "\" in write mode..." << std::endl;

  bool opened = false;
  do
    {
      out.open(jsonPhysicalFilePath.c_str());
      if(out.fail())
	{
	  std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
	  std::cerr << "WARNING: Failed to open file \"" << jsonPhysicalFilePath << "\" in write mode." << std::endl;
	  std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
	  exit(-1);
	  //sleep(1);
	}
      else
	{
	  std::cout << "JSON output file \"" << jsonPhysicalFilePath << "\" is now opened in write mode." << std::endl;
	  opened = true;
	}
    }
  while(opened == false);
}

CallersData::JsonFileWriter::~JsonFileWriter()
{
  if(jsonLogicalFilePath != jsonPhysicalFilePath)
  {
    if(!(boost::filesystem::exists(jsonLogicalFilePath)))
    {
      std::cout << "Creating file symlink from " << jsonLogicalFilePath << " to " << jsonPhysicalFilePath << std::endl;
      boost::filesystem::create_symlink(jsonPhysicalFilePath, jsonLogicalFilePath);
    }
  }
  std::cout << "Close file \"" << jsonPhysicalFilePath << "\"." << std::endl;
  out.close();
  if(out.fail())
    {
      std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
      std::cerr << "Failed to close file \"" << jsonPhysicalFilePath << "\"." << std::endl;
      std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
      exit(2);
    }
  else
    {
      std::cout << "JSON output file \"" << jsonPhysicalFilePath << "\" is now closed." << std::endl;
    }
}

/***************************************** class Dir ****************************************/

CallersData::Dir::Dir()
{
  assert(0);
}

CallersData::Dir::Dir(std::string dir, std::string path)
  : dir(dir),
    path(path),
    jsonfilename(dir + CALLERS_DIR_JSON_EXT),
    jsonfilepath(path + "/" + dir)
{
  assert_rootdir_prefix(path);
  //assert_homedir_prefix(path);
}

CallersData::Dir::~Dir()
{

}

std::string CallersData::Dir::get_dirpath()
{
  std::string fullpath = path + "/" + dir;
  return fullpath;
}

void CallersData::Dir::add_file(std::string file)
{
  std::cout << "Register file name \"" << file << "\" in directory \"" << this->get_dirpath() << "\"" << std::endl;
  filenames.push_back(file);
}

void CallersData::Dir::add_file(File file)
{
  std::cout << "Register file path \"" << file.get_filepath() << "\"" << std::endl;
  files.insert(file);
}

std::set<CallersData::File>::iterator CallersData::Dir::create_or_get_file(std::string filename, std::string dirpath)
{
  std::string filepath = dirpath + "/" + filename;
  //std::cout << "Check whether the file \"" << filepath << "\" is already opened or not..." << std::endl;
  std::set<CallersData::File>::iterator search_result;
  CallersData::File searched_file(filename, dirpath);
  search_result = files.find(searched_file);
  if(search_result != files.end())
    {
      std::cout << "The file \"" << filepath << "\" is already opened." << std::endl;
    }
  else
    {
      std::cout << "The file \"" << filepath << "\" is not yet registered in dir." << std::endl;
      CallersData::File file(filename, dirpath);
      this->add_file(file);
      search_result = files.find(searched_file);
      if(search_result != files.end())
	{
	  std::cout << "The file \"" << filepath << "\" is well registered in dir \"" << this->dir << "\" now !" << std::endl;
          std::cout << "Tries to open and parse the file \"" << filepath << "\"..." << std::endl;
          search_result->parse_json_file(this);
	  std::cout << "The file \"" << filepath << "\" is well opened and parsed now !" << std::endl;

          // // complete the parsed fct_decl with a redeclared method when needed
          // search_result->try_to_add_redeclared_and_redeclaration_methods(this);
	}
      else
	{
	  std::cerr << "CallersData::Dir::create_or_get_file:ERROR: to open file \"" << filepath << "\" as expected !" << std::endl;
          assert(search_result != files.end());
	}
    }
  return search_result;
}

void CallersData::Dir::output_json_files()
{
  std::set<CallersData::File>::const_iterator f;

  for(f=files.begin(); f!=files.end(); ++f)
    {
      std::cout << "Edit file \"" << f->get_filepath() << "\"..." << std::endl;
      f->output_json_desc();
    }
}

void CallersData::Dir::output_json_dir()
{
  CallersData::JsonFileWriter js(this->jsonfilepath, this->jsonfilepath);
  js.out << "{\"dir\":\"" << dir
         << "\",\"path\":\"" << path
         << "\",\"files\":[";

  std::list<std::string>::const_iterator i, last;
  last = filenames.empty() ? filenames.end() : --filenames.end();
  for(i=filenames.begin(); i!=filenames.end(); ++i)
    {
      if(i != last)
	{
	  js.out << "\"" << *i << "\",";
	}
      else
	{
	  js.out << "\"" << *i << "\"";
	}
    }

  js.out << "]}" << std::endl;
}

/***************************************** class File ****************************************/

CallersData::File::File(std::string filename, std::string dirpath)
  : filename(filename),
    dirpath(dirpath),
    filepath(dirpath + "/" + filename),
    jsonLogicalFilePath(CALLERS_ROOTDIR_PREFIX + dirpath + "/" + filename + CALLERS_FILE_JSON_EXT)
{
  namespaces = new std::set<CallersData::Namespace>;
  declared = new std::set<CallersData::FctDecl>;
  defined = new std::set<CallersData::FctDef>;
  records = new std::set<CallersData::Record>;
  threads = new std::set<CallersData::Thread>;
  calls = new std::set<CallersData::FctCall>;

  CallersData::FileKind fileType = CallersData::File::getKind(filename);
  switch (fileType)
  {
    case E_SourceFile:
    {
      kind = "src";
      jsonPhysicalFilePath = jsonLogicalFilePath;
      break;
    }

    case E_HeaderFile:
    case E_UnknownFileKind:
    {
      kind = "inc";
      jsonPhysicalFilePath = std::string(CALLERS_INCLUDES_FULL_DIR) + "/" + filename + CALLERS_FILE_JSON_EXT;
      break;
    }
  }

  // Check whether the related callers'analysis path does already exists or not in the filesystem
  std::string jsonPhysicalDirPath = CALLERS_ROOTDIR_PREFIX + dirpath;
  if(!(boost::filesystem::exists(jsonPhysicalDirPath)))
  {
    std::cout << "Creating tmp directory: " << jsonPhysicalDirPath << std::endl;
    boost::filesystem::create_directories(jsonPhysicalDirPath);
  }
}

CallersData::File::~File()
{
  //this->output_json_desc();
  delete namespaces;
  delete declared;
  delete defined;
  delete records;
  delete threads;
  delete calls;
}

CallersData::File::File(const CallersData::File& copy_from_me)
{
  filename = copy_from_me.filename;
  kind = copy_from_me.kind;
  dirpath = copy_from_me.dirpath;
  filepath = copy_from_me.filepath;
  jsonLogicalFilePath = copy_from_me.jsonLogicalFilePath;
  jsonPhysicalFilePath = copy_from_me.jsonPhysicalFilePath;

  namespaces = new std::set<CallersData::Namespace>;
  declared = new std::set<CallersData::FctDecl>;
  defined = new std::set<CallersData::FctDef>;
  records = new std::set<CallersData::Record>;
  threads = new std::set<CallersData::Thread>;
  calls = new std::set<CallersData::FctCall>;

  // std::cout << "File Copy Constructor of file \"" << filename
  //           << "\" defining " << copy_from_me.records->size()
  //           << " classes, " << copy_from_me.defined->size() << " functions"
  //           << " and " << copy_from_me.calls->size() << " function calls"
  //           << std::endl;

  // copy namespaces definitions
  std::set<Namespace>::const_iterator n;
  for(n=copy_from_me.namespaces->begin(); n!=copy_from_me.namespaces->end(); ++n)
    {
      namespaces->insert(*n);
    }

  // copy records definitions
  std::set<Record>::const_iterator r;
  for(r=copy_from_me.records->begin(); r!=copy_from_me.records->end(); ++r)
    {
      records->insert(*r);
    }

  // copy function declarations
  std::set<FctDecl>::const_iterator d;
  for(d=copy_from_me.declared->begin(); d!=copy_from_me.declared->end(); ++d)
    {
      declared->insert(*d);
    }

  // copy function definitions
  std::set<FctDef>::const_iterator f;
  for(f=copy_from_me.defined->begin(); f!=copy_from_me.defined->end(); ++f)
    {
      defined->insert(*f);
    }

  // copy function calls definitions
  std::set<FctCall>::const_iterator c;
  for(c=copy_from_me.calls->begin(); c!=copy_from_me.calls->end(); ++c)
    {
      calls->insert(*c);
    }

  // copy threads definitions
  std::set<Thread>::const_iterator thr;
  for(thr=copy_from_me.threads->begin(); thr!=copy_from_me.threads->end(); ++thr)
    {
      threads->insert(*thr);
    }
}

void CallersData::File::parse_json_file(CallersData::Dir *files) const
{
  /* Check whether the related json file does already exists or not. */
  std::cout << "Parsing json file \"" << jsonPhysicalFilePath << "\"..." << std::endl;
  assert_rootdir_prefix(jsonPhysicalFilePath);
  FILE* pFile = fopen(jsonPhysicalFilePath.c_str(), "rb");
  // Always check to see if file opening succeeded
  if (pFile == NULL)
    {
      //std::cout << "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww" << std::endl;
      std::cout << "WARNING: do not parse json file \"" << jsonPhysicalFilePath << "\" which doesn't exists yet !" << std::endl;
      //std::cout << "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww" << std::endl;
    }
  else
    // Parse symbol definitions
    {
      char buffer[65536];
      ::rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
      ::rapidjson::Document file;
      file.ParseStream<0, rapidjson::UTF8<>, ::rapidjson::FileReadStream>(is);
      // Check whether the file is wellformed or not
      if(file.IsObject())
	// Document is a JSON value that represents the root of DOM. Root can be either an object or array.
	{
	  assert(file.HasMember("file"));
	  // WARNING: assert below are not always true because the related ATD member are optional and not mandatory
	  // assert(file.HasMember("path"));
	  // assert(file.HasMember("declared"));
	  // assert(file.HasMember("defined"));

	  std::string filename(file["file"].GetString());
	  std::string dirpath(file["path"].GetString());
	  std::string filepath(dirpath + "/" + filename);

	  std::cout << "file: " << filename << std::endl;
	  std::cout << "path: " << dirpath << std::endl;

	  if(file.HasMember("records"))
          {
            const rapidjson::Value& records = file["records"];
            if(records.IsArray())
            {
              // rapidjson uses SizeType instead of size_t.
              for (rapidjson::SizeType s = 0; s < records.Size(); s++)
                {
                  const rapidjson::Value& rc = records[s];
                  const rapidjson::Value& rc_name  = rc["name"];
                  const rapidjson::Value& rc_kind  = rc["kind"];
                  const rapidjson::Value& rc_debut = rc["debut"];
                  const rapidjson::Value& rc_fin   = rc["fin"];

                  std::string name = rc_name.GetString();

                  std::string kind = rc_kind.GetString();
                  clang::TagTypeKind cg_kind = (kind == "class") ? clang::TTK_Class : clang::TTK_Struct;

                  int debut = rc_debut.GetInt();
                  int fin = rc_fin.GetInt();

                  CallersData::Record record(name, cg_kind, filepath, debut, fin);

                  if(rc.HasMember("inherits"))
                  {
                    const rapidjson::Value& inherits = rc["inherits"];
                    if(inherits.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t.
                      for (rapidjson::SizeType s = 0; s < inherits.Size(); s++)
                        {
                          const rapidjson::Value& bc = inherits[s];
                          const rapidjson::Value& bc_parent = bc["record"];
                          const rapidjson::Value& bc_file   = bc["file"];
                          const rapidjson::Value& bc_debut  = bc["debut"];
                          const rapidjson::Value& bc_fin    = bc["fin"];

                          std::string parent = bc_parent.GetString();
                          std::string file = bc_file.GetString();
                          int debut = bc_debut.GetInt();
                          int fin = bc_fin.GetInt();

                          // std::cout << "Parsed parent record: \"" << parent << std::endl;
                          CallersData::Inheritance base(parent, file, debut, fin);
                          record.add_base_class(base);
                        }
                    }
                  }

                  if(rc.HasMember("inherited"))
                  {
                    const rapidjson::Value& inherited = rc["inherited"];
                    if(inherited.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t.
                      for (rapidjson::SizeType s = 0; s < inherited.Size(); s++)
                        {
                          const rapidjson::Value& bc = inherited[s];
                          const rapidjson::Value& bc_child = bc["record"];
                          const rapidjson::Value& bc_file  = bc["file"];
                          const rapidjson::Value& bc_debut = bc["debut"];
                          const rapidjson::Value& bc_fin   = bc["fin"];

                          std::string child = bc_child.GetString();
                          std::string file = bc_file.GetString();
                          int debut = bc_debut.GetInt();
                          int fin = bc_fin.GetInt();

                          // std::cout << "Parsed child record: \"" << child << std::endl;
                          CallersData::Inheritance fils(child, file, debut, fin);
                          record.add_child_class(fils);
                        }
                    }
                  }

                  if(rc.HasMember("methods"))
                  {
                    const rapidjson::Value& methods = rc["methods"];
                    if(methods.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t.
                      for (rapidjson::SizeType s = 0; s < methods.Size(); s++)
                        {
                          const rapidjson::Value& def = methods[s];
                          std::string meth_sign = def.GetString();
                          // std::cout << "Parsed method: \"" << meth_sign << std::endl;
                          record.add_method(meth_sign);
                        }
                    }
                  }

                  if(rc.HasMember("members"))
                  {
                    const rapidjson::Value& members = rc["members"];
                    if(members.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t.
                      for (rapidjson::SizeType s = 0; s < members.Size(); s++)
                        {
                          const rapidjson::Value& mb = members[s];
                          const rapidjson::Value& mb_member = mb["member"];
                          const rapidjson::Value& mb_kind = mb["kind"];

                          std::string member = mb_member.GetString();
                          std::string kind = mb_kind.GetString();
                          // std::cout << "Parsed member: \"" << member << ", kind: " << kind << std::endl;
                          record.add_member(member, kind);
                        }
                    }
                  }

                  if(rc.HasMember("calls"))
                  {
                    const rapidjson::Value& calls = rc["calls"];
                    if(calls.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t.
                      for (rapidjson::SizeType s = 0; s < calls.Size(); s++)
                        {
                          const rapidjson::Value& def = calls[s];
                          std::string called_record = def.GetString();
                          std::cout << "Parsed called record: \"" << called_record << std::endl;
                          record.add_record_call(called_record);
                        }
                    }
                  }

                  if(rc.HasMember("called"))
                  {
                    const rapidjson::Value& called = rc["called"];
                    if(called.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t.
                      for (rapidjson::SizeType s = 0; s < called.Size(); s++)
                        {
                          const rapidjson::Value& def = called[s];
                          std::string caller_record = def.GetString();
                          std::cout << "Parsed caller record: \"" << caller_record << std::endl;
                          record.add_record_called(caller_record);
                        }
                    }
                  }

                  this->get_or_create_record(&record, files);
                  // std::cout << "Parsed record r[" << s << "]:\"" << name << "\"" << std::endl;
                }
            }
          }

	  if(file.HasMember("declared"))
          {
            const rapidjson::Value& declared = file["declared"];
            if(declared.IsArray())
            {
              // rapidjson uses SizeType instead of size_t.
              for (rapidjson::SizeType s = 0; s < declared.Size(); s++)
                {
                  const rapidjson::Value& symb = declared[s];
                  const rapidjson::Value& sign = symb["sign"];
                  const rapidjson::Value& line = symb["line"];
                  const rapidjson::Value& mangledName = symb["mangled"];

                  MangledName mangled = mangledName.GetString();
                  std::string symbol = sign.GetString();
                  std::string recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
                  if(symb.HasMember("recordName"))
                  {
                    const rapidjson::Value& rec = symb["recordName"];
                    recordName = rec.GetString();
                  }
                  std::string recordFilePath = CALLERS_DEFAULT_NO_RECORD_PATH;
                  if(symb.HasMember("recordPath"))
                  {
                    const rapidjson::Value& rec = symb["recordPath"];
                    recordFilePath = rec.GetString();
                  }

                  CallersData::Virtuality virtuality;
                  if(symb.HasMember("virtuality"))
                    {
                      const rapidjson::Value& virtuality_value = symb["virtuality"];
                      std::string virtuality_text = virtuality_value.GetString();
                      virtuality =
                        ((virtuality_text == "declared") ? VVirtualDeclared
                         : (virtuality_text == "defined") ? VVirtualDefined
                         : ((virtuality_text == "pure") ? VVirtualPure : VNoVirtual));
                    }
                  else
                    {
                      virtuality = VNoVirtual;
                    }

                  int pos = line.GetInt();
                  CallersData::FctDecl fctDecl(mangled, symbol, virtuality, this->filename, pos, recordName, recordFilePath);

                  if(symb.HasMember("params"))
                  {
                    const rapidjson::Value& params = symb["params"];
                    if(params.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t
                      for (rapidjson::SizeType s = 0; s < params.Size(); s++)
                        {
                          const rapidjson::Value& param = params[s];
                          const rapidjson::Value& p_name = param["name"];
                          const rapidjson::Value& p_type = param["kind"];

                          std::string param_name = p_name.GetString();
                          std::string param_type = p_type.GetString();

                          // std::cout << "Parsed function parameter: param=\"" << param_name << "\", type=" << param_type << "\"" << std::endl;
                          CallersData::Parameter parameter(param_name, param_type);
                          fctDecl.add_parameter(parameter);
                        }
                    }
                  }

                  if(symb.HasMember("definitions"))
                  {
                    const rapidjson::Value& definitions = symb["definitions"];
                    if(definitions.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t.
                      for (rapidjson::SizeType s = 0; s < definitions.Size(); s++)
                        {
                          const rapidjson::Value& def = definitions[s];
                          std::string def_pos = def.GetString();
                          // std::cout << "Parsed function def pos: \"" << def_pos << std::endl;
                          /* std::string fct_def_pos = id_filter_prefix("local:", def_pos);
                          if(fct_def_pos != def_pos)
                          {
                            fct_def_pos = dirpath + filename + fct_def_pos;
                          }
                          // std::cout << "Completed function def pos: \"" << fct_def_pos << std::endl; */
                          fctDecl.add_definition(symbol, /*fct_*/def_pos);
                        }
                    }
                  }

                  if(symb.HasMember("redeclared"))
                  {
                    const rapidjson::Value& redeclared = symb["redeclared"];
                    if(redeclared.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t
                      for (rapidjson::SizeType s = 0; s < redeclared.Size(); s++)
                        {
                          const rapidjson::Value& redecl_method = redeclared[s];
                          const rapidjson::Value& sgn = redecl_method["sign"];
                          const rapidjson::Value& mgl = redecl_method["mangled"];
                          const rapidjson::Value& def = redecl_method["decl"];

                          std::string sign = sgn.GetString();
                          MangledName mangled = mgl.GetString();
                          std::string extfct_decl_loc = def.GetString();

                          // std::cout << "Parsed redeclared method: sign=\"" << sign << "\", def=" << extfct_decl_loc << "\"" << std::endl;
                          CallersData::ExtFctDecl redeclared_method(mangled, sign, extfct_decl_loc);
                          fctDecl.add_redeclared_method(redeclared_method);
                        }
                    }
                  }

                  if(symb.HasMember("redeclarations"))
                  {
                    const rapidjson::Value& redeclarations = symb["redeclarations"];
                    if(redeclarations.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t
                      for (rapidjson::SizeType s = 0; s < redeclarations.Size(); s++)
                        {
                          const rapidjson::Value& redecl = redeclarations[s];
                          const rapidjson::Value& sgn = redecl["sign"];
                          const rapidjson::Value& mgl = redecl["mangled"];
                          const rapidjson::Value& def = redecl["decl"];

                          std::string sign = sgn.GetString();
                          MangledName mangled = mgl.GetString();
                          std::string extfct_decl_loc = def.GetString();

                          // std::cout << "Parsed function redeclaration: sign=\"" << sign << "\", def=" << extfct_decl_loc << "\"" << std::endl;
                          CallersData::ExtFctDecl redeclaration(mangled, sign, extfct_decl_loc);
                          fctDecl.add_redeclaration(redeclaration);
                        }
                    }
                  }

                  if(symb.HasMember("locallers"))
                  {
                    const rapidjson::Value& locallers = symb["locallers"];
                    if(locallers.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t.
                      for (rapidjson::SizeType s = 0; s < locallers.Size(); s++)
                        {
                          const rapidjson::Value& def = locallers[s];
                          std::string localler = def.GetString();
                          // std::cout << "Parsed function localler: \"" << localler << std::endl;
                          fctDecl.add_local_caller(localler);
                        }
                    }
                  }

                  if(symb.HasMember("extcallers"))
                  {
                    const rapidjson::Value& extcallers = symb["extcallers"];
                    if(extcallers.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t
                      for (rapidjson::SizeType s = 0; s < extcallers.Size(); s++)
                        {
                          const rapidjson::Value& extcaller = extcallers[s];
                          const rapidjson::Value& sgn = extcaller["sign"];
                          const rapidjson::Value& mgl = extcaller["mangled"];
                          const rapidjson::Value& def = extcaller["def"];

                          std::string sign = sgn.GetString();
                          MangledName mangled = mgl.GetString();
                          std::string extfct_def_loc = def.GetString();

                          // std::cout << "Parsed function extcaller: sign=\"" << sign << "\", def=" << extfct_def_loc << "\"" << std::endl;
                          fctDecl.add_external_caller(mangled, sign, extfct_def_loc);
                        }
                    }
                  }

                  this->get_or_create_declared_function(&fctDecl, filepath, files);
                  // std::cout << "Parsed declared function s[" << s << "]:\"" << symbol << "\" in file " << this->filepath << std::endl;
                }
            }
          }

	  if(file.HasMember("defined"))
          {
            // rapidjson uses SizeType instead of size_t.
            const rapidjson::Value& defined = file["defined"];
            if(defined.IsArray())
            {
              // rapidjson uses SizeType instead of size_t.
              for (rapidjson::SizeType s = 0; s < defined.Size(); s++)
                {
                  const rapidjson::Value& symb = defined[s];
                  const rapidjson::Value& sign = symb["sign"];
                  const rapidjson::Value& def_line = symb["line"];
                  const rapidjson::Value& mangledName = symb["mangled"];

                  std::string decl_pos = CALLERS_NO_FCT_DECL_FILE;
                  std::string decl_file = CALLERS_NO_FCT_DECL_FILE;
                  int decl_line = -1;
                  if(symb.HasMember("decl"))
                  {
                    const rapidjson::Value& dc = symb["decl"];
                    decl_pos = dc.GetString();
                    decode_function_location(decl_pos, decl_file, decl_line);
                  }

                  std::string record = CALLERS_DEFAULT_NO_RECORD_NAME;
                  if(symb.HasMember("record"))
                  {
                    const rapidjson::Value& rec = symb["record"];
                    record = rec.GetString();
                  }

                  MangledName mangled = mangledName.GetString();
                  std::string symbol = sign.GetString();

                  CallersData::Virtuality virtuality;
                  if(symb.HasMember("virtuality"))
                    {
                      const rapidjson::Value& virtuality_value = symb["virtuality"];
                      std::string virtuality_text = virtuality_value.GetString();
                      virtuality =
                        ((virtuality_text == "declared") ? VVirtualDeclared
                         : (virtuality_text == "defined") ? VVirtualDefined
                         : ((virtuality_text == "pure") ? VVirtualPure : VNoVirtual));
                    }
                  else
                    {
                      virtuality = VNoVirtual;
                    }

                  int def_pos = def_line.GetInt();
                  // std::ostringstream sdef_pos;
                  // sdef_pos << def_pos;
                  // std::string def_location(filepath + ":" + sdef_pos.str());
                  // symbol_location.insert(SymbLoc::value_type(symbol, def_location));
                  CallersData::FctDef fctDef(mangled, symbol, virtuality, this->filename, def_pos, decl_file, decl_line,  record);

                  if(symb.HasMember("locallees"))
                  {
                    const rapidjson::Value& locallees = symb["locallees"];
                    if(locallees.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t.
                      for (rapidjson::SizeType s = 0; s < locallees.Size(); s++)
                        {
                          const rapidjson::Value& def = locallees[s];
                          std::string locallee = def.GetString();
                          // std::cout << "Parsed function locallee: \"" << locallee << std::endl;
                          fctDef.add_local_callee(locallee);
                        }
                    }
                  }

                  if(symb.HasMember("extcallees"))
                  {
                    const rapidjson::Value& extcallees = symb["extcallees"];
                    if(extcallees.IsArray())
                    {
                      // rapidjson uses SizeType instead of size_t
                      for (rapidjson::SizeType s = 0; s < extcallees.Size(); s++)
                        {
                          const rapidjson::Value& extcallee = extcallees[s];
                          const rapidjson::Value& sgn = extcallee["sign"];
                          const rapidjson::Value& mgl = extcallee["mangled"];
                          const rapidjson::Value& def = extcallee["decl"];

                          std::string sign = sgn.GetString();
                          MangledName mangled = mgl.GetString();
                          std::string extfct_decl_loc = def.GetString();

                          // std::cout << "Parsed function extcallee: sign=\"" << sign << "\", def=" << extfct_decl_loc << "\"" << std::endl;
                          fctDef.add_external_callee(mangled, sign, extfct_decl_loc);
                        }
                    }
                  }

                  this->add_defined_function(&fctDef, filepath, files);
                  // this->add_defined_function(mangled, symbol, virtuality, this->file, def_pos, filepath, decl_file, decl_line, record, files);
                  // std::cout << "Parsed symbol s[" << s << "]:\"" << symbol << "\"" << std::endl;
                }
            }
          }
	}
      else
	{
	  std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
	  std::cerr << "ERROR: Empty or Malformed file \"" << jsonPhysicalFilePath << "\"\n" << std::endl;
	  std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
	  exit(3);
	}
    }
}

/* WARNING: some file may not have any extension while being a valid file.
   This is the case for the header file /usr/include/c++/4.8/exception
 */
CallersData::FileKind CallersData::File::getKind(std::string filename)
{
  assert(filename != CALLERS_NO_FILE_PATH);
  std::string ext = boost::filesystem::extension(filename);
  CallersData::FileKind kind = E_UnknownFileKind;
  boost::regex headers(".h|.hh|.hpp");
  boost::regex sources(".c|.cpp|.tcc");
  //boost::cmatch m;
  if(boost::regex_match(ext,/*m,*/ headers))
  {
    kind = E_HeaderFile;
  }
  else if(boost::regex_match(ext,/*m,*/ sources))
  {
    kind = E_SourceFile;
  }

  if(ext == "")
  {
    std::cout << "CallersData.cpp:WARNING: file without any extension \"" << filename << "\". We suppose here this is a valid header file !\n" << std::endl;
    kind = E_HeaderFile;
  }

  if(kind == E_UnknownFileKind)
  {
    std::cout << "CallersData.cpp:WARNING: Unsupported file extension \"" << ext << "\"\n" << std::endl;
    assert(kind != E_UnknownFileKind);
  }
  return kind;
}

std::string CallersData::File::get_filepath() const
{
  return filepath;
}

bool CallersData::File::is_same_file(std::string otherFilePath, std::string otherFileName) const
{
  bool is_same_file = false;

  if(this->get_filepath() == otherFilePath)
  {
    is_same_file = true;
  }
  else
  {
    if(this->kind == "inc")
    {
      std::string otherfilename = CALLERS_NO_FILE_NAME;

      if(otherFileName == CALLERS_NO_FILE_NAME)
      {
        boost::filesystem::path p(otherFilePath);
        otherfilename = p.filename().string();
      }
      else
      {
        otherfilename = otherFileName;
      }
      if(boost::algorithm::ends_with(otherfilename, this->filename))
      {
        is_same_file = true;
      }
    }
  }
  return is_same_file;
}

void CallersData::File::assertSameFile(std::string otherFilePath, std::string otherFileName) const
{
  assert(this->is_same_file(otherFilePath, otherFileName));
}

std::set<CallersData::Namespace>::iterator
CallersData::File::create_or_get_namespace(std::string qualifiers, const clang::NamespaceDecl* nspc)
{
  // std::cout << "CallersData::DEBUG: Check whether the namespace \"" << qualifiers << "\" is already created or not..." << std::endl;
  std::set<CallersData::Namespace>::iterator search_result;
  CallersData::Namespace searched_nspc(qualifiers);
  search_result = namespaces->find(searched_nspc);
  if(search_result != namespaces->end())
    {
      // std::cout << "The namespace \"" << qualifiers << "\" is already present." << std::endl;
    }
  else
    {
      CallersData::Namespace c_namespace(qualifiers, *nspc);
      this->add_namespace(c_namespace);
      search_result = namespaces->find(searched_nspc);
      assert(search_result != namespaces->end());
      //std::cout << "The namespace \"" << qualifiers << "\" is well present now !" << std::endl;
    }
  return search_result;
}

void CallersData::File::add_namespace(CallersData::Namespace nspc) const
{
  std::cout << "Register namespace \"" << nspc.name
	    << "\" defined in file \"" << this->get_filepath() << std::endl;
  namespaces->insert(nspc);
}

void CallersData::File::add_record(CallersData::Record *rec) const
{
  assertSameFile(rec->file);
  std::string kind = ((rec->kind == clang::TTK_Struct) ? "struct"
		      : ((rec->kind == clang::TTK_Class) ? "class"
			 : "anonym"));
  int nb_base_classes = rec->inherits->size();
  std::cout << "Register " << kind << " record \"" << rec->name
	    << "\" with " << nb_base_classes << " base classes, "
	    << "defined in file \"" << this->get_filepath() << ":"
	    << rec->begin << ":" << rec->end << "\"" << std::endl;
  records->insert(*rec);
}

std::set<CallersData::Record>::iterator
CallersData::File::get_or_create_record(CallersData::Record* record, CallersData::Dir* files) const
{
  std::set<CallersData::Record>::iterator search_record;
  std::cout << "CallersData::File::get_or_create_record: Lookup for record \"" << record->name << "\"  from file \"" << this->get_filepath() << "\"..." << std::endl;
  if(this->is_same_file(record->file))
    // the declared function belongs to the current file
    {
      // std::cout << "The declared function belongs to the current file, so we look for it locally\n" << std::endl;
      search_record = this->get_or_create_local_record(record);
    }
  else
    // the declared function doesn't belong to the current file
    {
      // std::cout << "The declared record doesn't belong to the current file" << std::endl;
      // check first whether a json file is already present for the declared record
      // if true, parse it and return the declared record when found
      // if false, create it and return the declared record when found
      boost::filesystem::path p(record->file);
      std::string record_basename = p.filename().string();
      std::string record_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
      assert(files != NULL);
      std::set<CallersData::File>::iterator record_file = files->create_or_get_file(record_basename, record_dirpath);
      search_record = record_file->get_or_create_local_record(record);
    }
  return search_record;
}

std::set<CallersData::Record>::iterator CallersData::File::get_or_create_local_record(CallersData::Record *record) const
{
  std::set<CallersData::Record>::iterator search_record;
  std::cout << "CallersData::File::get_or_create_record: Lookup for record \"" << record->name << "\" in file \"" << this->get_filepath() << "\"" << std::endl;

  assertSameFile(record->file);

  for(search_record=records->begin(); search_record!=records->end(); ++search_record)
  {
    if(search_record->name == record->name)
    {
      std::cout << "Well found record \"" << record->name << "\" declared in file \"" << this->filename << "\"" << std::endl;
      return search_record;
    }
  }

  if(search_record == records->end())
    {
      std::cout << "CallersData::File::get_or_create_local_record:NOT_FOUND: the record \"" << record->name
                << "\" is not yet declared in file " << record->file << ", so we create it now !" << std::endl;
      this->add_record(record);
      search_record = get_or_create_local_record(record);
    }

  return search_record;
}

void CallersData::File::add_record(std::string name, clang::TagTypeKind kind, int begin, int end) const
{
  //Record *rec = new Record(name, kind, deb, fin); // fuite mémoire sur la pile si pas désalloué !
  Record rec(name, kind, this->get_filepath(), begin, end);
  records->insert(rec);
  int nb_base_classes = rec.inherits->size();
  std::cout << "Create " << kind << " record \"" << name
	    << "\" with " << nb_base_classes << " base classes, "
	    << "located in file \"" << this->get_filepath()
	    << ":" << begin << ":" << end << "\"" << std::endl;
}

/*
std::set<CallersData::Record>::iterator CallersData::File::get_record(std::string recordName, std::string recordFilePath, CallersData::Dir* files) const
{
  std::set<CallersData::Record>::iterator record;
  std::cout << "Lookup for record \"" << recordName << "\" from file \"" << this->get_filepath() << "\"..." << std::endl;
  if(this->is_same_file(recordFilePath))
    // the declared function belongs to the current file
    {
      std::cout << "The declared function belongs to the current file, so we look for it locally\n" << std::endl;
      record = this->get_local_record(recordName, recordFilePath);
    }
  else
    // the declared function doesn't belong to the current file
    {
      std::cout << "The declared function doesn't belong to the current file" << std::endl;
      // check first whether a json file is already present for the declared function
      // if true, parse it and return the declared record when found
      // if false, create it and return the declared record when found
      boost::filesystem::path p(recordFilePath);
      std::string record_basename = p.filename().string();
      std::string record_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
      assert(files != NULL);
      std::set<CallersData::File>::iterator record_file = files->create_or_get_file(record_basename, record_dirpath);
      record = record_file->get_local_record(recordName, recordFilePath);
    }
  return record;
}

std::set<CallersData::Record>::iterator CallersData::File::get_local_record(std::string recordName, std::string recordFilePath) const
{
  std::set<CallersData::Record>::iterator rc = records->begin();
  std::cout << "Lookup for record \"" << recordName << "\" in local file \"" << this->get_filepath() << "\"" << std::endl;
  assertSameFile(recordFilePath);
  for(; rc!=records->end(); ++rc)
  {
    // Check whether the recordName is a substring of the rc->name
    if(boost::starts_with(rc->name, recordName))
    {
      std::cout << "Well found record \"" << recordName << "\" as substr of rc->name=\"" << rc->name
                << "\" declared in local file \"" << this->file << "\"" << std::endl;
      return rc;
    }
    else
    {
      std::cout << "CallersData::File::get_local_record:DEBUG: rc->name=" << rc->name << " != recordName=" << recordName << std::endl;
    }
  }
  return rc;
}
*/

// Add a thread to the current file
// Do not add the thread if it already exist in the file
void CallersData::File::add_thread(CallersData::Thread *thr, CallersData::Dir *files) const
{
  std::cout << "Register thread instance \"" << thr->inst_name
	    << "\" executing the routine \"" << thr->routine_sign
             << "\"" << std::endl;
  threads->insert(*thr);

  //std::set<CallersData::FctDef>::const_iterator caller_def;
  std::set<CallersData::FctDecl>::const_iterator routine_decl;

  // Register the thread instance in the caller function def
  assert(thr->caller_file == this->filepath);
  // get a reference to the thread caller definition (if well present as expected)
  CallersData::FctDef thr_caller_def(thr->caller_mangled, thr->caller_sign, thr->caller_virtuality,
                                     thr->caller_file, thr->caller_line, thr->caller_decl_file, thr->caller_decl_line,
                                     thr->caller_recordName /*, thr->caller_recordFilePath*/);
  auto caller_def = defined->find(thr_caller_def);
  // ensure caller def has really been found
  assert(caller_def != defined->end());
  caller_def->add_thread(thr->id);

  // add the thread to the routine decl
  assert(thr->routine_file == this->filepath);
  // get a reference to the thread routine declaration (if well present as expected)
  CallersData::FctDecl thr_routine_decl(thr->routine_mangled, thr->routine_sign, thr->routine_virtuality,
                                        thr->routine_file, thr->routine_line, thr->routine_recordName, thr->routine_recordFilePath);
  routine_decl = declared->find(thr_routine_decl);
  // ensure routine decl has really been found
  assert(routine_decl != declared->end());
  routine_decl->add_thread(thr->id);

  // // add the thread to the routine definition
  // // get a reference to the thread routine definition (if well present as expected)
  // CallersData::FctDef thr_routine_def(thr->routine_mangled, thr->routine_sign, thr->routine_def_virtuality,
  //                                      thr->routine_def_file, thr->routine_def_line, thr->routine_decl_pos, thr->routine_recordName, thr->routine_recordFilePath);
  // routine_def = defined->find(thr_routine_def);
  // // ensure routine decl has really been found
  // assert(routine_def != defined->end());
  // routine_def->add_thread(thr->id);
}

void CallersData::File::add_declared_function(CallersData::FctDecl* fct, std::string fct_filepath, CallersData::Dir* files) const
{
  std::cout << "Tries to add function \"" << fct->sign
	    << "\" declared in file \"" << fct->file << ":"
	    << fct->line << "\"" << std::endl;
  assertSameFile(fct_filepath);

  declared->insert(*fct);
}

std::set<CallersData::FctDecl>::const_iterator
CallersData::File::get_or_create_local_declared_function(CallersData::FctDecl *fct_decl, std::string decl_filepath, CallersData::Dir* files) const
{
  std::cout << "Tries to get function \"" << fct_decl->sign
	    << "\" declared in file \"" << decl_filepath << "\"" << std::endl;
  assertSameFile(decl_filepath, fct_decl->file);
  std::set<CallersData::FctDecl>::iterator search_result;
  CallersData::FctDecl searched_decl(fct_decl->sign, decl_filepath);
  search_result = this->declared->find(searched_decl);
  if(search_result != this->declared->end())
    {
      std::cout << "CallersData::File::get_or_create_local_declared_function:FOUND: The function \"" << fct_decl->sign
                << "\" is already declared in file " << decl_filepath << std::endl;
    }
  else
    {
      std::cout << "CallersData::File::get_or_create_local_declared_function:NOT_FOUND: The function \"" << fct_decl->sign
                << "\" is not yet declared in file " << decl_filepath << ", so we create it now !" << std::endl;
      this->add_declared_function(fct_decl, decl_filepath, files);
      search_result = get_or_create_local_declared_function(fct_decl, decl_filepath, files);
      // at this point we should have found or created the local declared function,
      // so a valid fct_decl should have been succesfully retrieved
      assert(search_result != this->declared->end());
    }

  // complete the fct_decl with a redeclared method when needed
  this->try_to_add_redeclared_and_redeclaration_methods(*search_result, decl_filepath, files);

  return search_result;
}

std::set<CallersData::FctDecl>::const_iterator
CallersData::File::get_or_create_declared_function(CallersData::FctDecl* fct, std::string filepath, CallersData::Dir *files) const
{
  std::cout << "Register function \"" << fct->sign
	    << "\" declared in file \"" << fct->file << ":"
	    << fct->line << "\"" << std::endl;

  std::set<CallersData::FctDecl>::const_iterator fct_decl;

  // Check whether the declared function belongs to the current file.
  if(this->is_same_file(filepath))
    // the declared source function belongs to the current file
    {
      std::cout << "The declared function belongs to the current file, so we look for it locally\n" << std::endl;
      fct_decl = this->get_or_create_local_declared_function(fct, filepath, files);
    }
  else if((kind == "inc") && (fct->file == this->filename))
    // the declared function belongs to the current header file
    {
      std::cout << "The declared function belongs to the current header file, so we look for it locally\n" << std::endl;
      fct_decl = this->get_or_create_local_declared_function(fct, filepath, files);
    }
  else
    // the declared function doesn't belong to the current file
    {
      std::cout << "The declared function doesn't belong to the current file" << std::endl;
      // check first whether a json file is already present for the declared function
      // if true, parse it and add the declared function only when necessary
      // if false, create the json file and add the declared function
      boost::filesystem::path p(fct->file);
      std::string fct_decl_basename = p.filename().string();
      std::string fct_decl_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
      assert(files != NULL);
      std::set<CallersData::File>::iterator fct_decl_file = files->create_or_get_file(fct_decl_basename, fct_decl_dirpath);
      fct_decl = fct_decl_file->get_or_create_local_declared_function(fct, filepath, files);
    }
  return fct_decl;
}

bool CallersData::File::add_definition_to_declaration(std::string def_pos, std::string decl_sign, std::string decl_filepath) const
{
  assert(decl_filepath != CALLERS_NO_FCT_DECL_FILE);
  std::set<CallersData::FctDecl>::iterator search_result;
  CallersData::FctDecl searched_decl(decl_sign, decl_filepath);
  search_result = this->declared->find(searched_decl);
  if(search_result != this->declared->end())
    {
      std::cout << "add_def_to_decl: The function \"" << decl_sign << "\" is already declared in file " << decl_filepath << std::endl;
      search_result->add_definition(decl_sign, def_pos);
      return true;
    }
  else
    {
      std::cout << "add_def_to_decl: The function \"" << decl_sign << "\" is not yet declared in file " << decl_filepath << " !" << std::endl;
      return false;
    }
}

bool CallersData::File::add_definition_to_declaration(std::string def_pos, std::string decl_sign,
                                                      std::string decl_filepath, CallersData::Dir *other_files) const
{
  std::cout << "CallersData::File::add_definition_to_declaration:BEGIN: sign=\"" << decl_sign << "\" defined in \"" << def_pos << "\" is declared in file \"" << decl_filepath << "\"" << std::endl;
  assert(def_pos != CALLERS_NO_FILE_PATH);
  assert(def_pos != CALLERS_NO_FCT_DEF_FILE);
  assert(decl_filepath != CALLERS_NO_FILE_PATH);
  assert(decl_filepath != CALLERS_NO_FCT_DECL_FILE);

  // Check whether the declared function belongs to the current file
  if(this->is_same_file(decl_filepath))
    // the declared function belongs to the current file
    {
      std::cout << "The searched declared function should belong to the current file, so we look for it locally\n" << std::endl;
      return this->add_definition_to_declaration(def_pos, decl_sign, decl_filepath);
    }
  else
    // the declared function doesn't belong to the current file
    {
      std::cout << "The searched declared function does not belong to the current file, so we look for it in other files...\n" << std::endl;
      // check first whether a json file is already present for the declared function
      // if true, parse it and get the declared function when present
      boost::filesystem::path p(decl_filepath);
      std::string fct_decl_basename = p.filename().string();
      std::string fct_decl_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
      assert(other_files != NULL);
      std::set<CallersData::File>::iterator fct_decl_file = other_files->create_or_get_file(fct_decl_basename, fct_decl_dirpath);
      return fct_decl_file->add_definition_to_declaration(def_pos, decl_sign, decl_filepath, other_files);
    }
}

void CallersData::File::add_defined_function(CallersData::FctDef* fct_def, std::string fct_def_filepath, CallersData::Dir *otherJsonFiles) const
{
  std::cout << "Register function \"" << fct_def->sign
            << "\" defined in file \"" << fct_def->def_file << ":"
            << fct_def->def_line << "\"" << std::endl;

  // Check whether the defined function has well to be added to the current file.
  if(!this->is_same_file(fct_def_filepath, fct_def->def_file))
  {
    // the defined function does not belong to the current file

    std::cout << "CallersData::File::add_defined_function:WARNING: The defined function \"" << fct_def->sign << "\" doesn't belong to the current file: "
              << this->get_filepath() << ", so we try to open the right file..." << std::endl;

    // check whether a json file is already present for the visited defined function
    // if true, parse it and add the defined function only when necessary
    // if false, create this json file and add the defined function
    {
      boost::filesystem::path p(fct_def_filepath);
      std::string basename = p.filename().string();
      std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
      std::set<CallersData::File>::iterator file = otherJsonFiles->create_or_get_file(basename, dirpath);
      file->add_defined_function(fct_def, fct_def_filepath, otherJsonFiles);
    }
  }

  // the defined function should here belong to the current file
  {
    std::cout << "The defined function belongs to the current file, so we add it directly\n" << std::endl;
    defined->insert(*fct_def);
    if(fct_def->decl_file != CALLERS_NO_FCT_DECL_FILE)
    {
      std::ostringstream sdef_pos;
      sdef_pos << fct_def->def_line;
      if(fct_def->decl_file == CALLERS_LOCAL_FCT_DECL)
      {
        std::string def_pos = std::string(CALLERS_LOCAL_FCT_DECL) + ":" + sdef_pos.str();
        this->add_definition_to_declaration(def_pos, fct_def->sign, filepath, otherJsonFiles);
      }
      else
      {
        std::string def_pos = filepath + ":" + sdef_pos.str();
        this->add_definition_to_declaration(def_pos, fct_def->sign, fct_def->decl_file, otherJsonFiles);
      }
    }
  }
}

void CallersData::File::add_defined_function(MangledName fct_mangled, std::string fct_sign, Virtuality fct_virtuality,
					     std::string fct_def_file, int fct_def_line, std::string fct_def_filepath,
                                             std::string fct_decl_file, int fct_decl_line, std::string record, CallersData::Dir *otherJsonFiles) const
{
  FctDef fct_def(fct_mangled, fct_sign, fct_virtuality, fct_def_file, fct_def_line, fct_decl_file, fct_decl_line, record);

  // Check whether the defined function has well to be added to the current file.
  if(!this->is_same_file(fct_def_filepath, fct_def_file))
  {
    // the defined function does not belong to the current file

    std::cout << "CallersData::File::add_defined_function:WARNING: The defined function \"" << fct_sign << "\" doesn't belong to the current file: "
              << this->get_filepath() << ", so we try to open the right file..." << std::endl;

    // check whether a json file is already present for the visited defined function
    // if true, parse it and add the defined function only when necessary
    // if false, create this json file and add the defined function
    {
      boost::filesystem::path p(fct_def_file);
      std::string basename = p.filename().string();
      std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
      std::set<CallersData::File>::iterator file = otherJsonFiles->create_or_get_file(basename, dirpath);
      file->add_defined_function(&fct_def, fct_def_file, otherJsonFiles);
    }
  }

  // the defined function should here belong to the current file
  {
    if(record == CALLERS_DEFAULT_RECORD_NAME)
    {
      std::cout << "CallersData::File::add_defined_function: Create function definition \"" << fct_sign
                << "\" located in file \"" << fct_def_file << ":" << fct_def_line << "\"" << std::endl;
    }
    else {
      std::cout << "CallersData::File::add_defined_function: Create " << record << " method definition \"" << fct_sign
                << "\" located in file \"" << fct_def_file << ":" << fct_def_line << "\"" << std::endl;
    }
    this->add_defined_function(&fct_def, fct_def_filepath, otherJsonFiles);
  }
}

void
CallersData::File::add_function_call(CallersData::FctCall* fc, CallersData::Dir *files) const
{
  std::cout << "Register function call from caller \"" << fc->caller.def_file << ":" << fc->caller.sign
            << "\" to callee \"" << fc->callee.file << ":" << fc->callee.sign
	    << "\" in file \"" << this->get_filepath() << "\"" << std::endl;
  std::cout << "caller sign: " << fc->caller.sign << std::endl;
  std::cout << "caller virtual: "
	    << ((fc->caller.virtuality == CallersData::VNoVirtual) ? "no"
	    	: ((fc->caller.virtuality == CallersData::VVirtualDeclared) ? "declared"
	    	: ((fc->caller.virtuality == CallersData::VVirtualDefined) ? "defined"
		: "pure")))
            << std::endl;
  std::cout << "callee sign: " << fc->callee.sign << std::endl;
  std::cout << "callee virtual: "
	    << ((fc->callee.virtuality == CallersData::VNoVirtual) ? "no"
	    	: ((fc->callee.virtuality == CallersData::VVirtualDeclared) ? "declared"
	    	: ((fc->callee.virtuality == CallersData::VVirtualDefined) ? "defined"
		: "pure")))
            << std::endl;
  std::cout << "current file: " << this->get_filepath() << std::endl;
  std::cout << "caller def pos: " << fc->caller.def_file << ":" << fc->caller.def_line << std::endl;
  std::cout << "callee decl pos: " << fc->callee.file << ":" << fc->callee.line << std::endl;

  calls->insert(*fc);

  // // complete the callee's fct_decl with the redeclared method when needed
  // this->add_redeclared_method(&fc->callee, fc->callee.file, files);

  // // complete the caller's fct_decl with a redeclaration when needed
  // this->add_redeclaration(&fc->caller, fc->caller.def_file, files);

  std::set<CallersData::FctDef>::const_iterator caller;
  std::set<CallersData::FctDecl>::const_iterator callee;

  // Check whether the caller function belongs to the current file.
  if(this->is_same_file(fc->caller.def_file))

    // the caller function belongs to the current file
    {
      std::cout << "The caller function belongs to the current file" << std::endl;

      // adds the caller function to the defined functions of the current file
      add_defined_function(fc->caller.mangled, fc->caller.sign, fc->caller.virtuality, fc->caller.def_file, fc->caller.def_line, fc->caller.def_file, fc->caller.decl_file, fc->caller.decl_line, fc->caller.record, files);
      // get a reference to the related defined function
      CallersData::FctDef caller_fct(fc->caller.mangled, fc->caller.sign, fc->caller.virtuality, fc->caller.def_file, fc->caller.def_line, fc->caller.decl_file, fc->caller.decl_line, fc->caller.record);
      caller = defined->find(caller_fct);
      // ensure caller has really been found
      assert(caller != defined->end());

      // Check whether the callee function belongs to the current file.
      if(this->is_same_file(fc->callee.file))

	// the callee function belongs to the current file
	{
	  std::cout << "The callee function belongs to the current file" << std::endl;

	  // adds the callee function to the declared functions of the current file
          CallersData::FctDecl fctDecl(fc->callee.mangled, fc->callee.sign, fc->callee.virtuality, fc->callee.file,
                                       fc->callee.line, fc->callee.recordName, fc->callee.recordFilePath);
          this->get_or_create_declared_function(&fctDecl, fc->callee.file, files);

	  // get a reference to the related declared function
	  CallersData::FctDecl callee_fct(fc->callee.mangled, fc->callee.sign, fc->callee.virtuality, fc->callee.file,
                                          fc->callee.line, fc->callee.recordName, fc->callee.recordFilePath);
	  callee = declared->find(callee_fct);
	  // ensure callee has really been found
	  assert(callee != declared->end());

	  // add local caller to local callee
	  callee->add_local_caller(fc->caller.sign);

	  // add local callee to local caller
	  caller->add_local_callee(fc->callee.sign);
	}
      else
	// the callee function is defined externally
	{
	  std::cout << "The callee function is defined externally" << std::endl;

	  // check first whether a json file is already present for the callee function
	  // if true, parse it and add the defined function only when necessary
	  // if false, create the callee json file and add the defined function
	  boost::filesystem::path p(fc->callee.file);
	  std::string callee_basename = p.filename().string();
	  std::string callee_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
	  std::set<CallersData::File>::iterator callee_file = files->create_or_get_file(callee_basename, callee_dirpath);
	  //CallersData::File callee_file(callee_basename, callee_dirpath);
	  //callee_file.parse_json_file();
	  CallersData::FctDecl callee_decl(fc->callee.mangled, fc->callee.sign, fc->callee.virtuality, fc->callee.file,
                                           fc->callee.line, fc->callee.recordName, fc->callee.recordFilePath, fc->callee.is_builtin);
	  callee_file->get_or_create_declared_function(&callee_decl, fc->callee.file, files);

	  // get a reference to the related defined function
	  callee = callee_file->declared->find(callee_decl);
	  // ensure callee has really been found
	  assert(callee != callee_file->declared->end());

	  if(fc->callee.is_builtin == true)
	    {
              // add the builtin callee to the local caller
	      caller->add_builtin_callee(fc->callee.mangled, fc->callee.sign, fc->callee.virtuality, fc->callee.file, fc->callee.line);

	      // add the local caller to the builtin callee
	      callee->add_external_caller(fc->caller.mangled, fc->caller.sign, fc->caller.def_file, fc->caller.def_line);
	    }
	  else
            {
              // add the external callee to the local caller
              caller->add_external_callee(fc->callee.mangled, fc->callee.sign, fc->callee.file, fc->callee.line);

	      // add the local caller to the external callee
	      callee->add_external_caller(fc->caller.mangled, fc->caller.sign, fc->caller.def_file, fc->caller.def_line);
            }
	}
    }

  else
    // the caller function does not belong to the current file
    {
      std::cout << "The caller function does not belong to the current file" << std::endl;

      // check first whether a json file is already present for the caller function
      // if true, parse it and add the defined function only when necessary
      // if false, create the caller json file and add the defined function
      boost::filesystem::path p(fc->caller.def_file);
      std::string caller_basename = p.filename().string();
      std::string caller_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
      std::set<CallersData::File>::iterator caller_file = files->create_or_get_file(caller_basename, caller_dirpath);
      // CallersData::File caller_file(caller_basename, caller_dirpath);
      // caller_file.parse_json_file();
      CallersData::FctDef caller_def(fc->caller.mangled, fc->caller.sign, fc->caller.virtuality,
                                     fc->caller.def_file, fc->caller.def_line, fc->caller.decl_file, fc->caller.decl_line, fc->caller.record);
      caller_file->add_defined_function(&caller_def, fc->caller.def_file, files);
      // get a reference to the related defined function
      caller = caller_file->defined->find(caller_def);
      // ensure caller has really been found
      assert(caller != caller_file->defined->end());

      // check whether the callee function belongs to the current file or not
      if(this->is_same_file(fc->callee.file))

	// the callee function belongs to the current file
	{
	  std::cout << "The callee function belongs to the current file" << std::endl;

	  // adds the callee function to the declared functions of the current file
          CallersData::FctDecl fctDecl(fc->callee.mangled, fc->callee.sign, fc->callee.virtuality, fc->callee.file, fc->callee.line, fc->callee.recordName, fc->callee.recordFilePath);
	  this->get_or_create_declared_function(&fctDecl, fc->callee.file, files);

	  // get a reference to the related defined function
	  CallersData::FctDecl callee_fct(fc->callee.mangled, fc->callee.sign, fc->callee.virtuality, fc->callee.file, fc->callee.line, fc->callee.recordName, fc->callee.recordFilePath);
	  callee = declared->find(callee_fct);
	  // ensure callee has really been found
	  assert(callee != declared->end());

	  // add the external caller to the local callee
	  callee->add_external_caller(fc->caller.mangled, fc->caller.sign, fc->caller.def_file, fc->caller.def_line);

	  // add the local callee to the external caller
	  caller->add_external_callee(fc->caller.mangled, fc->callee.sign, fc->callee.file, fc->callee.line);
	}
      else
	// the callee function is defined externally as the caller !!!
	{
	  std::cout << "The callee function is defined externally as the caller !!!" << std::endl;

          // check first whether a json file is already present for the callee function
          // if true, parse it and add the defined function only when necessary
          // if false, create the callee json file and add the defined function
          boost::filesystem::path p(fc->callee.file);
          std::string callee_basename = p.filename().string();
          std::string callee_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
          std::set<CallersData::File>::iterator callee_file = files->create_or_get_file(callee_basename, callee_dirpath);
          //CallersData::File callee_file(callee_basename, callee_dirpath);
          //callee_file.parse_json_file();
          CallersData::FctDecl callee_decl(fc->callee.mangled, fc->callee.sign, fc->callee.virtuality, fc->callee.file, fc->callee.line, fc->callee.recordName, fc->callee.recordFilePath);
          callee_file->get_or_create_declared_function(&callee_decl, fc->callee.file, files);

	  // get a reference to the related defined function
	  callee = callee_file->declared->find(callee_decl);
	  // ensure callee has really been found
	  assert(callee != callee_file->declared->end());

	  // check whether the caller and callee functions are collocated or not
	  if( fc->caller.def_file == fc->callee.file )
	    {
	      std::cout << "The caller and callee functions are collocated in the same file" << std::endl;

	      // add the local caller to the local callee
	      callee->add_local_caller(fc->caller.sign);

	      // add the local callee to the local caller
	      caller->add_local_callee(fc->callee.sign);
	    }
	  else
	    {
	      std::cout << "The caller and callee functions are located in different files" << std::endl;

	      // add the external caller to the external callee
	      callee->add_external_caller(fc->caller.mangled, fc->caller.sign, fc->caller.def_file, fc->caller.def_line);

	      // add the external callee to the external caller
	      caller->add_external_callee(fc->callee.mangled, fc->callee.sign, fc->callee.file, fc->callee.line);
	    }
	}
    }

    // Check whether the caller function is a method of a record different from callee method
    if((fc->caller.record != fc->callee.recordName)&&
       (fc->caller.record != CALLERS_DEFAULT_NO_RECORD_NAME))
    {
       // get the fct caller's record declaration
      CallersData::Record caller_rc(fc->caller.record, fc->caller.decl_file);
      std::set<CallersData::Record>::iterator caller_record = this->get_or_create_record(&caller_rc, files);
      assert(caller_record != records->end());

      // Check whether the called function is a method
      if((fc->callee.recordName != CALLERS_DEFAULT_NO_RECORD_NAME) &&
         (fc->callee.recordFilePath != CALLERS_DEFAULT_NO_RECORD_PATH))
        {
          // Add a record call to the caller record
          caller_record->add_record_call(fc->callee.recordName);

           // get the called function's record declaration
          CallersData::Record callee_rc(fc->callee.recordName, fc->callee.recordFilePath);
          std::set<CallersData::Record>::iterator callee_record = this->get_or_create_record(&callee_rc, files);
          assert(callee_record != records->end());

          // Add a record called to the callee record
          callee_record->add_record_called(fc->caller.record);
        }
    }
}

void CallersData::File::try_to_add_redeclared_and_redeclaration_methods(CallersData::Dir* files) const
{
  // browse file functions declarations
  std::set<FctDecl>::const_iterator d;
  for(d=declared->begin(); d!=declared->end(); ++d)
    {
      this->try_to_add_redeclared_and_redeclaration_methods(*d, this->filepath, files);
    }
}

void CallersData::File::try_to_add_redeclared_and_redeclaration_methods(const FctDecl& fct_decl, std::string fct_filepath, CallersData::Dir* files) const
{
  // Check whether the input function decl is a method and its record is well defined
  if((fct_decl.recordName != CALLERS_DEFAULT_NO_RECORD_NAME) &&
     (fct_decl.recordName != CALLERS_DEFAULT_RECORD_BUILTIN))
  {
    std::cout << "CallersData::File::try_to_add_local_redeclared_and_redeclaration_methods:DEBUG: sign=\"" << fct_decl.sign
              << "\", recordName=\"" << fct_decl.recordName << "\""
              << "\", recordFile=\"" << fct_decl.recordFilePath << "\""
              << "\", fct_file=\"" << fct_filepath << "\"" << std::endl;

    // check consistency of record file path
    assert(fct_decl.recordFilePath != CALLERS_DEFAULT_RECORD_PATH);
    assert(fct_decl.recordFilePath != CALLERS_DEFAULT_NO_RECORD_PATH);

    // Make sure the fct_decl belongs to the current file
    assertSameFile(fct_filepath, fct_decl.file);

    // the fct_decl's belongs to the current file
    std::cout << "The fct_decl \"" << fct_decl.sign << "\" belongs to the current file" << std::endl;

    // get the fct_decl's record declaration
    CallersData::Record rc(fct_decl.recordName, fct_decl.recordFilePath);
    std::set<CallersData::Record>::iterator record = this->get_or_create_record(&rc, files);
    assert(record != records->end());

    // check whether the current method is a redeclared method
    auto redecl_method = record->get_redeclared_method(fct_decl.sign);
    if(redecl_method != record->redeclared_methods->end())
    {
      std::string redecl_method_pos = CALLERS_NO_FCT_DECL_FILE;
      std::string redecl_method_file = CALLERS_NO_FCT_DECL_FILE;
      int redecl_method_line = -1;
      decode_function_location(redecl_method->second.fctLoc, redecl_method_file, redecl_method_line);

      // get a reference to the base virtual method
      CallersData::FctDecl search_base_virt_method(redecl_method->second.sign, redecl_method_file);

      // get a reference to the virtual method declaration specified in input
      std::set<CallersData::FctDecl>::const_iterator
      base_virt_method = this->get_or_create_declared_function(&search_base_virt_method, redecl_method_file, files);

      // add a redeclared method to the child method declaration
      {
        CallersData::ExtFctDecl base_virtual_method( redecl_method->second.mangled, redecl_method->second.sign, redecl_method->second.fctLoc);
        fct_decl.add_redeclared_method(base_virtual_method);
      }

      // add a redeclaration to the base virtual method
      {
        std::ostringstream pos;
        pos << fct_decl.line;
        std::string fct_decl_pos = fct_decl.file + ":" + pos.str();
        CallersData::ExtFctDecl child_redeclared_method(fct_decl.mangled, fct_decl.sign, fct_decl_pos);
        base_virt_method->add_redeclaration(child_redeclared_method);
      }
    }
  }
}

/*
void CallersData::File::add_redeclared_method(FctDecl *fct_decl, std::string fct_filepath, Dir* files) const
{
  assert(files != NULL);
  // Check whether the input function decl is a method and its record is well defined
  if((fct_decl->recordName != CALLERS_DEFAULT_NO_RECORD_NAME) &&
     (fct_decl->recordName != CALLERS_DEFAULT_RECORD_BUILTIN))
  {
    assert(fct_decl->recordFilePath != CALLERS_DEFAULT_RECORD_PATH);
    assert(fct_decl->recordFilePath != CALLERS_DEFAULT_NO_RECORD_PATH);

    std::cout << "CallersData::File::add_redeclared_method:DEBUG: sign=\"" << fct_decl->sign << "\", record=\"" << fct_decl->recordName << "\"" << std::endl;

    // Check whether the fct_decl belongs to the current file
    if(this->is_same_file(fct_decl->filepath, fct_filename))

      // the fct_decl's record belongs to the current file
      {
        std::cout << "The fct_decl's record \"" << fct_decl->recordName << "\" belongs to the current file" << std::endl;

        // get the fct_decl's record declaration
        std::set<CallersData::Record>::iterator record = this->get_record(fct_decl->recordName);
        assert(record != records->end());

        // check whether the current method is a redeclared method
        auto redecl_method = record->get_redeclared_method(fct_decl->sign);
        if(redecl_method != record->redeclared_methods->end())
        {
          // get a reference to the function declaration specified in input
          std::set<CallersData::FctDecl>::const_iterator fct_decl_ref = this->get_declared_function(fct_decl->sign, fct_filepath);

          // add a redeclared method to the input function declaration
          fct_decl_ref->add_redeclared_method(redecl_method->second);
        }
      }
    else
      // the fct_decl is defined externally
      {
        std::cout << "The fct_decl is defined externally" << std::endl;

        // check first whether a json file is already present for the fct_decl
        // if true, parse it and add the redeclared method only when necessary
        // if false, create the callee json file and add the redeclared method
        boost::filesystem::path p(fct_filepath);
        std::string fct_basename = p.filename().string();
        std::string fct_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
        std::set<CallersData::File>::iterator fct_file = files->create_or_get_file(fct_basename, fct_dirpath);

        assert(fct_filepath == fct_file->get_filepath());
        fct_file->add_redeclared_method(fct_decl, fct_filepath, files);
      }
  }
}
*/

/*
void CallersData::File::add_redeclaration(FctDecl *fct_decl, std::string fct_filepath, Dir* files) const
{
  assert(files != NULL);
  // Check whether the input function decl is a method and its record is well defined
  if((fct_decl->recordName != CALLERS_DEFAULT_NO_RECORD_NAME) &&
     (fct_decl->recordName != CALLERS_DEFAULT_RECORD_BUILTIN))
  {
    assert(fct_decl->recordFilePath != CALLERS_DEFAULT_RECORD_PATH);
    assert(fct_decl->recordFilePath != CALLERS_DEFAULT_NO_RECORD_PATH);

    std::cout << "CallersData::File::add_redeclaration:DEBUG: sign=\"" << fct_decl->sign << "\", record=\"" << fct_decl->recordName << "\"" << std::endl;

    // Check whether the fct_decl belongs to the current file
    if(this->is_same_file(fct_decl->filepath, fct_filename))

      // the fct_decl's record belongs to the current file
      {
        std::cout << "The fct_decl's record \"" << fct_decl->recordName << "\" belongs to the current file" << std::endl;

        // get the fct_decl's record declaration
        std::set<CallersData::Record>::iterator record = this->get_record(fct_decl->recordName, fct_decl->recordFilePath, files);
        assert(record != records->end());

        // check whether the current method is a redeclaration
        auto redecl_method = record->get_redeclaration(fct_decl->sign);
        if(redecl_method != record->redeclarations->end())
        {
          // get a reference to the function declaration specified in input
          std::set<CallersData::FctDecl>::const_iterator fct_decl_ref = this->get_or_create_local_declared_function(fct_decl, fct_filepath, files);

          // add a redeclaration to the input function declaration
          fct_decl_ref->add_redeclaration(redecl_method->second);
        }
      }
    else
      // the fct_decl is defined externally
      {
        std::cout << "The fct_decl is defined externally" << std::endl;

        // check first whether a json file is already present for the fct_decl
        // if true, parse it and add the redeclaration only when necessary
        // if false, create the callee json file and add the redeclaration
        boost::filesystem::path p(fct_filepath);
        std::string fct_basename = p.filename().string();
        std::string fct_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
        std::set<CallersData::File>::iterator fct_file = files->create_or_get_file(fct_basename, fct_dirpath);

        assert(fct_filepath == fct_file->get_filepath());
        fct_file->add_redeclaration(fct_decl, fct_filepath, files);
      }
  }
}
*/

void CallersData::File::output_json_desc() const
{
  CallersData::JsonFileWriter js(this->jsonLogicalFilePath, this->jsonPhysicalFilePath);

  js.out
    //<< "{\"eClass\":\"" << CALLERS_TYPE_FILE << "\",\"file\":\"" << file
    << "{\"file\":\"" << filename
    << "\",\"kind\":\"" << kind
    << "\",\"path\":\"" << dirpath
    << "\"";

  if(namespaces->size() > 0)
  {
	  js.out << "\",\"namespaces\":[";
	  std::set<Namespace>::const_iterator n, last_nspc;
	  last_nspc = namespaces->empty() ? namespaces->end() : --namespaces->end();
	  for(n=namespaces->begin(); n!=namespaces->end(); ++n)
		{
		  if(n != last_nspc)
			{
			  n->output_json_desc(js.out);
			  js.out << ",";
			}
			  else
			{
			  n->output_json_desc(js.out);
			}
		}
	  js.out << "]";
  }

  if(records->size() > 0)
  {
	  js.out << ",\"records\":[";
	  std::set<Record>::const_iterator r, last_rec;
	  last_rec = records->empty() ? records->end() : --records->end();
	  for(r=records->begin(); r!=records->end(); ++r)
		{
		  if(r != last_rec)
			{
			  r->output_json_desc(js.out);
			  js.out << ",";
			}
			  else
			{
			  r->output_json_desc(js.out);
			}
	   }
	  js.out << "]";
  }

  if(threads->size() > 0)
  {
    js.out << ",\"threads\":[";
    std::set<Thread>::const_iterator thr, last_thr;
    last_thr = threads->empty() ? threads->end() : --threads->end();
    for(thr=threads->begin(); thr!=threads->end(); ++thr)
    {
      if(thr != last_thr)
      {
        thr->output_json_desc(js.out);
        js.out << ",";
      }
      else
      {
        thr->output_json_desc(js.out);
      }
    }
    js.out << "]";
  }

  if(declared->size() > 0)
  {
  js.out << ",\"declared\":[";
  std::set<FctDecl>::const_iterator d, last_decl;
  last_decl = declared->empty() ? declared->end() : --declared->end();
  for(d=declared->begin(); d!=declared->end(); ++d)
    {
      if(d != last_decl)
		{
		  d->output_json_desc(js.out);
		  js.out << ",";
		}
		  else
		{
		  d->output_json_desc(js.out);
		}
    }
	  js.out << "]";
  }

  if(defined->size() > 0)
  {
	  js.out << ",\"defined\":[";
	  std::set<FctDef>::const_iterator i, last_fct;
	  last_fct = defined->empty() ? defined->end() : --defined->end();
	  for(i=defined->begin(); i!=defined->end(); ++i)
		{
		  if(i != last_fct)
			{
			  i->output_json_desc(js.out);
			  js.out << ",";
			}
			  else
			{
			  i->output_json_desc(js.out);
			}
		}
	  js.out << "]";
  }

  js.out << "}" << std::endl;
}

bool CallersData::operator< (const CallersData::File& file1, const CallersData::File& file2)
{
  return file1.get_filepath() < file2.get_filepath();
}

/***************************************** class Namespace ****************************************/

void CallersData::Namespace::allocate()
{

}

CallersData::Namespace::Namespace(std::string qualifier)
  : qualifier(qualifier)
{
  allocate();
  std::cout << "Create namespace: " << std::endl;
  std::vector<std::string> namespaces;
  boost::algorithm::split_regex(namespaces, qualifier, boost::regex("::"));
  std::vector<std::string>::iterator nspc = namespaces.end();
  name = *nspc;
  this->print_cout();
}

CallersData::Namespace::Namespace(std::string qualifier, const clang::NamespaceDecl& nspc)
  : qualifier(qualifier)
{
  allocate();
  std::cout << "Create namespace: " << std::endl;
  name = nspc.getNameAsString();
  this->print_cout();
}

CallersData::Namespace::Namespace(const CallersData::Namespace& copy_from_me)
{
  allocate();
  std::cout << "Namespace copy constructor" << std::endl;
  name = copy_from_me.name;
  qualifier = copy_from_me.qualifier;

  // std::set<Namespace>::const_iterator n;
  // for(n=copy_from_me.namespaces->begin(); n!=copy_from_me.namespaces->end(); ++n)
  //   {
  //     namespaces->insert(*n);
  //   }

  // std::set<Record>::const_iterator b;
  // for(b=copy_from_me.records->begin(); b!=copy_from_me.records->end(); ++b)
  //   {
  //     records->insert(*b);
  //   }
}

CallersData::Namespace::~Namespace()
{
  // delete namespaces;
  // delete records;
}

std::string
CallersData::Namespace::get_qualifier() const
{
  return qualifier;
}

// void CallersData::Namespace::add_namespace(CallersData::Namespace nspc) const
// {
//   namespaces->insert(nspc);
//   std::cout << "Register nested namespace \"" << nspc.name
// 	    << " in namespace " << name
// 	    << ", nb_namespaces=" << namespaces->size()
// 	    << std::endl;
// }

// void CallersData::Namespace::add_namespace(std::string qualifier, const clang::NamespaceDecl& namespc) const
// {
//   //Namespace *namespace = new Namespace(name, kind, deb, fin); // fuite mémoire sur la pile si pas désalloué !
//   Namespace nspc(qualifier, namespc);
//   namespaces->insert(nspc);
//   std::cout << "Create nested namespace \"" << namespc.getNameAsString()
// 	    << " in namespace " << this->name
// 	    << ", nb_namespaces=" << namespaces->size()
// 	    << std::endl;
// }

// void CallersData::Namespace::add_record(CallersData::Record record) const
// {
//   records->insert(record);
//   std::cout << "Register record \"" << record.name
// 	    << " in namespace " << name
// 	    << ", nb_records=" << records->size()
// 	    << std::endl;
// }

// void CallersData::Namespace::add_record(std::string name, clang::TagTypeKind kind, int loc) const
// {
//   //Record *record = new Record(name, kind, deb, fin); // fuite mémoire sur la pile si pas désalloué !
//   Record record(name, kind, loc);
//   records->insert(record);
//   std::cout << "Create record \"" << name
// 	    << " in namespace " << this->name
// 	    << ", nb_records=" << records->size()
// 	    << std::endl;
// }

void CallersData::Namespace::print_cout() const
{
  std::cout << "{\"name\": \"" << name
	    << "\",\"qualifier\": \"" << qualifier << "\"";

  // std::cout << ",\"namespaces\":[";
  // std::set<Namespace>::const_iterator n, last_nspc;
  // last_nspc = namespaces->empty() ? namespaces->end() : --namespaces->end();
  // for(n=namespaces->begin(); n!=namespaces->end(); ++n)
  //   {
  //     if(n != last_nspc)
  // 	{
  // 	  n->print_cout();
  // 	  std::cout << ",";
  // 	}
  //     else
  // 	{
  // 	  n->print_cout();
  // 	}
  //   }

  // std::cout << "],\"records\":[";
  // std::set<Record>::const_iterator b, last_bc;
  // last_bc = records->empty() ? records->end() : --records->end();
  // for(b=records->begin(); b!=records->end(); ++b)
  //   {
  //     if(b != last_bc)
  // 	{
  // 	  b->print_cout();
  // 	  std::cout << ",";
  // 	}
  //     else
  // 	{
  // 	  b->print_cout();
  // 	}
  //   }
  // std::cout << "]";

  std::cout << "}";
}

void CallersData::Namespace::output_json_desc(std::ofstream &js) const
{
  js << "{\"name\":\"" << name
     << "\",\"qualifier\":\"" << qualifier << "\"}";

  // js << "\"namespaces\":[";
  // std::set<Namespace>::const_iterator n, last_nspc;
  // last_nspc = namespaces->empty() ? namespaces->end() : --namespaces->end();
  // for(n=namespaces->begin(); n!=namespaces->end(); ++n)
  //   {
  //     if(n != last_nspc)
  // 	{
  // 	  n->output_json_desc(js);
  // 	  js << ",";
  // 	}
  //     else
  // 	{
  // 	  n->output_json_desc(js);
  // 	}
  //   }
  // js << "]";

  // js << ",\"records\":[";
  // std::set<Record>::const_iterator b, last_bc;
  // last_bc = records->empty() ? records->end() : --records->end();
  // for(b=records->begin(); b!=records->end(); ++b)
  //   {
  //     if(b != last_bc)
  // 	{
  // 	  b->output_json_desc(js);
  // 	  js << ",";
  // 	}
  //     else
  // 	{
  // 	  b->output_json_desc(js);
  // 	}
  //   }
  // js << "]}";
}

bool CallersData::operator< (const CallersData::Namespace& nspc1, const CallersData::Namespace& nspc2)
{
  return nspc1.qualifier < nspc2.qualifier;
}

/***************************************** class Inheritance ****************************************/

CallersData::Inheritance::Inheritance(const char* name, const char* file, int begin, int end)
  : name(name),
    file(file),
    begin(begin),
    end(end)
{
  std::cout << "Create class reference: " << std::endl;
  this->print_cout();
}

CallersData::Inheritance::Inheritance(std::string name, std::string file, int begin, int end)
  : name(name),
    file(file),
    begin(begin),
    end(end)
{
  std::cout << "Create class reference: " << std::endl;
  this->print_cout();
}

CallersData::Inheritance::Inheritance(const CallersData::Inheritance& copy_from_me)
{
  std::cout << "Inheritance copy constructor" << std::endl;
  name = copy_from_me.name;
  file = copy_from_me.file;
  begin = copy_from_me.begin;
  end = copy_from_me.end;
}

void CallersData::Inheritance::print_cout() const
{
  std::ostringstream deb;
  deb << begin;
  std::ostringstream fin;
  fin << end;
  std::cout << "{\"record\":\"" << name
	    << "\",\"file\":\"" << file
	    << "\",\"debut\":" << begin
	    << ",\"fin\":" << end
	    << std::endl;
}

void CallersData::Inheritance::output_json_desc(std::ofstream &js) const
{
  std::ostringstream deb;
  deb << begin;
  std::ostringstream fin;
  fin << end;
  js << "{\"record\":\"" << name
     << "\",\"file\":\"" << file
     << "\",\"debut\":" << begin
     << ",\"fin\":" << end << "}";
}

bool CallersData::operator< (const CallersData::Inheritance& inheritance1,
			     const CallersData::Inheritance& inheritance2)
{
  return inheritance1.name < inheritance2.name;
}

/***************************************** class Record ****************************************/

void CallersData::Record::allocate()
{
  inherits = new std::set<CallersData::Inheritance>;
  inherited = new std::set<CallersData::Inheritance>;
  methods = new std::set<std::string>;
  members = new std::set<std::pair<std::string, std::string>>;
  redeclared_methods = new std::set<std::pair<std::string, CallersData::ExtFctDecl>>;
  redeclarations = new std::set<std::pair<std::string, CallersData::ExtFctDecl>>;
  calls = new std::set<std::string>;
  called = new std::set<std::string>;
}

CallersData::Record::~Record()
{
  delete inherits;
  delete inherited;
  delete methods;
  delete members;
  delete redeclared_methods;
  delete redeclarations;
  delete calls;
  delete called;
}

CallersData::Record::Record(std::string name, std::string file)
  : name(name),
    file(file)
{
  allocate();
  std::cout << "Create record: " << std::endl;
  this->print_cout();
}

CallersData::Record::Record(std::string name, clang::TagTypeKind kind, std::string file, int begin, int end)
  : name(name),
    kind(kind),
    file(file),
    begin(begin),
    end(end)
{
  allocate();
  std::cout << "Create record: " << std::endl;
  this->print_cout();
}

CallersData::Record::Record(const CallersData::Record& copy_from_me)
{
  allocate();
  std::cout << "Record copy constructor" << std::endl;
  name = copy_from_me.name;
  kind = copy_from_me.kind;
  file = copy_from_me.file;
  begin = copy_from_me.begin;
  end = copy_from_me.end;

  std::set<Inheritance>::const_iterator b;
  for(b=copy_from_me.inherits->begin(); b!=copy_from_me.inherits->end(); ++b)
    {
      inherits->insert(*b);
    }

  for(b=copy_from_me.inherited->begin(); b!=copy_from_me.inherited->end(); ++b)
    {
      inherited->insert(*b);
    }

  std::set<std::string>::const_iterator m;
  for(m=copy_from_me.methods->begin(); m!=copy_from_me.methods->end(); ++m)
    {
      methods->insert(*m);
    }

  std::set<std::pair<std::string, std::string>>::const_iterator f;
  for(f=copy_from_me.members->begin(); f!=copy_from_me.members->end(); ++f)
    {
      members->insert(*f);
    }

  std::set<std::pair<std::string,CallersData::ExtFctDecl>>::const_iterator rm;
  for(rm=copy_from_me.redeclared_methods->begin(); rm!=copy_from_me.redeclared_methods->end(); ++rm)
    {
      redeclared_methods->insert(*rm);
    }

  for(rm=copy_from_me.redeclarations->begin(); rm!=copy_from_me.redeclarations->end(); ++rm)
    {
      redeclarations->insert(*rm);
    }

  std::set<std::string>::const_iterator cr;
  for(cr=copy_from_me.calls->begin(); cr!=copy_from_me.calls->end(); ++cr)
    {
      calls->insert(*cr);
    }

  for(cr=copy_from_me.called->begin(); cr!=copy_from_me.called->end(); ++cr)
    {
      called->insert(*cr);
    }
}

void CallersData::Record::add_method(std::string method) const
{
  methods->insert(method);
  std::cout << "Add method \"" << method
	    << " to class " << this->name
	    << std::endl;
}

void CallersData::Record::add_member(std::string member, std::string type) const
{
  members->insert(std::make_pair(member, type));
  std::cout << "Add member \"" << member << " with type " << type
	    << " to class " << this->name
	    << std::endl;
}

void CallersData::Record::add_record_call(std::string calleeRecordName) const
{
  calls->insert(calleeRecordName);
  std::cout << "Add call from record " << this->name
            << " to another record \"" << calleeRecordName
	    << std::endl;
  assert(this->name != calleeRecordName);
}

void CallersData::Record::add_record_called(std::string callerRecordName) const
{
  called->insert(callerRecordName);
  std::cout << "Add call to record " << this->name
            << " from record \"" << callerRecordName
	    << std::endl;
  assert(this->name != callerRecordName);
}

void CallersData::Record::add_redeclared_method(std::string baseRecordName, CallersData::ExtFctDecl redecl_method) const
{
  redeclared_methods->insert(std::make_pair(baseRecordName, redecl_method));
  std::cout << "Add virtual method \"" << redecl_method.sign
            << "\" defined in base record \"" << baseRecordName
            << "\" and probably redeclared in class \"" << this->name
	    << "\"" << std::endl;
  // assert(0);
}

void CallersData::Record::add_redeclaration(std::string baseRecordName, CallersData::ExtFctDecl redeclaration) const
{
  redeclarations->insert(std::make_pair(baseRecordName, redeclaration));
  std::cout << "Add virtual child method \"" << redeclaration.sign
            << "\" defined in child record \"" << baseRecordName
            << "\" and probably declared in base class \"" << this->name
	    << "\"" << std::endl;
  // assert(0);
}

std::set<std::pair<std::string, CallersData::ExtFctDecl>>::const_iterator
CallersData::Record::get_redeclared_method(std::string method_sign) const
{
  std::set<std::pair<std::string, CallersData::ExtFctDecl>>::const_iterator redeclared_method;

  // lookup for the virtual base method within the list of probably redeclared methods

  for(redeclared_method=redeclared_methods->begin(); redeclared_method!=redeclared_methods->end(); ++redeclared_method)
    {
      // replace the current record name by its base class in the method_sign
      std::string baseRecordName = redeclared_method->first;
      std::string baseMethodSign1(method_sign);
      std::string baseMethodSign2(method_sign);

      boost::replace_first(baseMethodSign1, this->name + "::", baseRecordName + "::");
      boost::replace_first(baseMethodSign2, "::" + this->name + "::", baseRecordName + "::");

      std::cout << "Lookup for virtual method \"" << baseMethodSign1 << "\" in base record \"" << baseRecordName << "\"" << std::endl;
      std::cout << "Lookup for virtual method \"" << baseMethodSign2 << "\" in base record \"" << baseRecordName << "\"" << std::endl;

      auto redecl_method  = redeclared_method->second;
      std::cout << "- base record: " << baseRecordName << std::endl;
      std::cout << "- probably redeclared method: " << redecl_method.sign << std::endl;

      if((redecl_method.sign == baseMethodSign1) || (redecl_method.sign == baseMethodSign2))
      {
        std::cout << "=> Found base virtual method: " << redecl_method.sign << std::endl;
        return redeclared_method;
      }
    }

  return redeclared_method;
}

std::set<std::pair<std::string, CallersData::ExtFctDecl>>::const_iterator
CallersData::Record::get_redeclaration(std::string method_sign) const
{
  std::set<std::pair<std::string, CallersData::ExtFctDecl>>::const_iterator redeclaration;

  // lookup for the virtual child method within the list of redeclarations

  for(redeclaration=redeclarations->begin(); redeclaration!=redeclarations->end(); ++redeclaration)
    {
      // replace the current record name by its child class in the method_sign
      std::string childRecordName = redeclaration->first;
      std::string childMethodSign(method_sign);
      boost::replace_first(childMethodSign, this->name + "::", childRecordName + "::");

      std::cout << "Lookup for virtual child method \"" << childMethodSign << "\" in child record \"" << childRecordName << "\"" << std::endl;
      // assert(0); // debug

      auto redecl_method  = redeclaration->second;
      std::cout << "- child record: " << childRecordName << std::endl;
      std::cout << "- probably redeclaration: " << redecl_method.sign << std::endl;
      if(redecl_method.sign == childMethodSign)
      {
        std::cout << "=> Found child virtual method: " << redecl_method.sign << std::endl;
        return redeclaration;
      }
    }

  if(redeclarations->begin() == redeclarations->end())
  {
    // WARNING: Uncomment this line for tmp debug purposes only !
    // assert(0);
  }

  // uncomment this line for debug only
  //assert(redeclaration != redeclarations->end());
  return redeclaration;
}

/*
// This code is currently useless because I didn't yet found how to
// check whether a CXXMethodDecl is declared public, private or friend
// For the moment, all methods are considered the same.
void CallersData::Record::add_public_method(std::string method) const
{
  public_methods->insert(method);
  std::cout << "Add public method \"" << method
	    << " in record " << this->name
	    << std::endl;
}

void CallersData::Record::add_private_method(std::string method) const
{
  private_methods->insert(method);
  std::cout << "Add private method \"" << method
	    << " in record " << this->name
	    << std::endl;
}

void CallersData::Record::add_friend_method(std::string method) const
{
  friend_methods->insert(method);
  std::cout << "Add friend method \"" << method
	    << " in record " << this->name
	    << std::endl;
}
*/

void CallersData::Record::add_base_class(CallersData::Inheritance bclass) const
{
  inherits->insert(bclass);
  std::cout << "CallersData::Record::add_base_class: Register base record \"" << bclass.name
	    << "\" defined in file \"" << bclass.file << "\""
	    << " in record " << name
	    << ", nb_inherits=" << inherits->size()
	    << std::endl;
}

void CallersData::Record::add_child_class(CallersData::Inheritance bclass) const
{
  inherited->insert(bclass);
  std::cout << "CallersData::Record::add_child_class: Register child record \"" << bclass.name
	    << "\" defined in file \"" << bclass.file << "\""
	    << " in record " << name
	    << ", nb_inherited=" << inherited->size()
	    << std::endl;
}

void CallersData::Record::print_cout() const
{
  std::ostringstream debut;
  debut << begin;
  std::ostringstream fin;
  fin << end;
  std::cout
    << "{\"name\":\"" << name
    << "\",\"kind\":\"" << ((kind == clang::TTK_Struct) ? "struct"
			    : ((kind == clang::TTK_Class) ? "class"
			       : "anonym"))
    << "\",\"debut\":" << debut.str()
    << ",\"fin\":" << fin.str()
    << ",\"inherits\":[";

  std::set<Inheritance>::const_iterator b, last_bc;
  last_bc = inherits->empty() ? inherits->end() : --inherits->end();
  for(b=inherits->begin(); b!=inherits->end(); ++b)
    {
      if(b != last_bc)
	{
	  b->print_cout();
	  std::cout << ",";
	}
      else
	{
	  b->print_cout();
	}
    }

  std::cout << "]}";
}

void CallersData::Record::output_json_desc(std::ofstream &js) const
{
  std::ostringstream debut;
  debut << begin;
  std::ostringstream fin;
  fin << end;

  js  //<< "{\"eClass\":\"" << CALLERS_TYPE_RECORD << "\", \"fullname\": \"" << name
      << "{\"name\": \"" << name
      << "\",\"kind\":\"" << ((kind == clang::TTK_Struct) ? "struct"
                             : ((kind == clang::TTK_Class) ? "class"
                             : "anonym"))
      << "\",\"debut\":" << debut.str()
      << ",\"fin\":" << fin.str()
      << ",\"inherits\":[";

  std::set<Inheritance>::const_iterator b, last_bc;
  last_bc = inherits->empty() ? inherits->end() : --inherits->end();
  for(b=inherits->begin(); b!=inherits->end(); ++b)
    {
      if(b != last_bc)
	{
	  b->output_json_desc(js);
	  js << ",";
	}
      else
	{
	  b->output_json_desc(js);
	}
    }

  js << "],\"inherited\":[";

  last_bc = inherited->empty() ? inherited->end() : --inherited->end();
  for(b=inherited->begin(); b!=inherited->end(); ++b)
    {
      if(b != last_bc)
	{
	  b->output_json_desc(js);
	  js << ",";
	}
      else
	{
	  b->output_json_desc(js);
	}
    }

  js << "],\"methods\":[";

  std::set<std::string>::const_iterator m, last_m;
  last_m = methods->empty() ? methods->end() : --methods->end();
  for(m=methods->begin(); m!=methods->end(); ++m)
    {
      if(m != last_m)
	{
	  js << "\"" << *m << "\",";
	}
      else
	{
	  js << "\"" << *m << "\"";
	}
    }

  js << "],\"members\":[";

  std::set<std::pair<std::string, std::string>>::const_iterator f, last_f;
  last_f = members->empty() ? members->end() : --members->end();
  for(f=members->begin(); f!=members->end(); ++f)
    {
      if(f != last_f)
	{
	  js << "{\"member\":\"" << f->first << "," << "\",\"kind\":" << f->second << "\"},";
	}
      else
	{
	  js << "{\"member\":\"" << f->first << "," << "\",\"kind\":" << f->second << "\"},";
	}
    }

  js << "],\"calls\":[";

  std::set<std::string>::const_iterator cr, last_cr;
  last_cr = calls->empty() ? calls->end() : --calls->end();
  for(cr=calls->begin(); cr!=calls->end(); ++cr)
    {
      if(cr != last_cr)
	{
	  js << "\"" << *cr << "\",";
	}
      else
	{
	  js << "\"" << *cr << "\"";
	}
    }

  js << "],\"called\":[";

  last_cr = called->empty() ? called->end() : --called->end();
  for(cr=called->begin(); cr!=called->end(); ++cr)
    {
      if(cr != last_cr)
	{
	  js << "\"" << *cr << "\",";
	}
      else
	{
	  js << "\"" << *cr << "\"";
	}
    }

  // js << "],\"redeclared\":[";

  // std::set<std::pair<std::string, ExtFctDecl>>::const_iterator rm, last_rm;
  // last_rm = redeclared_methods->empty() ? redeclared_methods->end() : --redeclared_methods->end();
  // for(rm=redeclared_methods->begin(); rm!=redeclared_methods->end(); ++rm)
  //   {
  //     if(rm != last_rm)
  //       {
  //         js << "{\"bc\":\"" << rm->first << "\",\"rm\":";
  //         rm->second.output_json_desc(js);
  //         js << "},";
  //       }
  //     else
  //       {
  //         js << "{\"bc\":\"" << rm->first << "\",\"rm\":";
  //         rm->second.output_json_desc(js);
  //         js << "}";
  //       }
  //   }

  // js << "],\"redeclarations\":[";

  // std::set<std::pair<std::string, ExtFctDecl>>::const_iterator rm, last_rm;
  // last_rm = redeclarations->empty() ? redeclarations->end() : --redeclarations->end();
  // for(rm=redeclarations->begin(); rm!=redeclarations->end(); ++rm)
  //   {
  //     if(rm != last_rm)
  //       {
  //         js << "{\"bc\":\"" << rm->first << "\",\"rm\":";
  //         rm->second.output_json_desc(js);
  //         js << "},";
  //       }
  //     else
  //       {
  //         js << "{\"bc\":\"" << rm->first << "\",\"rm\":";
  //         rm->second.output_json_desc(js);
  //         js << "}";
  //       }
  //   }

  js << "]}";
}

bool CallersData::operator< (const CallersData::Record& record1, const CallersData::Record& record2)
{
  return record1.name < record2.name;
}

/***************************************** class Thread ****************************************/

/*
void CallersData::Thread::allocate()
{
}
*/

CallersData::Thread::~Thread()
{
}

CallersData::Thread::Thread(std::string inst_name,
                            std::string routine_name,
                            std::string routine_sign,
                            std::string routine_mangled,
                            Virtuality routine_virtuality,
                            std::string routine_file,
                            int routine_line,
                            // Virtuality routine_def_virtuality,
                            // std::string routine_def_file,
                            // int routine_def_line,
                            std::string routine_recordName,
                            std::string routine_recordFilePath,
                            std::string create_location,
                            std::string caller_mangled,
                            std::string caller_sign,
                            Virtuality caller_virtuality,
                            std::string caller_filepath,
                            int caller_filepos,
                            std::string caller_decl_file,
                            int caller_decl_line,
                            std::string caller_recordName,
                            std::string caller_recordFilePath)
  : inst_name(inst_name),
    routine_name(routine_name),
    routine_sign(routine_sign),
    routine_mangled(routine_mangled),
    routine_virtuality(routine_virtuality),
    routine_file(routine_file),
    routine_line(routine_line),
    // routine_def_virtuality(routine_def_virtuality),
    // routine_def_file(routine_def_file),
    // routine_def_line(routine_def_line),
    routine_recordName(routine_recordName),
    routine_recordFilePath(routine_recordFilePath),
    create_location(create_location),
    caller_mangled(caller_mangled),
    caller_sign(caller_sign),
    caller_virtuality(caller_virtuality),
    caller_file(caller_filepath),
    caller_line(caller_filepos),
    caller_decl_file(caller_decl_file),
    caller_decl_line(caller_decl_line),
    caller_recordName(caller_recordName),
    caller_recordFilePath(caller_recordFilePath)
{
  assert(this->inst_name != "unknownThreadInstanceName");
  assert(this->routine_name != "unknownThreadRoutineName");
  assert(this->routine_sign != "unknownThreadRoutineSign");
  assert(this->routine_mangled != "unknownThreadRoutineMangled");
  // assert(this->routine_virtuality != CallersData::VNoVirtual);
  assert(this->routine_file != "unknownThreadRoutineDeclFile");
  assert(this->routine_line != -1);
  // // assert(this->routine_def_virtuality != CallersData::VNoVirtual);
  // assert(this->routine_def_file != "unknownThreadRoutineDefFile");
  // assert(this->routine_def_line != -1);
  assert(this->routine_recordName != CALLERS_DEFAULT_RECORD_NAME);
  assert(this->routine_recordFilePath != CALLERS_DEFAULT_RECORD_PATH);
  assert(this->create_location != "unknownThreadCreateLocation");
  assert(this->caller_sign != "unknownThreadCallerSign");
  assert(this->caller_mangled != "unknownThreadCallerMangled");
  // assert(this->caller_decl_file != CALLERS_NO_FCT_DECL_FILE);
  assert(this->caller_recordName != CALLERS_DEFAULT_RECORD_NAME);
  assert(this->caller_recordFilePath != CALLERS_DEFAULT_RECORD_PATH);

  // allocate();
  std::cout << "Create thread: " << std::endl;
  this->id = this->inst_name + this->routine_mangled;
  this->print_cout();
}

CallersData::Thread::Thread(const CallersData::Thread& copy_from_me)
{
  // allocate();
  std::cout << "Thread copy constructor" << std::endl;
  inst_name = copy_from_me.inst_name;
  id = copy_from_me.id;
  routine_sign = copy_from_me.routine_sign;
  routine_name = copy_from_me.routine_name;
  routine_mangled = copy_from_me.routine_mangled;
  routine_virtuality = copy_from_me.routine_virtuality;
  routine_file = copy_from_me.routine_file;
  routine_line = copy_from_me.routine_line;
  // routine_def_virtuality = copy_from_me.routine_def_virtuality;
  // routine_def_file = copy_from_me.routine_def_file;
  // routine_def_line = copy_from_me.routine_def_line;
  routine_recordName = copy_from_me.routine_recordName;
  routine_recordFilePath = copy_from_me.routine_recordFilePath;
  create_location = copy_from_me.create_location;
  caller_sign = copy_from_me.caller_sign;
  caller_decl_file = copy_from_me.caller_decl_file;
  caller_decl_line = copy_from_me.caller_decl_line;
  caller_mangled = copy_from_me.caller_mangled;
  caller_recordName = copy_from_me.caller_recordName;
  caller_recordFilePath = copy_from_me.caller_recordFilePath;
}

void CallersData::Thread::print_cout() const
{
  std::string decl_virtuality = ((routine_virtuality == CallersData::VNoVirtual) ? "no"
	    	: ((routine_virtuality == CallersData::VVirtualDeclared) ? "declared"
	    	: ((routine_virtuality == CallersData::VVirtualDefined) ? "defined"
		: "pure")));

  std::cout
    << "{\"inst\":\"" << inst_name
    << "\",\"routine_name\":\"" << routine_name
    << "\",\"routine_sign\":\"" << routine_sign
    << "\",\"routine_mangled\":\"" << routine_mangled
    << "\",\"routine_virtuality\":\"" << decl_virtuality
    << "\",\"routine_file\":\"" << routine_file
    << "\",\"routine_line\":" << routine_line
    // << ",\"routine_def_virtuality\":\"" << routine_def_virtuality
    // << "\",\"routine_def_file\":\"" << routine_def_file
    // << "\",\"routine_def_line\":" << routine_def_line
    << ",\"routine_recordName\":\"" << routine_recordName
    << "\",\"routine_recordPath\":\"" << routine_recordFilePath
    << "\",\"caller_sign\":\"" << caller_sign
    << "\",\"caller_mangled\":\"" << caller_mangled
    << "\",\"caller_decl_file\":\"" << caller_decl_file
    << "\",\"caller_recordName\":\"" << caller_recordName
    << "\",\"caller_recordFilePath\":\"" << caller_recordFilePath
    << "\",\"id\":\"" << id
    << "\",\"loc\":\"" << create_location << "\"}";
}

void CallersData::Thread::output_json_desc(std::ofstream &js) const
{
  std::string decl_virtuality = ((routine_virtuality == CallersData::VNoVirtual) ? "no"
	    	: ((routine_virtuality == CallersData::VVirtualDeclared) ? "declared"
	    	: ((routine_virtuality == CallersData::VVirtualDefined) ? "defined"
		: "pure")));

  js  << "{\"inst\": \"" << inst_name
      << "\",\"routine_name\":\"" << routine_name
      << "\",\"routine_sign\":\"" << routine_sign
      << "\",\"routine_mangled\":\"" << routine_mangled
      << "\",\"routine_virtuality\":\"" << decl_virtuality
      << "\",\"routine_file\":\"" << routine_file
      << "\",\"routine_line\":" << routine_line
      // << ",\"routine_def_virtuality\":\"" << routine_def_virtuality
      // << "\",\"routine_def_file\":\"" << routine_def_file
      // << "\",\"routine_def_line\":" << routine_def_line
      << ",\"routine_recordName\":\"" << routine_recordName
      << "\",\"routine_recordPath\":\"" << routine_recordFilePath
      << "\",\"caller_sign\":\"" << caller_sign
      << "\",\"caller_mangled\":\"" << caller_mangled
      << "\",\"caller_decl_file\":\"" << caller_decl_file
      << "\",\"caller_recordName\":\"" << caller_recordName
      << "\",\"caller_recordFilePath\":\"" << caller_recordFilePath
      << "\",\"id\":\"" << id
      << "\",\"loc\":\"" << create_location << "\"}";
}

bool CallersData::operator< (const CallersData::Thread& thread1, const CallersData::Thread& thread2)
{
  return thread1.id < thread2.id;
}

/******************************************* class Fct ******************************************/

CallersData::Fct::Fct(std::string sign) : sign(sign) {}

CallersData::Fct::Fct(MangledName mangled, std::string sign, Virtuality is_virtual)
  : mangled(mangled),
    sign(sign),
    virtuality(is_virtual)
{}

CallersData::Fct::~Fct() {}

CallersData::Fct::Fct(const CallersData::Fct& copy_from_me)
{
  std::cout << "Fct copy constructor" << std::endl;
  mangled = copy_from_me.mangled;
  sign = copy_from_me.sign;
  virtuality = copy_from_me.virtuality;
}

/***************************************** class FctDecl ****************************************/

void CallersData::FctDecl::allocate()
{
  parameters = new std::set<Parameter>;
  threads = new std::set<std::string>;
  redeclared = new std::set<ExtFctDecl>;
  redeclarations = new std::set<ExtFctDecl>;
  definitions = new std::set<std::string>;
  // definitions = new std::set<ExtFctDef>;
  redefinitions = new std::set<ExtFctDef>;
  locallers = new std::set<std::string>;
  extcallers = new std::set<ExtFctDef>;
}

CallersData::FctDecl::~FctDecl()
{
  delete parameters;
  delete threads;
  delete redeclared;
  delete redeclarations;
  delete definitions;
  delete redefinitions;
  delete locallers;
  delete extcallers;
}

CallersData::FctDecl::FctDecl(MangledName mangled, std::string sign, Virtuality is_virtual,
                              std::string filepath, int line, bool is_builtin)
  : Fct(mangled, sign, is_virtual),
    file(filepath),
    line(line),
    is_builtin(is_builtin)
{
  assert(filepath != CALLERS_NO_FILE_PATH);
  assert(filepath != CALLERS_NO_FCT_DECL_FILE);
  allocate();
  if(is_builtin == true)
  {
    std::cout << "Create builtin function declaration: " << std::endl;
    recordName = CALLERS_DEFAULT_RECORD_BUILTIN;
  }
  else
  {
    std::cout << "Create function declaration: " << std::endl;
    recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
  }
  recordFilePath = CALLERS_DEFAULT_NO_RECORD_PATH;
  this->print_cout();
}

CallersData::FctDecl::FctDecl(MangledName mangled, std::string sign, Virtuality is_virtual,
                              std::string filepath, int line, std::string recordName, std::string recordFilePath, bool is_builtin)
  : Fct(mangled, sign, is_virtual),
    file(filepath),
    line(line),
    recordName(recordName),
    recordFilePath(recordFilePath),
    is_builtin(is_builtin)
{
  assert(recordName != CALLERS_DEFAULT_RECORD_NAME);
  assert(recordFilePath != CALLERS_DEFAULT_RECORD_PATH);
  assert(recordFilePath != CALLERS_NO_FILE_PATH);
  allocate();
  if(recordName == CALLERS_DEFAULT_RECORD_BUILTIN)
  {
    is_builtin = true;
  }
  if(is_builtin == true)
  {
    std::cout << "Create builtin function declaration: " << std::endl;
    assert(recordName == CALLERS_DEFAULT_RECORD_BUILTIN);
    assert(recordFilePath == CALLERS_DEFAULT_NO_RECORD_PATH);
  }
  else
  {
    if(recordName == CALLERS_DEFAULT_NO_RECORD_NAME)
    {
      std::cout << "Create function declaration: " << std::endl;
      // assert(recordFilePath == CALLERS_DEFAULT_NO_RECORD_PATH);
    }
    else
    {
      std::cout << "Create \"" << recordName << "\"'s method declaration: " << std::endl;
      std::cout << "Record \"" << recordName << "\" is declared in file \"" << recordFilePath << "\"" << std::endl;
      assert(recordFilePath != CALLERS_DEFAULT_NO_RECORD_PATH);
    }
  }
  this->print_cout();
}

CallersData::FctDecl::FctDecl(std::string sign, std::string filepath)
  : Fct(sign),
    file(filepath)
{
  allocate();
  std::cout << "Partial function's declaration used just to find the complete one: " << std::endl;
}

CallersData::FctDecl::FctDecl(const CallersData::FctDecl& copy_from_me)
  : Fct(copy_from_me.mangled, copy_from_me.sign, copy_from_me.virtuality)
{
  allocate();
  std::cout << "FctDecl copy constructor" << std::endl;
  file = copy_from_me.file;
  line = copy_from_me.line;
  recordName = copy_from_me.recordName;
  recordFilePath = copy_from_me.recordFilePath;
  is_builtin = copy_from_me.is_builtin;

  // copy parameters
  std::set<Parameter>::const_iterator p;
  for(p=copy_from_me.parameters->begin(); p!=copy_from_me.parameters->end(); ++p )
    {
      parameters->insert(*p);
    };

  // copy threads
  std::set<std::string>::const_iterator i;
  for(i=copy_from_me.threads->begin(); i!=copy_from_me.threads->end(); ++i )
    {
      threads->insert(*i);
    };

  // copy local callers
  for(i=copy_from_me.locallers->begin(); i!=copy_from_me.locallers->end(); ++i)
    {
      locallers->insert(*i);
    };

  // copy redeclared methods
  std::set<ExtFctDecl>::const_iterator xdc;
  for(xdc=copy_from_me.redeclared->begin(); xdc!=copy_from_me.redeclared->end(); ++xdc )
    {
      redeclared->insert(*xdc);
    };

  // copy redeclarations
  for(xdc=copy_from_me.redeclarations->begin(); xdc!=copy_from_me.redeclarations->end(); ++xdc )
    {
      redeclarations->insert(*xdc);
    };

  // copy definitions
  std::set<std::string>::const_iterator ld;
  for(ld=copy_from_me.definitions->begin(); ld!=copy_from_me.definitions->end(); ++ld)
    {
      definitions->insert(*ld);
    };

  // copy external callers
  std::set<ExtFctDef>::const_iterator xdf;
  for( xdf=copy_from_me.extcallers->begin(); xdf!=copy_from_me.extcallers->end(); ++xdf )
    {
      extcallers->insert(*xdf);
    };

  // copy redefinitions
  for( xdf=copy_from_me.redefinitions->begin(); xdf!=copy_from_me.redefinitions->end(); ++xdf )
    {
      redefinitions->insert(*xdf);
    }
}

void CallersData::FctDecl::add_parameter(const CallersData::Parameter& parameter) const
{
  // std::cout << "Check whether the parameter \"" << parameter.name << "\" is already present or not..." << std::endl;

  std::set<CallersData::Parameter>::iterator search_result;
  search_result = parameters->find(parameter);
  if(search_result != parameters->end())
    {
      // std::cout << "Already present parameter \"" << parameter.name
      //           << "\" in function \"" << this->sign << "\", so do not add it twice."
      //           << std::endl;
    }
  else
    {
      // std::cout << "Add parameter \"" << parameter.name
      //           << "\" to function \"" << this->sign << "\". "
      //           << std::endl;
      CallersData::Parameter param (parameter);
      parameters->insert(param);
      search_result = parameters->find(parameter);
      assert(search_result != parameters->end());
      //std::cout << "the parameter \"" << parameter.name << "\" is well present now !" << std::endl;
    }
  return;
}

void CallersData::FctDecl::add_thread(std::string thread_id) const
{
  assert(thread_id != CALLERS_DEFAULT_NO_THREAD_ID);
  std::cout << "Add thread id \"" << thread_id << "\" to function declaration \"" << this->sign << "\"" << std::endl;
  this->threads->insert(thread_id);
}

void CallersData::FctDecl::add_local_caller(std::string caller_sign) const
{
  if(this->sign == caller_sign)
  {
    std::cout << "WARNING: Do not add recursive local call from caller function \"" << caller_sign << "\" to itself \" ! " << std::endl;
    return;
  }

  if(recordName == CALLERS_DEFAULT_RECORD_NAME)
  {
    std::cout << "Add local caller \"" << caller_sign << "\" to callee function declaration \"" << this->sign << "\"" << std::endl;
  }
  else
  {
    std::cout << "Add local caller \"" << caller_sign << "\" to callee method declaration \"" << this->sign << "\"" << std::endl;
  }

  locallers->insert(caller_sign);
}

void CallersData::FctDecl::add_redeclared_method(const CallersData::ExtFctDecl& redecl_method) const
{
  std::cout << "Check whether the redeclared method \"" << redecl_method.sign << "\" is already present or not..." << std::endl;

  std::set<CallersData::ExtFctDecl>::iterator search_result;
  search_result = redeclared->find(redecl_method);
  if(search_result != redeclared->end())
    {
      std::cout << "Already present redeclared method \"" << redecl_method.sign
                << "\" of function \"" << sign << "\", so do not add it twice."
                << "The base virtual method is declared in file: " << redecl_method.fctLoc
                << std::endl;
    }
  else
    {
      std::cout << "Add redeclared method \"" << redecl_method.sign
                << "\" to function \"" << this->sign << "\". "
                << "The base virtual method is declared in file: " << redecl_method.fctLoc
                << std::endl;
      CallersData::ExtFctDecl redecl (redecl_method);
      redeclared->insert(redecl);
      search_result = redeclared->find(redecl_method);
      assert(search_result != redeclared->end());
      //std::cout << "the redeclared method \"" << redecl_method.sign << "\" is well present now !" << std::endl;
    }
  return;
}

void CallersData::FctDecl::add_redeclaration(const CallersData::ExtFctDecl& redecl_method) const
{
  std::cout << "Check whether the redeclaration \"" << redecl_method.sign << "\" is already present or not..." << std::endl;

  std::set<CallersData::ExtFctDecl>::iterator search_result;
  search_result = redeclarations->find(redecl_method);
  if(search_result != redeclarations->end())
    {
      std::cout << "Already present redeclaration \"" << redecl_method.sign
                << "\" of function \"" << sign << "\", so do not add it twice."
                << "The base virtual method is declared in file: " << redecl_method.fctLoc
                << std::endl;
    }
  else
    {
      std::cout << "Add redeclaration \"" << redecl_method.sign
                << "\" to function \"" << this->sign << "\". "
                << "The base virtual method is declared in file: " << redecl_method.fctLoc
                << std::endl;
      CallersData::ExtFctDecl redecl (redecl_method);
      redeclarations->insert(redecl);
      search_result = redeclarations->find(redecl_method);
      assert(search_result != redeclarations->end());
      //std::cout << "the redeclaration \"" << redecl_method.sign << "\" is well present now !" << std::endl;
    }
  return;
}

void CallersData::FctDecl::add_definition(std::string fct_sign, std::string def_file_pos) const
{
  std::cout << "Add definition to function declaration \"" << fct_sign << "\"" << std::endl;
  std::cout << "Function definition is located at: " << def_file_pos << std::endl;

  // ExtFctDef definition(def_mangled, def_sign, virtuality, def_file_pos, E_FctDef, record);
  // definitions->insert(definition);
  definitions->insert(def_file_pos);
}

/*
void CallersData::FctDecl::add_definition(MangledName def_mangled, std::string fct_sign, std::string def_sign,
					  Virtuality def_virtuality, std::string def_file_pos, std::string record) const
{
  if(record == CALLERS_DEFAULT_RECORD_NAME)
  {
    std::cout << "Add definition \"" << def_sign << "\" to function declaration \"" << fct_sign << "\"" << std::endl;
    std::cout << "Function definition is located at: " << def_file_pos << std::endl;
  }
  else {
    std::cout << "Add definition \"" << def_sign << "\" to method declaration \"" << fct_sign << "\" in class " << record << std::endl;
    std::cout << "Method definition is located at: " << def_file_pos << std::endl;
  }

  ExtFctDef definition(def_mangled, def_sign, def_file_pos);
  definitions->insert(definition);
}
*/

void CallersData::FctDecl::add_external_caller(MangledName caller_mangled, std::string caller_sign, std::string caller_file_pos) const
{
  std::cout << "Add external caller \"" << caller_sign << "\" to callee function declaration \"" << this->sign << "\"" << std::endl
;
  // {
  //   // BEGIN DEBUG SECTION
  //   std::cout << "CallersData::FctDecl::add_external_caller:DEBUG:BEGIN: before addition" << std::endl;
  //   this->print_cout();
  //   std::cout << "CallersData::FctDecl::add_external_caller:DEBUG:END" << std::endl;
  //   // BEGIN DEBUG SECTION
  // }

  std::cout << "Caller function is located at: " << caller_file_pos << std::endl;
  ExtFctDef extfct(caller_mangled, caller_sign, caller_file_pos);
  extcallers->insert(extfct);

  // {
  //   // BEGIN DEBUG SECTION
  //   std::cout << "CallersData::FctDecl::add_external_caller:DEBUG:BEGIN: after addition" << std::endl;
  //   print_cout();
  //   std::cout << "CallersData::FctDecl::add_external_caller:DEBUG:END" << std::endl;
  //   // BEGIN DEBUG SECTION
  // }

}

void CallersData::FctDecl::add_external_caller(MangledName caller_mangled, std::string caller_sign, std::string caller_file, int caller_line) const
{
  std::cout << "Add external caller \"" << caller_sign << "\" to callee function declaration \"" << this->sign << "\"" << std::endl;
  // {
  //   // BEGIN DEBUG SECTION
  //   std::cout << "CallersData::FctDecl::add_external_caller:DEBUG:BEGIN: before addition" << std::endl;
  //   this->print_cout();
  //   std::cout << "CallersData::FctDecl::add_external_caller:DEBUG:END" << std::endl;
  //   // BEGIN DEBUG SECTION
  // }
  std::cout << "Caller function is located at: " << caller_file << ":" << caller_line << std::endl;
  std::ostringstream pos;
  pos << caller_line;
  std::string caller_file_pos = caller_file + ":" + pos.str();
  ExtFctDef extfct(caller_mangled, caller_sign, caller_file_pos);
  extcallers->insert(extfct);
  // {
  //   // BEGIN DEBUG SECTION
  //   std::cout << "CallersData::FctDecl::add_external_caller:DEBUG:BEGIN: after addition" << std::endl;
  //   print_cout();
  //   std::cout << "CallersData::FctDecl::add_external_caller:DEBUG:END" << std::endl;
  //   // BEGIN DEBUG SECTION
  // }
}

void CallersData::FctDecl::add_redefinition(MangledName redef_mangled, std::string redef_sign, CallersData::Virtuality redef_virtuality,
                                            std::string redef_decl_file, int redef_decl_line, std::string redef_record) const
{
  std::string redef_decl_location = redef_decl_file;
  std::ostringstream out;
  out << redef_decl_line;
  redef_decl_location += ":";
  redef_decl_location += out.str();

  if(recordName == CALLERS_DEFAULT_RECORD_NAME)
  {
    std::cout << "Add redefinition \"" << redef_sign
              << "\" to function \"" << sign << "\". "
              << "Function is redefined in file: " << redef_decl_file
              << " at line: " << redef_decl_line
              << std::endl;
  }
  else {
    std::cout << "Add redefinition \"" << redef_sign
              << "\" to method \"" << sign << "\". "
              << "Method is redefined in file: " << redef_decl_file
              << " at line: " << redef_decl_line
              << std::endl;
  }

  ExtFctDef extfct (redef_mangled, redef_sign, redef_decl_location);
  redefinitions->insert(extfct);
}

namespace CallersData
{
  std::ostream &operator<<(std::ostream &output, const Parameter &param);
}

void CallersData::FctDecl::output_parameters(std::ostream &js) const
{
  if(not parameters->empty())
    {
      js << ", \"params\": [";

      std::set<Parameter>::const_iterator p, last;
      //last = std::prev(parameters.end(); // requires C++ 11
      last = parameters->empty() ? parameters->end() : --parameters->end();
      for( p=parameters->begin(); p!=parameters->end(); ++p )
      {
        if(p != last)
        {
          js << *p << ", ";
        }
        else
        {
          js << *p;
        }
      };
      js << "]";
    }
}

void CallersData::FctDecl::output_threads(std::ostream &js) const
{
  if (not threads->empty())
    {
      js << ", \"threads\": [";
      std::set<std::string>::const_iterator i, last;
      //last = std::prev(threads.end(); // requires C++ 11
      last = --threads->end();
      for( i=threads->begin(); i!=threads->end(); ++i )
	{
	  if(i != last)
	    {
	      js << "\"" << *i << "\", ";
	    }
	  else
	    {
	      js << "\"" << *i << "\"";
	    }
	};
      js << "]";
    }
}

void CallersData::FctDecl::output_local_callers(std::ostream &js) const
{
  if (not locallers->empty())
    {
      js << ", \"locallers\": [";
      std::set<std::string>::const_iterator i, last;
      //last = std::prev(locallers.end(); // requires C++ 11
      last = --locallers->end();
      for( i=locallers->begin(); i!=locallers->end(); ++i )
		{
		  if(i != last)
			{
			  js << "\"" << *i << "\", ";
			}
		  else
			{
			  js << "\"" << *i << "\"";
			}
		};

      js << "]";
    }
}

void CallersData::FctDecl::output_redeclared_methods(std::ostream &js) const
{
  if(not redeclared->empty())
    {
      js << ", \"redeclared\": [";

      std::set<ExtFctDecl>::const_iterator x, extlast;
      //last = std::prev(redeclared.end(); // requires C++ 11
      extlast = redeclared->empty() ? redeclared->end() : --redeclared->end();
      for( x=redeclared->begin(); x!=redeclared->end(); ++x )
		{
		  if(x != extlast)
			{
			  js << *x << ", ";
			}
		  else
			{
			  js << *x;
			}
		};
      js << "]";

    }
}

void CallersData::FctDecl::output_redeclarations(std::ostream &js) const
{
  if(not redeclarations->empty())
    {
      js << ", \"redeclarations\": [";

      std::set<ExtFctDecl>::const_iterator x, extlast;
      //last = std::prev(redeclarations.end(); // requires C++ 11
      extlast = redeclarations->empty() ? redeclarations->end() : --redeclarations->end();
      for( x=redeclarations->begin(); x!=redeclarations->end(); ++x )
		{
		  if(x != extlast)
			{
			  js << *x << ", ";
			}
		  else
			{
			  js << *x;
			}
		};
      js << "]";

    }
}

void CallersData::FctDecl::output_definitions(std::ostream &js) const
{
  if(not definitions->empty())
    {
      js << ", \"definitions\": [";

      //std::set<ExtFctDef>::const_iterator x, extlast;
      std::set<std::string>::const_iterator x, extlast;
      //last = std::prev(definitions.end(); // requires C++ 11
      extlast = definitions->empty() ? definitions->end() : --definitions->end();
      for(x=definitions->begin(); x!=definitions->end(); ++x)
	{
	  if(x != extlast)
	    {
	      js << "\"" << *x << "\",";
	    }
	  else
	    {
	      js << "\"" << *x << "\"";
	    }
	};
      js << "]";
    }
}

void CallersData::FctDecl::output_external_callers(std::ostream &js) const
{
  if( not extcallers->empty())
    {
      js << ", \"extcallers\": [";

      std::set<ExtFctDef>::const_iterator x, extlast;
      //last = std::prev(extcallers.end(); // requires C++ 11
      extlast = extcallers->empty() ? extcallers->end() : --extcallers->end();
      for( x=extcallers->begin(); x!=extcallers->end(); ++x )
	{
	  if(x != extlast)
	    {
	      js << *x << ", ";
	    }
	  else
	    {
	      js << *x;
	    }
	};
      js << "]";
    }
}

void CallersData::FctDecl::output_redefinitions(std::ostream &js) const
{
  if(not redefinitions->empty())
    {
      js << ", \"redefinitions\": [";

      std::set<ExtFctDef>::const_iterator x, extlast;
      //last = std::prev(redefinitions.end(); // requires C++ 11
      extlast = redefinitions->empty() ? redefinitions->end() : --redefinitions->end();
      for( x=redefinitions->begin(); x!=redefinitions->end(); ++x )
	{
	  if(x != extlast)
	    {
	      js << *x << ", ";
	    }
	  else
	    {
	      js << *x;
	    }
	};
      js << "]";

    }
}

void CallersData::FctDecl::output_json_desc(std::ostream &js) const
{
  std::ostringstream out;
  out << line;
  js //<< "{\"eClass\":\"" << CALLERS_TYPE_FCT_DECL << "\", \"sign\": \"" << sign
     << "{\"sign\": \"" << sign
     << "\", \"line\": " << out.str()
     << ", \"virtuality\": \""
     << ((virtuality == VNoVirtual) ? "no"
	 : ((virtuality == VVirtualDeclared) ? "declared"
	    : ((virtuality == VVirtualDefined) ? "defined"
	       : /* virtuality == VVirtualPure */ "pure")))
     << "\", \"mangled\": \"" << mangled << "\"";

  if(recordName != CALLERS_DEFAULT_RECORD_NAME )
  {
    js << ", \"recordName\": \"" << recordName << "\"";
    js << ", \"recordPath\": \"" << recordFilePath << "\"";
    if((recordName != CALLERS_DEFAULT_NO_RECORD_NAME) &&
       (recordName != CALLERS_DEFAULT_RECORD_BUILTIN))
    {
      assert(recordFilePath != CALLERS_DEFAULT_NO_RECORD_PATH);
    }
  }

  this->output_parameters(js);
  this->output_threads(js);
  this->output_redeclared_methods(js);
  this->output_redeclarations(js);
  this->output_definitions(js);
  this->output_redefinitions(js);
  this->output_local_callers(js);
  this->output_external_callers(js);

  js << "}";
}

/* private functions */

void CallersData::FctDecl::print_cout() const
{
  std::ostream *std_out = &std::cout;
  this->output_json_desc(*std_out);
}

/* public functions */

bool CallersData::operator< (const CallersData::FctDecl& fct1, const CallersData::FctDecl& fct2)
{
  return fct1.sign < fct2.sign;
}

/***************************************** class FctDef ****************************************/

void CallersData::FctDef::allocate()
{
  threads = new std::set<std::string>;
  locallees  = new std::set<std::string>;
  extcallees = new std::set<ExtFctDecl>;
}

CallersData::FctDef::~FctDef()
{
  delete threads;
  delete locallees;
  delete extcallees;
}

CallersData::FctDef::FctDef(MangledName mangled,
                            std::string sign,
                            Virtuality is_virtual,
                            std::string def_filepath,
                            int def_line,
                            std::string decl_file,
                            int decl_line,
                            std::string record)
  : Fct(mangled, sign, is_virtual),
    def_file(def_filepath),
    def_line(def_line),
    decl_file(decl_file),
    decl_line(decl_line),
    record(record)
{
  assert(def_filepath != CALLERS_NO_FILE_PATH);
  assert(def_filepath != CALLERS_NO_FCT_DEF_FILE);

  assert(decl_file != CALLERS_NO_FILE_PATH);
  assert(decl_file != CALLERS_NO_FCT_DECL_FILE);

  allocate();

  if(sign.find("::") != std::string::npos)
    assert(record != CALLERS_DEFAULT_RECORD_NAME);

  if(record == CALLERS_DEFAULT_RECORD_NAME)
  {
    std::cout << "Create function definition: " << std::endl;
  }
  else
  {
    std::cout << "Create " << record << "'s method definition: " << std::endl;
  }
  this->print_cout(sign, is_virtual, def_file, def_line, record);
}

CallersData::FctDef::FctDef(const CallersData::FctDef& copy_from_me)
  : Fct(copy_from_me.mangled, copy_from_me.sign, copy_from_me.virtuality)
{
  allocate();
  std::cout << "FctDef copy constructor" << std::endl;
  def_file = copy_from_me.def_file;
  def_line = copy_from_me.def_line;
  decl_file = copy_from_me.decl_file;
  decl_line = copy_from_me.decl_line;
  record = copy_from_me.record;

  // copy threads
  std::set<std::string>::const_iterator i;
  for( i=copy_from_me.threads->begin(); i!=copy_from_me.threads->end(); ++i )
    {
      threads->insert(*i);
    };

  // copy local callees
  for( i=copy_from_me.locallees->begin(); i!=copy_from_me.locallees->end(); ++i )
    {
      locallees->insert(*i);
    };

  // copy external callees
  std::set<ExtFctDecl>::const_iterator x;
  for(x=copy_from_me.extcallees->begin(); x!=copy_from_me.extcallees->end(); ++x )
    {
      extcallees->insert(*x);
    };
}

void CallersData::FctDef::add_thread(std::string thread_id) const
{
  assert(thread_id != CALLERS_DEFAULT_NO_THREAD_ID);
  std::cout << "Add thread id \"" << thread_id << "\" to function definition \"" << this->sign << "\"" << std::endl;
  this->threads->insert(thread_id);
}

void CallersData::FctDef::add_local_callee(std::string callee_sign) const
{
  if(this->sign == callee_sign)
  {
    std::cerr << "WARNING: Do not add recursive local call from callee function \"" << callee_sign << "\" to itself \" ! " << std::endl;
    return;
  }

  if(this->record == CALLERS_DEFAULT_RECORD_NAME)
  {
    std::cout << "Add local callee function \"" << callee_sign << "\" to function \"" << this->sign << "\"" << std::endl;
  }
  else
  {
    std::cout << "Add local callee method \"" << callee_sign << "\" to function \"" << this->sign << "\", record=" << this->record << std::endl;
  }

  locallees->insert(callee_sign);
}

void CallersData::FctDef::add_external_callee(MangledName callee_builtin, std::string callee_sign, std::string callee_decl_file_pos) const
{
  std::cout << "Add external callee \"" << callee_sign
	    << "\" to function \"" << sign << "\". "
	    << "Callee is declared in file: " << callee_decl_file_pos
	    << std::endl;

  ExtFctDecl extfct (callee_builtin, callee_sign, callee_decl_file_pos);
  extcallees->insert(extfct);
}

void CallersData::FctDef::add_external_callee(MangledName callee_builtin, std::string callee_sign, std::string callee_decl_file, int callee_decl_line) const
{
  std::string callee_decl_location = callee_decl_file;
  std::ostringstream out;
  out << callee_decl_line;
  callee_decl_location += ":";
  callee_decl_location += out.str();

  std::cout << "Add external callee \"" << callee_sign
	    << "\" to function \"" << sign << "\". "
	    << "Callee is declared in file: " << callee_decl_file
	    << std::endl;

  ExtFctDecl extfct (callee_builtin, callee_sign, callee_decl_location);
  extcallees->insert(extfct);
}

void CallersData::FctDef::add_builtin_callee(MangledName builtin_mangled, std::string builtin_sign, CallersData::Virtuality builtin_virtuality,
                                             std::string builtin_decl_file, int builtin_decl_line) const
{
  std::string builtin_decl_location = builtin_decl_file;
  std::ostringstream out;
  out << builtin_decl_line;
  builtin_decl_location += ":";
  builtin_decl_location += out.str();

  std::cout << "Add builtin callee \"" << builtin_sign
	    << "\" to function \"" << sign << "\". "
	    << "Builtin callee is declared in file: " << builtin_decl_file
	    << " at line: " << builtin_decl_line
	    << std::endl;

  std::string record(CALLERS_DEFAULT_NO_RECORD_NAME);
  ExtFctDecl extfct (builtin_mangled, builtin_sign, builtin_decl_location);
  extcallees->insert(extfct);
}

void CallersData::FctDef::output_threads(std::ofstream &js) const
{
  if (not threads->empty())
    {
      js << ", \"threads\": [";
      std::set<std::string>::const_iterator i, last;
      //last = std::prev(threads.end(); // requires C++ 11
      last = --threads->end();
      for( i=threads->begin(); i!=threads->end(); ++i )
	{
	  if(i != last)
	    {
	      js << "\"" << *i << "\", ";
	    }
	  else
	    {
	      js << "\"" << *i << "\"";
	    }
	};
      js << "]";
    }
}

void CallersData::FctDef::output_local_callees(std::ofstream &js) const
{
  if (not locallees->empty())
    {
      js << ", \"locallees\": [";
      std::set<std::string>::const_iterator i, last;
      //last = std::prev(locallees.end(); // requires C++ 11
      last = --locallees->end();
      for(i=locallees->begin(); i!=locallees->end(); ++i)
	{
	  if(i != last)
	    {
	      js << "\"" << *i << "\", ";
	    }
	  else
	    {
	      js << "\"" << *i << "\"";
	    }
	};

      js << "]";
    }
}

void CallersData::FctDef::output_external_callees(std::ofstream &js) const
{
  if(not extcallees->empty())
    {
      js << ", \"extcallees\": [";

      std::set<ExtFctDecl>::const_iterator x, extlast;
      //last = std::prev(extcallees.end(); // requires C++ 11
      extlast = extcallees->empty() ? extcallees->end() : --extcallees->end();
      for( x=extcallees->begin(); x!=extcallees->end(); ++x )
	{
	  if(x != extlast)
	    {
	      js << *x << ", ";
	    }
	  else
	    {
	      js << *x;
	    }
	};
      js << "]";

    }
}

void CallersData::FctDef::output_json_desc(std::ofstream &js) const
{
  std::ostringstream out;
  out << def_line;
  std::ostringstream decl_pos;
  decl_pos << decl_line;
  js //<< "{\"eClass\":\"" << CALLERS_TYPE_FCT_DEF << "\", \"sign\": \"" << sign
     << "{\"sign\": \"" << sign
     << "\", \"line\": " << out.str()
     << ", \"virtuality\": \""
     << ((virtuality == VNoVirtual) ? "no"
	 : ((virtuality == VVirtualDeclared) ? "declared"
	    : ((virtuality == VVirtualDefined) ? "defined"
	       : /* virtuality == VVirtualPure */ "pure")))
     << "\", \"mangled\": \"" << mangled << "\"";

  if(decl_file != def_file)
  {
    js << ", \"decl\": \"" << decl_file << ":" << decl_pos.str() << "\"";
  }
  else
  {
    js << ", \"decl\": \"local:" << decl_pos.str() << "\"";
  }

  if(record != CALLERS_DEFAULT_RECORD_NAME )
  {
    js << ", \"record\": \"" << record << "\"";
  }

  this->output_threads(js);
  // this->output_local_callers(js);
  this->output_local_callees(js);
  // this->output_external_callers(js);
  this->output_external_callees(js);

  js << "}";
}

/* private functions */

void CallersData::FctDef::print_cout(std::string sign, Virtuality virtuality, std::string file, int line, std::string record)
{
  std::ostringstream loc;
  loc << line;
  std::ostringstream decl_pos;
  decl_pos << decl_line;

  std::cout << "{\"sign\":\"" << sign
	    << "\",\"line\":" << loc.str()
            << ",\"virtuality\":\"" << ((virtuality == CallersData::VNoVirtual) ? "no"
                                       : ((virtuality == CallersData::VVirtualDeclared) ? "declared"
                                       : ((virtuality == CallersData::VVirtualDefined) ? "defined"
                                       : "pure")))
            << "\",\"file\":\"" << file
            << "\", \"mangled\": \"" << mangled << "\"";

  if(decl_file != def_file)
  {
    std::cout << ", \"decl\": \"" << decl_file << ":" << decl_pos.str() << "\"";
  }
  else
  {
    std::cout << ", \"decl\": \"local:" << decl_pos.str() << "\"";
  }

  if(record != CALLERS_DEFAULT_RECORD_NAME )
  {
    std::cout << ", \"record\": \"" << record << "\"";
  }
  std::cout << "\"}"
	    << std::endl;
}

/* public functions */

bool CallersData::operator< (const CallersData::FctDef& fct1, const CallersData::FctDef& fct2)
{
  return fct1.sign < fct2.sign;
}

/**************************************** class FctCall ***************************************/

CallersData::FctCall::FctCall(FctDef caller, FctDecl callee)
  : caller(caller),
    callee(callee)
{
  id = caller.sign + " -> " + callee.sign;
  {
    // for debug only
    // std::ostringstream def_line, decl_line;
    // def_line << caller_line;
    // decl_line << callee_decl_line;
    std::cout << "CallersData::FctCall::FctCall::DEBUG_2: create function call: " << caller.sign << " -> " << callee.sign << std::endl
              << " from caller def file: " << caller.def_file << ":" << caller.def_line << std::endl
              << " to callee decl file: " << callee.file << ":" << callee.line << std::endl;
  }
}

bool CallersData::operator< (const CallersData::FctCall& fc1, const CallersData::FctCall& fc2)
{
  return fc1.id < fc2.id;
}

/**************************************** class ExtFct ***************************************/

CallersData::ExtFct::ExtFct(MangledName mangled, std::string sign, std::string fctLoc)
  : mangled(mangled),
    sign(sign),
    fctLoc(fctLoc)
{
  // std::cout << "Create external function: " << std::endl;
  // if(mangled == "_Z17EVP_DecryptUpdate")
  // {
  //   std::cout << "TO_DEBUG..." << std::endl;
  // }
}

CallersData::ExtFct::ExtFct(const CallersData::ExtFct& copy_from_me)
{
  mangled = copy_from_me.mangled;
  sign = copy_from_me.sign;
  fctLoc = copy_from_me.fctLoc;
  std::cout << "Copy external function: " << std::endl;
  // print_cout(mangled, sign, fct);
}

// bool CallersData::operator< (const CallersData::ExtFct& fct1, const CallersData::ExtFct& fct2)
// {
//   return fct1.sign < fct2.sign;
// }

namespace CallersData {

  bool operator< (const ExtFct& fct1, const ExtFct& fct2)
  {
    return fct1.sign < fct2.sign;
  }
}

/**************************************** class ExtFctDecl ***************************************/

void CallersData::ExtFctDecl::print_cout(MangledName mangled, std::string sign, std::string fctLoc)
{
  std::cout << "{\"sign\":\"" << sign;
  std::cout << "\",\"decl\":\"" << fctLoc;
  std::cout << "\",\"mangled\":\"" << mangled << "\"}" << std::endl;
}

void CallersData::ExtFctDecl::output_json_desc(std::ostream &js) const
{
  js << "{\"sign\": \"" << sign
     << "\", \"mangled\": \"" << mangled
     << "\", \"fctLoc\": \"" << fctLoc << "\"}";
}

std::ostream &CallersData::operator<<(std::ostream &output, const ExtFctDecl &fct)
{
  output << "{\"sign\":\"" << fct.sign;
  output << "\",\"decl\":\"" << fct.fctLoc;
  output << "\",\"mangled\":\"" << fct.mangled << "\"}";
  return output;
}

/**************************************** class ExtFctDef ***************************************/

void CallersData::ExtFctDef::print_cout(MangledName mangled, std::string sign, std::string fctLoc)
{
  std::cout << "{\"sign\":\"" << sign;
  std::cout << "\",\"def\":\"" << fctLoc;
  std::cout << "\",\"mangled\":\"" << mangled << "\"}" << std::endl;
}

std::ostream &CallersData::operator<<(std::ostream &output, const ExtFctDef &fct)
{
  output << "{\"sign\":\"" << fct.sign;
  output << "\",\"def\":\"" << fct.fctLoc;
  output << "\",\"mangled\":\"" << fct.mangled << "\"}";
  return output;
}

/******************************************* class Data ******************************************/

CallersData::Data::Data() {}

CallersData::Data::~Data() {}

CallersData::Data::Data(const CallersData::Data& copy_from_me)
{
  // std::cout << "Data copy constructor" << std::endl;
}

/******************************************* class NamedData ******************************************/

CallersData::NamedData::NamedData(std::string name) : name(name) {}

CallersData::NamedData::~NamedData() {}

CallersData::NamedData::NamedData(const CallersData::NamedData& copy_from_me) : Data()
{
  // std::cout << "NamedData copy constructor" << std::endl;
  name = copy_from_me.name;
}

/******************************************* class Parameter ******************************************/

CallersData::Parameter::Parameter(std::string name, std::string type) : NamedData(name), type(type) {}

CallersData::Parameter::~Parameter() {}

CallersData::Parameter::Parameter(const CallersData::Parameter& copy_from_me)
: NamedData(copy_from_me.name)
{
  // std::cout << "Parameter copy constructor" << std::endl;
  type = copy_from_me.type;
}

namespace CallersData {

  bool operator< (const Parameter& param1, const Parameter& param2)
  {
    return param1.name < param2.name;
  }

  std::ostream &operator<<(std::ostream &output, const Parameter &param)
  {
    output << "{\"name\":\"" << param.name;
    output << "\",\"kind\":\"" << param.type << "\"}";
    return output;
  }
}
