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
  std::cout << "decode_function_location: fct_file=" << fct_file << std::endl;
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
    std::cout << "decode_function_location: fct_line=" << line << std::endl;
  }
  return true;
}

/***************************************** class JsonFileWriter ****************************************/

CallersData::JsonFileWriter::JsonFileWriter(std::string jsonFileName)
  : fileName(jsonFileName), out()
{
  std::cout << "Try to open file \"" << jsonFileName << "\" in write mode..." << std::endl;

  // Check whether the input path does well begins with the expected rootdir path
  assert_rootdir_prefix(jsonFileName);

  bool opened = false;
  do
    {
      out.open(jsonFileName.c_str());
      if(out.fail())
	{
	  std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
	  std::cerr << "WARNING: Failed to open file \"" << fileName << "\" in write mode." << std::endl;
	  std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
	  exit(-1);
	  //sleep(1);
	}
      else
	{
	  std::cout << "JSON output file \"" << fileName << "\" is now opened in write mode." << std::endl;
	  opened = true;
	}
    }
  while( opened == false );
}

CallersData::JsonFileWriter::~JsonFileWriter()
{
  std::cout << "Close file \"" << fileName << "\"." << std::endl;
  out.close();
  if(out.fail())
    {
      std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
      std::cerr << "Failed to close file \"" << fileName << "\"." << std::endl;
      std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
      exit(2);
    }
  else
    {
      std::cout << "JSON output file \"" << fileName << "\" is now closed." << std::endl;
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
    jsonfilename(path + "/" + dir + "/" + dir + ".dir.callers.gen.json")
{
  assert_rootdir_prefix(path);
  //assert_homedir_prefix(path);
}

CallersData::Dir::~Dir()
{

}

std::string CallersData::Dir::fullPath()
{
  std::string fullPath = path + "/" + dir;
  return fullPath;
}

void CallersData::Dir::add_file(std::string file)
{
  std::cout << "Register file name \"" << file << "\" in directory \"" << this->fullPath() << "\"" << std::endl;
  filenames.push_back(file);
}

void CallersData::Dir::add_file(File file)
{
  std::cout << "Register file path \"" << file.fullPath() << "\"" << std::endl;
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
      //std::cout << "The file \"" << filepath << "\" is not yet opened." << std::endl;
      std::cout << "Tries to open and parse the file \"" << filepath << "\"..." << std::endl;
      CallersData::File file(filename, dirpath);
      file.parse_json_file(this);
      this->add_file(file);
      search_result = files.find(searched_file);
      if(search_result != files.end())
	{
	  std::cout << "The file \"" << filepath << "\" is well opened now !" << std::endl;
	}
    }
  return search_result;
}

void CallersData::Dir::output_json_files()
{
  std::set<CallersData::File>::const_iterator f;

  for(f=files.begin(); f!=files.end(); ++f)
    {
      std::cout << "Edit file \"" << f->fullPath() << "\"..." << std::endl;
      f->output_json_desc();
    }
}

void CallersData::Dir::output_json_dir()
{
  CallersData::JsonFileWriter js(this->jsonfilename);
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

CallersData::File::File(std::string file, std::string path)
  : file(file),
    path(path),
    fullpath(CALLERS_ROOTDIR_PREFIX + path),
    jsonfilename(CALLERS_ROOTDIR_PREFIX + path + "/" + file + ".file.callers.gen.json")
{
  namespaces = new std::set<CallersData::Namespace>;
  declared = new std::set<CallersData::FctDecl>;
  defined = new std::set<CallersData::FctDef>;
  records = new std::set<CallersData::Record>;
  threads = new std::set<CallersData::Thread>;
  calls = new std::set<CallersData::FctCall>;

  kind = this->getKind();
  filepath = this->fullPath();

  // Check whether the related callers'analysis path does already exists or not in the filesystem
  if(!(boost::filesystem::exists(fullpath)))
  {
    std::cout << "Creating tmp directory: " << fullpath << std::endl;
    boost::filesystem::create_directories(fullpath);
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
  file = copy_from_me.file;
  kind = copy_from_me.kind;
  path = copy_from_me.path;
  filepath = copy_from_me.filepath;
  fullpath = copy_from_me.fullpath;
  jsonfilename = copy_from_me.jsonfilename;

  namespaces = new std::set<CallersData::Namespace>;
  declared = new std::set<CallersData::FctDecl>;
  defined = new std::set<CallersData::FctDef>;
  records = new std::set<CallersData::Record>;
  threads = new std::set<CallersData::Thread>;
  calls = new std::set<CallersData::FctCall>;

  std::cout << "File Copy Constructor of file \"" << file
	    << "\" defining " << copy_from_me.records->size()
	    << " classes, " << copy_from_me.defined->size() << " functions"
	    << " and " << copy_from_me.calls->size() << " function calls"
	    << std::endl;

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
  //std::string jsonfilename = this->js.fileName;
  std::string jsonfilename = this->fullpath + "/" + file + ".file.callers.gen.json";
  std::cout << "Parsing json file \"" << jsonfilename << "\"..." << std::endl;
  assert_rootdir_prefix(jsonfilename);
  FILE* pFile = fopen(jsonfilename.c_str(), "rb");
  // Always check to see if file opening succeeded
  if (pFile == NULL)
    {
      //std::cout << "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww" << std::endl;
      std::cout << "WARNING: do not parse json file \"" << jsonfilename << "\" which doesn't exists yet !" << std::endl;
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
                  std::string record = CALLERS_DEFAULT_NO_RECORD_NAME;
                  if(symb.HasMember("record"))
                  {
                    const rapidjson::Value& rec = symb["record"];
                    record = rec.GetString();
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
                  // std::ostringstream spos;
                  // spos << pos;
                  // std::string location(filepath + ":" + spos.str());
                  // symbol_location.insert(SymbLoc::value_type(symbol, location));
                  CallersData::FctDecl fctDecl(mangled, symbol, virtuality, this->file, pos, record);

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
                          std::cout << "Parsed function def pos: \"" << def_pos << std::endl;
                          /* std::string fct_def_pos = id_filter_prefix("local:", def_pos);
                          if(fct_def_pos != def_pos)
                          {
                            fct_def_pos = dirpath + filename + fct_def_pos;
                          }
                          std::cout << "Completed function def pos: \"" << fct_def_pos << std::endl; */
                          fctDecl.add_definition(symbol, /*fct_*/def_pos);
                        }
                    }
                  }

                  this->add_declared_function(&fctDecl, filepath, files);
                  std::cout << "Parsed declared function s[" << s << "]:\"" << symbol << "\"" << std::endl;
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

                  this->add_defined_function(mangled, symbol, virtuality, this->file, def_pos, filepath, decl_file, decl_line, record, files);
                  std::cout << "Parsed symbol s[" << s << "]:\"" << symbol << "\"" << std::endl;
                }
            }
          }
	}
      else
	{
	  std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
	  std::cerr << "ERROR: Empty or Malformed file \"" << jsonfilename << "\"\n" << std::endl;
	  std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
	  exit(3);
	}
    }
}

/* WARNING: some file may not have any extension while being a valid file.
   This is the case for the header file /usr/include/c++/4.8/exception
 */
std::string CallersData::File::getKind() const
{
  assert(this->file != CALLERS_NO_FILE_PATH);
  std::string ext = boost::filesystem::extension(this->file);
  std::string kind = "none";
  boost::regex headers(".h|.hpp");
  boost::regex sources(".c|.cpp|.tcc");
  //boost::cmatch m;
  if(boost::regex_match(ext,/*m,*/ headers))
  {
      kind = "inc";
  }
  else if(boost::regex_match(ext,/*m,*/ sources))
  {
      kind = "src";
  }

  if(ext == "")
  {
    std::cout << "CallersData.cpp:WARNING: file without any extension \"" << this->file << "\". We suppose here this is a valid header file !\n" << std::endl;
    kind = "inc";
  }

  if(kind == "none")
  {
    std::cout << "CallersData.cpp:WARNING: Unsupported file extension \"" << ext << "\"\n" << std::endl;
    assert(kind != "none");
  }
  return kind;
}

std::string CallersData::File::fullPath() const
{
  std::string fullPath = path + "/" + file;
  return fullPath;
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
      std::cout << "The namespace \"" << qualifiers << "\" is already present." << std::endl;
    }
  else
    {
      CallersData::Namespace c_namespace(qualifiers, *nspc);
      this->add_namespace(c_namespace);
      search_result = namespaces->find(searched_nspc);
      if(search_result != namespaces->end())
        {
          std::cout << "The namespace \"" << qualifiers << "\" is well present now !" << std::endl;
        }
    }
  return search_result;
}

void CallersData::File::add_namespace(CallersData::Namespace nspc) const
{
  std::cout << "Register namespace \"" << nspc.name
	    << "\" defined in file \"" << this->fullPath() << std::endl;
  namespaces->insert(nspc);
}

void CallersData::File::add_record(CallersData::Record *rec) const
{
  std::string kind = ((rec->kind == clang::TTK_Struct) ? "struct"
		      : ((rec->kind == clang::TTK_Class) ? "class"
			 : "anonym"));
  int nb_base_classes = rec->inherits->size();
  std::cout << "Register " << kind << " record \"" << rec->name
	    << "\" with " << nb_base_classes << " base classes, "
	    << "defined in file \"" << this->fullPath() << ":"
	    << rec->begin << ":" << rec->end << "\"" << std::endl;
  records->insert(*rec);
}

void CallersData::File::add_record(std::string name, clang::TagTypeKind kind, int begin, int end) const
{
  //Record *rec = new Record(name, kind, deb, fin); // fuite mémoire sur la pile si pas désalloué !
  Record rec(name, kind, this->fullPath(), begin, end);
  records->insert(rec);
  int nb_base_classes = rec.inherits->size();
  std::cout << "Create " << kind << " record \"" << name
	    << "\" with " << nb_base_classes << " base classes, "
	    << "located in file \"" << this->fullPath()
	    << ":" << begin << ":" << end << "\"" << std::endl;
}

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
                                     thr->caller_file, thr->caller_line, thr->caller_decl_file, thr->caller_decl_line, thr->caller_record);
  auto caller_def = defined->find(thr_caller_def);
  // ensure caller def has really been found
  assert(caller_def != defined->end());
  caller_def->add_thread(thr->id);

  // add the thread to the routine decl
  assert(thr->routine_file == this->filepath);
  // get a reference to the thread routine declaration (if well present as expected)
  CallersData::FctDecl thr_routine_decl(thr->routine_mangled, thr->routine_sign, thr->routine_virtuality,
                                        thr->routine_file, thr->routine_line, thr->routine_record);
  routine_decl = declared->find(thr_routine_decl);
  // ensure routine decl has really been found
  assert(routine_decl != declared->end());
  routine_decl->add_thread(thr->id);

  // // add the thread to the routine definition
  // // get a reference to the thread routine definition (if well present as expected)
  // CallersData::FctDef thr_routine_def(thr->routine_mangled, thr->routine_sign, thr->routine_def_virtuality,
  //                                      thr->routine_def_file, thr->routine_def_line, thr->routine_decl_pos, thr->routine_record);
  // routine_def = defined->find(thr_routine_def);
  // // ensure routine decl has really been found
  // assert(routine_def != defined->end());
  // routine_def->add_thread(thr->id);
}

void CallersData::File::add_declared_function(CallersData::FctDecl* fct, std::string filepath, CallersData::Dir *files) const
{
  std::cout << "Register function \"" << fct->sign
	    << "\" declared in file \"" << fct->file << ":"
	    << fct->line << "\"" << std::endl;

  // Check whether the declared function belongs to the current file.
  if( filepath == this->fullPath() )
    // the declared function belongs to the current file
    {
      std::cout << "The declared function belongs to the current file, so we add it directly\n" << std::endl;
      declared->insert(*fct);
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
      fct_decl_file->add_declared_function(fct, filepath, files);
    }
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
  if( decl_filepath == this->fullPath() )
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

void CallersData::File::add_defined_function(CallersData::FctDef* fct, std::string filepath, CallersData::Dir *otherFiles) const
{
  std::cout << "Register function \"" << fct->sign
	    << "\" defined in file \"" << fct->def_file << ":"
	    << fct->def_line << "\"" << std::endl;

  // Check whether the defined function belongs to the current file.
  if(filepath == this->fullPath())
    // the defined function belongs to the current file
    {
      std::cout << "The defined function belongs to the current file, so we add it directly\n" << std::endl;
      defined->insert(*fct);
      if(fct->decl_file != CALLERS_NO_FCT_DECL_FILE)
      {
        std::ostringstream sdef_pos;
        sdef_pos << fct->def_line;
        if(fct->decl_file == CALLERS_LOCAL_FCT_DECL)
        {
          std::string def_pos = std::string(CALLERS_LOCAL_FCT_DECL) + ":" + sdef_pos.str();
          this->add_definition_to_declaration(def_pos, fct->sign, filepath, otherFiles);
        }
        else
        {
          std::string def_pos = filepath + ":" + sdef_pos.str();
          this->add_definition_to_declaration(def_pos, fct->sign, fct->decl_file, otherFiles);
        }
      }
    }
  else
    {
      std::cerr << "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n" << std::endl;
      std::cerr << "CallersData::File::add_defined_function:ERROR: unsupported case: " << std::endl;
      std::cerr << "   The defined function \"" << fct->sign << "\" doesn't belong to the current file: " << this->fullPath() << std::endl;
      std::cerr << "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n" << std::endl;
      exit(CALLERS_ERROR_UNSUPPORTED_CASE);
    }
}

void CallersData::File::add_defined_function(MangledName fct_mangled, std::string fct_sign, Virtuality fct_virtuality,
					     std::string fct_def_file, int fct_def_line, std::string fct_def_filepath,
                                             std::string fct_decl_file, int fct_decl_line, std::string record, CallersData::Dir *otherFiles) const
{
  // Check whether the defined function has well to be added the current file.
  if( fct_def_filepath == this->fullPath() )
    // the defined function belongs to the current file
    {
      if(record == CALLERS_DEFAULT_RECORD_NAME) {
        std::cout << "CallersData::File::add_defined_function: Create function definition \"" << fct_sign
                  << "\" located in file \"" << fct_def_file << ":" << fct_def_line << "\"" << std::endl;
      }
      else {
        std::cout << "CallersData::File::add_defined_function: Create " << record << " method definition \"" << fct_sign
                  << "\" located in file \"" << fct_def_file << ":" << fct_def_line << "\"" << std::endl;
      }
      FctDef fct(fct_mangled, fct_sign, fct_virtuality, fct_def_file, fct_def_line, fct_decl_file, fct_decl_line, record);
      this->add_defined_function(&fct, fct_def_filepath, otherFiles);
      // if(fct_decl_file != CALLERS_NO_FCT_DECL_FILE)
      // {
      //   std::ostringstream sdef_pos;
      //   sdef_pos << fct_def_line;
      //   std::string def_pos = fct_def_file + ":" + sdef_pos.str();
      //   this->add_definition_to_declaration(def_pos, fct_sign, fct_decl_file, otherFiles);
      // }
    }
  else
    {
      std::cerr << "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n" << std::endl;
      std::cerr << "CallersData::File::add_defined_function:ERROR: unsupported case: " << std::endl;
      std::cerr << "   The defined function \"" << fct_sign << "\" doesn't belong to the current file: " << this->fullPath() << std::endl;
      std::cerr << "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n" << std::endl;
      exit(CALLERS_ERROR_UNSUPPORTED_CASE);
    }
}

void
CallersData::File::add_function_call(CallersData::FctCall* fc, CallersData::Dir *files) const
{
  // std::cerr << "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii" << std::endl;
  std::cout << "Register function call from caller \"" << fc->caller_file << ":" << fc->caller_sign
            << "\" to callee \"" << fc->callee_decl_file << ":" << fc->callee_sign
	    << "\" in file \"" << this->fullPath() << "\"" << std::endl;
  std::cout << "caller sign: " << fc->caller_sign << std::endl;
  std::cout << "caller virtual: "
	    << ((fc->caller_virtuality == CallersData::VNoVirtual) ? "no"
	    	: ((fc->caller_virtuality == CallersData::VVirtualDeclared) ? "declared"
	    	: ((fc->caller_virtuality == CallersData::VVirtualDefined) ? "defined"
		: "pure")))
            << std::endl;
  std::cout << "callee sign: " << fc->callee_sign << std::endl;
  std::cout << "callee virtual: "
	    << ((fc->callee_virtuality == CallersData::VNoVirtual) ? "no"
	    	: ((fc->callee_virtuality == CallersData::VVirtualDeclared) ? "declared"
	    	: ((fc->callee_virtuality == CallersData::VVirtualDefined) ? "defined"
		: "pure")))
            << std::endl;
  std::cout << "current file: " << this->fullPath() << std::endl;
  std::cout << "caller def pos: " << fc->caller_file << ":" << fc->caller_line << std::endl;
  std::cout << "callee decl pos: " << fc->callee_decl_file << ":" << fc->callee_decl_line << std::endl;

  calls->insert(*fc);

  std::set<CallersData::FctDef>::const_iterator caller;
  std::set<CallersData::FctDecl>::const_iterator callee;

  // Check whether the caller function belongs to the current file.
  if( fc->caller_file == this->fullPath() )

    // the caller function belongs to the current file
    {
      std::cout << "The caller function belongs to the current file" << std::endl;

      // adds the caller function to the defined functions of the current file
      add_defined_function(fc->caller_mangled, fc->caller_sign, fc->caller_virtuality, fc->caller_file, fc->caller_line, fc->caller_file, fc->caller_decl_file, fc->caller_decl_line, fc->caller_record, files);
      // get a reference to the related defined function
      CallersData::FctDef caller_fct(fc->caller_mangled, fc->caller_sign, fc->caller_virtuality, fc->caller_file, fc->caller_line, fc->caller_decl_file, fc->caller_decl_line, fc->caller_record);
      caller = defined->find(caller_fct);
      // ensure caller has really been found
      assert(caller != defined->end());

      // Check whether the callee function belongs to the current file.
      if( fc->callee_decl_file == this->fullPath() )

	// the callee function belongs to the current file
	{
	  std::cout << "The callee function belongs to the current file" << std::endl;

	  // adds the callee function to the declared functions of the current file
          CallersData::FctDecl fctDecl(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line, fc->callee_record);
          this->add_declared_function(&fctDecl, fc->callee_decl_file, files);

	  // get a reference to the related declared function
	  CallersData::FctDecl callee_fct(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line, fc->callee_record);
	  callee = declared->find(callee_fct);
	  // ensure callee has really been found
	  assert(callee != declared->end());

	  // add local caller to local callee
	  callee->add_local_caller(fc->caller_mangled, fc->caller_sign, fc->caller_record);

	  // add local callee to local caller
	  caller->add_local_callee(fc->callee_mangled, fc->callee_sign, fc->callee_record);
	}
      else
	// the callee function is defined externally
	{
	  std::cout << "The callee function is defined externally" << std::endl;

	  // check first whether a json file is already present for the callee function
	  // if true, parse it and add the defined function only when necessary
	  // if false, create the callee json file and add the defined function
	  boost::filesystem::path p(fc->callee_decl_file);
	  std::string callee_basename = p.filename().string();
	  std::string callee_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
	  std::set<CallersData::File>::iterator callee_file = files->create_or_get_file(callee_basename, callee_dirpath);
	  //CallersData::File callee_file(callee_basename, callee_dirpath);
	  //callee_file.parse_json_file();
	  CallersData::FctDecl callee_decl(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line, fc->callee_record);
	  callee_file->add_declared_function(&callee_decl, fc->callee_decl_file, files);

	  // get a reference to the related defined function
	  callee = callee_file->declared->find(callee_decl);
	  // ensure callee has really been found
	  assert(callee != callee_file->declared->end());

	  if(fc->is_builtin == true)
	    {
              // add the builtin callee to the local caller
	      caller->add_builtin_callee(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line);

	      // add the local caller to the builtin callee
	      callee->add_external_caller(fc->caller_mangled, fc->caller_sign, fc->callee_virtuality, fc->caller_file, fc->caller_line, fc->caller_record);
	    }
	  else
            {
              // add the external callee to the local caller
              caller->add_external_callee(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line, fc->callee_record);

	      // add the local caller to the external callee
	      callee->add_external_caller(fc->caller_mangled, fc->caller_sign, fc->callee_virtuality, fc->caller_file, fc->caller_line, fc->caller_record);
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
      boost::filesystem::path p(fc->caller_file);
      std::string caller_basename = p.filename().string();
      std::string caller_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
      std::set<CallersData::File>::iterator caller_file = files->create_or_get_file(caller_basename, caller_dirpath);
      // CallersData::File caller_file(caller_basename, caller_dirpath);
      // caller_file.parse_json_file();
      CallersData::FctDef caller_def(fc->caller_mangled, fc->caller_sign, fc->caller_virtuality,
                                     fc->caller_file, fc->caller_line, fc->caller_decl_file, fc->caller_decl_line, fc->caller_record);
      caller_file->add_defined_function(&caller_def, fc->caller_file, files);
      // get a reference to the related defined function
      caller = caller_file->defined->find(caller_def);
      // ensure caller has really been found
      assert(caller != caller_file->defined->end());

      // check whether the callee function belongs to the current file or not
      if( fc->callee_decl_file == this->fullPath() )

	// the callee function belongs to the current file
	{
	  std::cout << "The callee function belongs to the current file" << std::endl;

	  // adds the callee function to the declared functions of the current file
          CallersData::FctDecl fctDecl(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line, fc->callee_record);
	  add_declared_function(&fctDecl, fc->callee_decl_file, files);

	  // get a reference to the related defined function
	  CallersData::FctDecl callee_fct(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line, fc->callee_record);
	  callee = declared->find(callee_fct);
	  // ensure callee has really been found
	  assert(callee != declared->end());

	  // add the external caller to the local callee
	  callee->add_external_caller(fc->caller_mangled, fc->caller_sign, fc->callee_virtuality, fc->caller_file, fc->caller_line, fc->caller_record);

	  // add the local callee to the external caller
	  caller->add_external_callee(fc->caller_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line, fc->callee_record);
	}
      else
	// the callee function is defined externally as the caller !!!
	{
	  std::cout << "The callee function is defined externally as the caller !!!" << std::endl;

	  // check first whether a json file is already present for the callee function
	  // if true, parse it and add the defined function only when necessary
	  // if false, create the callee json file and add the defined function
	  boost::filesystem::path p(fc->callee_decl_file);
	  std::string callee_basename = p.filename().string();
	  std::string callee_dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
	  std::set<CallersData::File>::iterator callee_file = files->create_or_get_file(callee_basename, callee_dirpath);
	  //CallersData::File callee_file(callee_basename, callee_dirpath);
	  //callee_file.parse_json_file();
	  CallersData::FctDecl callee_decl(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line, fc->callee_record);
	  callee_file->add_declared_function(&callee_decl, fc->callee_decl_file, files);

	  // get a reference to the related defined function
	  callee = callee_file->declared->find(callee_decl);
	  // ensure callee has really been found
	  assert(callee != callee_file->declared->end());

	  // check whether the caller and callee functions are collocated or not
	  if( fc->caller_file == fc->callee_decl_file )
	    {
	      std::cout << "The caller and callee functions are collocated in the same file" << std::endl;

	      // add the local caller to the local callee
	      callee->add_local_caller(fc->caller_mangled, fc->caller_sign, fc->caller_record);

	      // add the local callee to the local caller
	      caller->add_local_callee(fc->callee_mangled, fc->callee_sign, fc->callee_record);
	    }
	  else
	    {
	      std::cout << "The caller and callee functions are located in different files" << std::endl;

	      // add the external caller to the external callee
	      callee->add_external_caller(fc->caller_mangled, fc->caller_sign, fc->callee_virtuality, fc->caller_file, fc->caller_line, fc->caller_record);

	      // add the external callee to the external caller
	      caller->add_external_callee(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line, fc->callee_record);
	    }

	  //callee_file->output_json_desc();
	}

      //caller_file->output_json_desc();
    }
  // std::cout << "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii" << std::endl;
}

void CallersData::File::output_json_desc() const
{
  CallersData::JsonFileWriter js(this->jsonfilename);

  js.out
    //<< "{\"eClass\":\"" << CALLERS_TYPE_FILE << "\",\"file\":\"" << file
    << "{\"file\":\"" << file
    << "\",\"kind\":\"" << kind
    << "\",\"path\":\"" << path
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
  return file1.fullPath() < file2.fullPath();
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
}

CallersData::Record::~Record()
{
  delete inherits;
  delete inherited;
  delete methods;
}

CallersData::Record::Record(const char* name, clang::TagTypeKind kind, const std::string file, const int begin, const int end)
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
}

void CallersData::Record::add_method(std::string method) const
{
  methods->insert(method);
  std::cout << "Add method \"" << method
	    << " to class " << this->name
	    << std::endl;
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
  std::cout << "Register inheritance \"" << bclass.name
	    << "\" defined in file \"" << bclass.file << "\""
	    << " in record " << name
	    << ", nb_inherits=" << inherits->size()
	    << std::endl;
}

void CallersData::Record::add_base_class(std::string name, std::string file,
                                         int begin, int end) const
{
  Inheritance bclass(name, file, begin, end);
  inherits->insert(bclass);
  std::cout << "Create inheritance \"" << name
	    << "\" located in file \"" << file << "\""
	    << " in record " << this->name
	    << ", nb_inherits=" << inherits->size()
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

/*
// output_base_classes
// output_child_classes

void CallersData::Record::output_json_desc(std::ofstream &js) const
{
  std::ostringstream debut;
  debut << begin;
  std::ostringstream fin;
  fin << end;
  js << "{\"name\":\"" << name
     << "\",\"kind\":\"" << RecordKind[kind]
     << "\",\"debut\":" << begin.str()
     << ",\"fin\":" << end.str();

  this->output_base_classes(js);

  this->output_child_classes(js);

  js << "}";
}
*/

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
                            std::string routine_record,
                            std::string create_location,
                            std::string caller_mangled,
                            std::string caller_sign,
                            Virtuality caller_virtuality,
                            std::string caller_filepath,
                            int caller_filepos,
                            std::string caller_decl_file,
                            int caller_decl_line,
                            std::string caller_record)
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
    routine_record(routine_record),
    create_location(create_location),
    caller_mangled(caller_mangled),
    caller_sign(caller_sign),
    caller_virtuality(caller_virtuality),
    caller_file(caller_filepath),
    caller_line(caller_filepos),
    caller_decl_file(caller_decl_file),
    caller_decl_line(caller_decl_line),
    caller_record(caller_record)
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
  assert(this->routine_record != CALLERS_DEFAULT_RECORD_NAME);
  assert(this->create_location != "unknownThreadCreateLocation");
  assert(this->caller_sign != "unknownThreadCallerSign");
  assert(this->caller_mangled != "unknownThreadCallerMangled");
  // assert(this->caller_decl_file != CALLERS_NO_FCT_DECL_FILE);

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
  routine_record = copy_from_me.routine_record;
  create_location = copy_from_me.create_location;
  caller_sign = copy_from_me.caller_sign;
  caller_decl_file = copy_from_me.caller_decl_file;
  caller_decl_line = copy_from_me.caller_decl_line;
  caller_mangled = copy_from_me.caller_mangled;
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
    << ",\"routine_record\":\"" << routine_record
    << "\",\"caller_sign\":\"" << caller_sign
    << "\",\"caller_mangled\":\"" << caller_mangled
    << "\",\"caller_decl_file\":\"" << caller_decl_file
    << "\",\"caller_record\":\"" << caller_record
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
      << ",\"routine_record\":\"" << routine_record
      << "\",\"caller_sign\":\"" << caller_sign
      << "\",\"caller_mangled\":\"" << caller_mangled
      << "\",\"caller_decl_file\":\"" << caller_decl_file
      << "\",\"caller_record\":\"" << caller_record
      << "\",\"id\":\"" << id
      << "\",\"loc\":\"" << create_location << "\"}";
}

bool CallersData::operator< (const CallersData::Thread& thread1, const CallersData::Thread& thread2)
{
  return thread1.id < thread2.id;
}

/***************************************** class FctDecl ****************************************/

void CallersData::FctDecl::allocate()
{
  threads = new std::set<std::string>;
  redeclarations = new std::set<ExtFct>;
  definitions = new std::set<std::string>;
  // definitions = new std::set<ExtFct>;
  redefinitions = new std::set<ExtFct>;
  locallers = new std::set<std::string>;
  extcallers = new std::set<ExtFct>;
}

CallersData::FctDecl::~FctDecl()
{
  delete threads;
  delete redeclarations;
  delete definitions;
  delete redefinitions;
  delete locallers;
  delete extcallers;
}

CallersData::FctDecl::FctDecl(const char* mangled, const char* sign, Virtuality is_virtual,
                              const char* filepath, const int line, const char* record)
  : mangled(mangled),
    sign(sign),
    file(filepath),
    virtuality(is_virtual),
    line(line),
    record(record)
{
  allocate();
  std::string sgn(sign);
  std::string rec(record);
  if(sgn.find("::") != std::string::npos)
    assert(rec != CALLERS_DEFAULT_RECORD_NAME);
  if(rec == CALLERS_DEFAULT_RECORD_NAME)
  {
    std::cout << "Create function declaration: " << std::endl;
  }
  else
  {
    std::cout << "Create " << rec << "'s method declaration: " << std::endl;
  }
  this->print_cout(sign, is_virtual, file, line, rec);
}

CallersData::FctDecl::FctDecl(MangledName mangled, std::string sign, Virtuality is_virtual,
                              std::string filepath, int line, std::string record)
  : mangled(mangled),
    sign(sign),
    file(filepath),
    virtuality(is_virtual),
    line(line),
    record(record)
{
  allocate();
  if(sign.find("::") != std::string::npos)
    assert(record != CALLERS_DEFAULT_RECORD_NAME);
  if(record == CALLERS_DEFAULT_RECORD_NAME)
  {
    std::cout << "Create function declaration: " << std::endl;
  }
  else
  {
    std::cout << "Create " << record << "'s method declaration: " << std::endl;
  }
  this->print_cout(sign, is_virtual, file, line, record);
}

CallersData::FctDecl::FctDecl(std::string sign, std::string filepath)
  : sign(sign),
    file(filepath)
{
  allocate();
  std::cout << "Partial function's declaration used just to find the complete one: " << std::endl;
}

CallersData::FctDecl::FctDecl(const CallersData::FctDecl& copy_from_me)
{
  allocate();
  std::cout << "FctDecl copy constructor" << std::endl;
  mangled = copy_from_me.mangled;
  sign = copy_from_me.sign;
  virtuality = copy_from_me.virtuality;
  line = copy_from_me.line;
  record = copy_from_me.record;

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

  // copy redeclarations
  std::set<ExtFct>::const_iterator x;
  for(x=copy_from_me.redeclarations->begin(); x!=copy_from_me.redeclarations->end(); ++x )
    {
      redeclarations->insert(*x);
    };

  // copy definitions
  std::set<std::string>::const_iterator d;
  for(d=copy_from_me.definitions->begin(); d!=copy_from_me.definitions->end(); ++d)
    {
      definitions->insert(*d);
    };

  // copy external callers
  for( x=copy_from_me.extcallers->begin(); x!=copy_from_me.extcallers->end(); ++x )
    {
      extcallers->insert(*x);
    };

  // copy redefinitions
  for( x=copy_from_me.redefinitions->begin(); x!=copy_from_me.redefinitions->end(); ++x )
    {
      redefinitions->insert(*x);
    }
}

void CallersData::FctDecl::add_thread(std::string thread_id) const
{
  assert(thread_id != CALLERS_DEFAULT_NO_THREAD_ID);
  std::cout << "Add thread id \"" << thread_id << "\" to function declaration \"" << this->sign << "\"" << std::endl;
  this->threads->insert(thread_id);
}

void CallersData::FctDecl::add_local_caller(MangledName mangled, std::string sign, std::string record) const
{
  if(record == CALLERS_DEFAULT_RECORD_NAME) {
    std::cout << "Add local caller \"" << sign << "\" to callee function declaration \"" << sign << "\"" << std::endl;
  }
  else {
    std::cout << "Add local caller \"" << sign << "\" to callee method declaration \"" << sign << "\"" << std::endl;
  }
  locallers->insert(sign);
}

// TODO: filter cases where the redeclaration is already present in the fct_decl
void CallersData::FctDecl::add_redeclaration(MangledName redecl_mangled, std::string redecl_sign, CallersData::Virtuality redecl_virtuality,
                                             std::string redecl_decl_file, int redecl_decl_line, std::string redecl_record) const
{
  std::string redecl_decl_location = redecl_decl_file;
  std::ostringstream out;
  out << redecl_decl_line;
  redecl_decl_location += ":";
  redecl_decl_location += out.str();
  // std::cout << "Check whether the redeclaration \"" << redecl_sign << "\" is already present or not..." << std::endl;
  std::set<CallersData::ExtFct>::iterator search_result;
  CallersData::ExtFct searched_redecl(redecl_mangled, redecl_sign, redecl_virtuality, redecl_decl_location, E_FctDecl, redecl_record);
  search_result = redeclarations->find(searched_redecl);
  if(search_result != redeclarations->end())
    {
      std::cout << "Already present redeclaration \"" << redecl_sign
                << "\" of function \"" << sign << "\", so do not add it twice. "
                << "Function is redefined in file: " << redecl_decl_file
                << " at line: " << redecl_decl_line
                << std::endl;
    }
  else
    {
      std::cout << "Add redeclaration \"" << redecl_sign
                << "\" to function \"" << sign << "\". "
                << "Function is redefined in file: " << redecl_decl_file
                << " at line: " << redecl_decl_line
                << std::endl;
      CallersData::ExtFct redecl (redecl_mangled, redecl_sign, redecl_virtuality, redecl_decl_location, E_FctDecl, redecl_record);
      redeclarations->insert(redecl);
      search_result = redeclarations->find(searched_redecl);
      if(search_result != redeclarations->end())
        {
          std::cout << "the redeclaration \"" << redecl_sign << "\" is well present now !" << std::endl;
        }
    }
  return;
}

void CallersData::FctDecl::add_definition(std::string fct_sign, std::string def_file_pos) const
{
  std::cout << "Add definition to function declaration \"" << fct_sign << "\"" << std::endl;
  std::cout << "Function definition is located at: " << def_file_pos << std::endl;

  // ExtFct definition(def_mangled, def_sign, virtuality, def_file_pos, E_FctDef, record);
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

  ExtFct definition(def_mangled, def_sign, virtuality, def_file_pos, E_FctDef, record);
  definitions->insert(definition);
}
*/

void CallersData::FctDecl::add_external_caller(MangledName caller_mangled, std::string caller_sign, Virtuality virtuality,
					       std::string file, int line, std::string record) const
{
  std::cout << "Add external caller \"" << caller_sign << "\" to callee function declaration \"" << sign << "\"" << std::endl;
  std::cout << "Caller function is located at: " << file << ":" << line << std::endl;
  std::ostringstream pos;
  pos << line;
  std::string caller_file = file + ":" + pos.str();
  ExtFct extfct(caller_mangled, caller_sign, virtuality, caller_file, E_FctDef, record);
  extcallers->insert(extfct);
}

void CallersData::FctDecl::add_redefinition(MangledName redef_mangled, std::string redef_sign, CallersData::Virtuality redef_virtuality,
                                            std::string redef_decl_file, int redef_decl_line, std::string redef_record) const
{
  std::string redef_decl_location = redef_decl_file;
  std::ostringstream out;
  out << redef_decl_line;
  redef_decl_location += ":";
  redef_decl_location += out.str();

  if(record == CALLERS_DEFAULT_RECORD_NAME)
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

  ExtFct extfct (redef_mangled, redef_sign, redef_virtuality, redef_decl_location, E_FctDef, redef_record);
  redefinitions->insert(extfct);
}

void CallersData::FctDecl::output_threads(std::ofstream &js) const
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

void CallersData::FctDecl::output_local_callers(std::ofstream &js) const
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

void CallersData::FctDecl::output_redeclarations(std::ofstream &js) const
{
  if(not redeclarations->empty())
    {
      js << ", \"redeclarations\": [";

      std::set<ExtFct>::const_iterator x, extlast;
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

void CallersData::FctDecl::output_definitions(std::ofstream &js) const
{
  if(not definitions->empty())
    {
      js << ", \"definitions\": [";

      //std::set<ExtFct>::const_iterator x, extlast;
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

void CallersData::FctDecl::output_external_callers(std::ofstream &js) const
{
  if( not extcallers->empty())
    {
      js << ", \"extcallers\": [";

      std::set<ExtFct>::const_iterator x, extlast;
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

void CallersData::FctDecl::output_redefinitions(std::ofstream &js) const
{
  if(not redefinitions->empty())
    {
      js << ", \"redefinitions\": [";

      std::set<ExtFct>::const_iterator x, extlast;
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

void CallersData::FctDecl::output_json_desc(std::ofstream &js) const
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

  if(record != CALLERS_DEFAULT_RECORD_NAME )
  {
    js << ", \"record\": \"" << record << "\"";
  }

  this->output_threads(js);
  this->output_redeclarations(js);
  this->output_definitions(js);
  this->output_redefinitions(js);
  this->output_local_callers(js);
  this->output_external_callers(js);

  js << "}";
}

/* private functions */

void CallersData::FctDecl::print_cout(std::string sign, Virtuality virtuality, std::string file, int line, std::string record)
{
  std::ostringstream loc;
  loc << line;

  std::cout << "{\"sign\":\"" << sign
	    << "\",\"line\":\"" << loc.str()
	    << "\",\"virtuality\":\""
	    << ((virtuality == CallersData::VNoVirtual) ? "no"
		: ((virtuality == CallersData::VVirtualDeclared) ? "declared"
		   : ((virtuality == CallersData::VVirtualDefined) ? "defined"
		      : "pure")))
	    << "\",\"file\":\"" << file
            << "\", \"mangled\": \"" << mangled << "\"";

  if(record != CALLERS_DEFAULT_RECORD_NAME )
  {
    std::cout << ", \"record\": \"" << record << "\"";
  }
  std::cout << "\"}"
	    << std::endl;
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
  // locallers  = new std::set<std::string>;
  locallees  = new std::set<std::string>;
  // extcallers = new std::set<ExtFct>;
  extcallees = new std::set<ExtFct>;
}

CallersData::FctDef::~FctDef()
{
  delete threads;
  // delete locallers;
  delete locallees;
  // delete extcallers;
  delete extcallees;
}

CallersData::FctDef::FctDef(const char* mangled,
                            const char* sign,
                            Virtuality is_virtual,
                            const char* def_filepath,
                            const int def_line,
                            const char* decl_filepath,
                            const int decl_line,
                            const char* record)
  : mangled(mangled),
    sign(sign),
    virtuality(is_virtual),
    def_file(def_filepath),
    def_line(def_line),
    decl_file(decl_filepath),
    decl_line(decl_line),
    record(record)
{
  assert(def_file != CALLERS_NO_FCT_DEF_FILE);
  allocate();
  std::string sgn(sign);
  std::string rec(record);
  if(sgn.find("::") != std::string::npos)
    assert(rec != CALLERS_DEFAULT_RECORD_NAME);
  if(rec == CALLERS_DEFAULT_RECORD_NAME)
  {
    std::cout << "Create function definition: " << std::endl;
  }
  else
  {
    std::cout << "Create " << rec << "'s method definition: " << std::endl;
  }
  this->print_cout(sign, is_virtual, def_file, def_line, rec);
}

CallersData::FctDef::FctDef(MangledName mangled,
                            std::string sign,
                            Virtuality is_virtual,
                            std::string def_filepath,
                            int def_line,
                            std::string decl_file,
                            int decl_line,
                            std::string record)
  : mangled(mangled),
    sign(sign),
    virtuality(is_virtual),
    def_file(def_filepath),
    def_line(def_line),
    decl_file(decl_file),
    decl_line(decl_line),
    record(record)
{
  assert(def_filepath != CALLERS_NO_FCT_DEF_FILE);
  // assert(decl_file != CALLERS_NO_FCT_DECL_FILE);
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
{
  allocate();
  std::cout << "FctDef copy constructor" << std::endl;
  mangled = copy_from_me.mangled;
  sign = copy_from_me.sign;
  virtuality = copy_from_me.virtuality;
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

  // // copy local callers
  // for( i=copy_from_me.locallers->begin(); i!=copy_from_me.locallers->end(); ++i )
  //   {
  //     locallers->insert(*i);
  //   };

  // copy local callees
  for( i=copy_from_me.locallees->begin(); i!=copy_from_me.locallees->end(); ++i )
    {
      locallees->insert(*i);
    };

  // // copy external callers
  // std::set<ExtFct>::const_iterator x;
  // for( x=copy_from_me.extcallers->begin(); x!=copy_from_me.extcallers->end(); ++x )
  //   {
  //     extcallers->insert(*x);
  //   };

  // copy external callees
  std::set<ExtFct>::const_iterator x;
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

// void CallersData::FctDef::add_local_caller(MangledName mangled, std::string sign) const
// {
//   std::cout << "Add local caller \"" << sign << "\" to function \"" << sign << "\"" << std::endl;
//   locallers->insert(sign);
// }

void CallersData::FctDef::add_local_callee(MangledName mangled, std::string sign, std::string record) const
{
  if(record == CALLERS_DEFAULT_RECORD_NAME)
  {
    std::cout << "Add local callee function \"" << sign << "\" to function \"" << sign << "\"" << std::endl;
  }
  else
  {
    std::cout << "Add local callee method \"" << sign << "\" to function \"" << sign << "\", record=" << record << std::endl;
  }
  locallees->insert(sign);
}

// void CallersData::FctDef::add_external_caller(MangledName mangled, std::string sign, Virtuality virtuality,
// 					      std::string file, int line) const
// {
//   std::cout << "Add external caller \"" << sign << "\" to callee function definition \"" << sign << "\"" << std::endl;
//   std::cout << "Caller function is located at: " << file << ":" << line << std::endl;
//   std::string caller_decl = file + ":" + line;
//   ExtFct extfct(sign, virtuality, caller_file);
//   extcallers->insert(extfct);
// }

void CallersData::FctDef::add_external_callee(MangledName callee_builtin, std::string callee_sign, CallersData::Virtuality callee_virtuality, std::string callee_decl_file, int callee_decl_line, std::string record) const
{
  std::string callee_decl_location = callee_decl_file;
  std::ostringstream out;
  out << callee_decl_line;
  callee_decl_location += ":";
  callee_decl_location += out.str();

  std::cout << "Add external callee \"" << callee_sign
	    << "\" to function \"" << sign << "\". "
	    << "Callee is declared in file: " << callee_decl_file
	    << " at line: " << callee_decl_line
	    << std::endl;

  ExtFct extfct (callee_builtin, callee_sign, callee_virtuality, callee_decl_location, E_FctDecl, record);
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
  ExtFct extfct (builtin_mangled, builtin_sign, builtin_virtuality, builtin_decl_location, E_FctDecl, record);
  extcallees->insert(extfct);
}

// void CallersData::FctDef::output_local_callers(std::ofstream &js) const
// {
//   if (not locallers->empty())
//     {
//       js << ", \"locallers\": [";
//       std::set<std::string>::const_iterator i, last;
//       //last = std::prev(locallers.end(); // requires C++ 11
//       last = --locallers->end();
//       for( i=locallers->begin(); i!=locallers->end(); ++i)
//       {
//         if(i != last)
//         {
//           js << "\"" << *i << "\", ";
//         }
//         else
//         {
//          js << "\"" << *i << "\"";
//         }
//       };
//       js << "]";
//     }
// }

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

// void CallersData::FctDef::output_external_callers(std::ofstream &js) const
// {
//   if( not extcallers->empty())
//     {
//       js << ", \"extcallers\": [";
//       std::set<ExtFct>::const_iterator x, extlast;
//       //last = std::prev(extcallers.end(); // requires C++ 11
//       extlast = extcallers->empty() ? extcallers->end() : --extcallers->end();
//       for( x=extcallers->begin(); x!=extcallers->end(); ++x )
// 	{
// 	  if(x != extlast)
// 	    {
// 	      js << *x << ", ";
// 	    }
// 	  else
// 	    {
// 	      js << *x;
// 	    }
// 	};
//       js << "]";
//     }
// }

void CallersData::FctDef::output_external_callees(std::ofstream &js) const
{
  if(not extcallees->empty())
    {
      js << ", \"extcallees\": [";

      std::set<ExtFct>::const_iterator x, extlast;
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

CallersData::FctCall::FctCall(MangledName caller_mangled, std::string caller_sign, Virtuality is_caller_virtual,
                              std::string caller_def_file, int caller_def_line, std::string caller_decl_file, int caller_decl_line,
			      MangledName callee_mangled, std::string callee_sign, Virtuality is_callee_virtual,
                              std::string callee_decl_file, int callee_decl_line,
                              std::string caller_record, std::string callee_record)
  : caller_mangled(caller_mangled),
    caller_sign(caller_sign),
    caller_virtuality(is_caller_virtual),
    caller_file(caller_def_file),
    caller_line(caller_def_line),
    caller_decl_file(caller_decl_file),
    caller_decl_line(caller_decl_line),
    callee_mangled(callee_mangled),
    callee_sign(callee_sign),
    callee_virtuality(is_callee_virtual),
    callee_decl_file(callee_decl_file),
    callee_decl_line(callee_decl_line),
    caller_record(caller_record),
    callee_record(callee_record),
    id(caller_sign + " -> " + callee_sign)
{
  assert(caller_def_file != CALLERS_NO_FILE_PATH);
  assert(caller_def_file != CALLERS_NO_FCT_DEF_FILE);

  assert(caller_decl_file != CALLERS_NO_FILE_PATH);
  assert(caller_decl_file != CALLERS_NO_FCT_DECL_FILE);

  assert(callee_decl_file != CALLERS_NO_FILE_PATH);
  assert(callee_decl_file != CALLERS_NO_FCT_DECL_FILE);

  {
    // for debug only
    // std::ostringstream def_line, decl_line;
    // def_line << caller_line;
    // decl_line << callee_decl_line;
    std::cout << "CallersData::FctCall::FctCall::DEBUG: create function call: " << caller_sign << " -> " << callee_sign << std::endl
              << " from caller def file: " << caller_file << ":" << caller_line << std::endl
              << " to callee decl file: " << callee_decl_file << ":" << callee_decl_line << std::endl;
  }
}

bool CallersData::operator< (const CallersData::FctCall& fc1, const CallersData::FctCall& fc2)
{
  return fc1.id < fc2.id;
}

/**************************************** class ExtFct ***************************************/

void CallersData::ExtFct::print_cout(MangledName mangled, std::string sign, Virtuality virtuality, std::string fct, FctKind kind)
{
  std::cout << "{\"sign\":\"" << sign
            << "\",\"virtuality\":\"" << ((virtuality == CallersData::VNoVirtual) ? "no"
                                          : ((virtuality == CallersData::VVirtualDeclared) ? "declared"
                                          : ((virtuality == CallersData::VVirtualDefined) ? "defined"
                                          : "pure")));
  if( kind == E_FctDecl ) {
    std::cout << "\",\"decl\":\"" << fct;
  } else {
    assert(kind == E_FctDef);
    std::cout << "\",\"def\":\"" << fct;
  }
  std::cout << "\",\"mangled\":\"" << mangled << "\"}" << std::endl;
}

CallersData::ExtFct::ExtFct(MangledName mangled, std::string sign, Virtuality is_virtual, std::string fct, FctKind kind, std::string record)
  : mangled(mangled),
    sign(sign),
    virtuality(is_virtual),
    fct(fct),
    kind(kind),
    record(record)
{
  // std::cout << "Create external function: " << std::endl;
  // if(mangled == "_Z17EVP_DecryptUpdate")
  // {
  //   std::cout << "TO_BE_DEBUUGED..." << std::endl;
  // }
  this->print_cout(mangled, sign, is_virtual, fct, kind);
}

CallersData::ExtFct::ExtFct(const CallersData::ExtFct& copy_from_me)
{
  mangled = copy_from_me.mangled;
  sign = copy_from_me.sign;
  virtuality = copy_from_me.virtuality;
  fct = copy_from_me.fct;
  kind = copy_from_me.kind;
  record = copy_from_me.record;
  std::cout << "Copy external function: " << std::endl;
  this->print_cout(mangled, sign, virtuality, fct, kind);
}

std::ostream &CallersData::operator<<(std::ostream &output, const ExtFct &fct)
{
  output << "{\"sign\":\"" << fct.sign;

  if( fct.kind == E_FctDecl ) {
    output << "\",\"decl\":\"" << fct.fct;
  } else {
    assert(fct.kind == E_FctDef);
    output << "\",\"def\":\"" << fct.fct;
  }
  output << "\",\"mangled\":\"" << fct.mangled << "\"}";
  return output;
}

bool CallersData::operator< (const CallersData::ExtFct& fct1, const CallersData::ExtFct& fct2)
{
  return fct1.sign < fct2.sign;
}
