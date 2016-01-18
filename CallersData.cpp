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
#include "utilities.hpp"

extern std::string getCanonicalAbsolutePath(const std::string& path);

/***************************************** shared utilities ****************************************/

const std::string root_prefix(CALLERS_ROOTDIR_PREFIX);

// return true if the root dir prefix is well present and false otherwise
bool has_rootdir_prefix(std::string path)
{
  // Check whether the path does well begins with root dir prefix
  bool has_prefix = boost::algorithm::contains(path, root_prefix);
  return has_prefix;
}

// stops the execution if the input path does not begins with the expected root dir prefix
void assert_rootdir_prefix(std::string path)
{
  // Check whether the path does well begins with root dir prefix
  bool has_prefix = has_rootdir_prefix(path);
  assert(has_prefix == true);
  return;
}

void assert_homedir_prefix(std::string path)
{
  // Check whether the path does not begins with home dir prefix
  std::string home_prefix ("/tmp/callers/home");
  bool has_prefix = boost::algorithm::contains(path, home_prefix);
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
  bool has_prefix = has_rootdir_prefix(path);
  if( has_prefix )
    {
      // the post-condition is ok, so nothing to do
      return path;
    }
  else
    // add the root dir prefix to the input path
    {
      std::cout << "check_root_dir: adds the rootdir prefix \"" << root_prefix << "\" to the path \"" << path << "\"" << std::endl;
      path = root_prefix + path;
      return path;
    }
}

// remove the root dir prefix when present in the input path
// similar to the ocaml shared function filter_root_dir in file common.ml of the callgraph plugin
std::string filter_rootdir_prefix(std::string path)
{
  // Check whether the path dir does well begins with root dir prefix
  bool has_prefix = has_rootdir_prefix(path);
  if( has_prefix )
    // remove the rootdir prefix from the input path
    {
      std::vector<std::string> parts;
      boost::algorithm::split_regex(parts, path, boost::regex(root_prefix));
      std::vector<std::string>::iterator part = parts.end();
      std::string filtered_path = *part;
      std::cout << "check_root_dir: removes the rootdir prefix \"" << root_prefix << "\" from the path \"" << path << "\": " << filtered_path << std::endl;
      return filtered_path;
    }
  else
    {
      // the post-condition is ok, so nothing to do
      return path;
    }
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

  // the dir id results from the base64 compression of the dir path
  id = encode64(path);
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
         << "\",\"id\":\"" << id
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
  calls = new std::set<CallersData::FctCall>;

  filepath = this->fullPath();
  // the file id results from the base64 encoding of the file path
  id = encode64(filepath);

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
  delete calls;
}

CallersData::File::File(const CallersData::File& copy_from_me)
{
  file = copy_from_me.file;
  path = copy_from_me.path;
  id = copy_from_me.id;
  filepath = copy_from_me.filepath;
  fullpath = copy_from_me.fullpath;
  jsonfilename = copy_from_me.jsonfilename;

  namespaces = new std::set<CallersData::Namespace>;
  declared = new std::set<CallersData::FctDecl>;
  defined = new std::set<CallersData::FctDef>;
  records = new std::set<CallersData::Record>;
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
                  std::ostringstream spos;
                  spos << pos;
                  std::string location(filepath + ":" + spos.str());
                  //symbol_location.insert(SymbLoc::value_type(symbol, location));
                  CallersData::FctDecl fctDecl(mangled, symbol, virtuality, this->file, pos);
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
                  const rapidjson::Value& line = symb["line"];
                  const rapidjson::Value& mangledName = symb["mangled"];

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

                  int pos = line.GetInt();
                  std::ostringstream spos;
                  spos << pos;
                  std::string location(filepath + ":" + spos.str());
                  //symbol_location.insert(SymbLoc::value_type(symbol, location));
                  this->add_defined_function(mangled, symbol, virtuality, this->file, pos, filepath);
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

void CallersData::File::add_record(CallersData::Record rec) const
{
  std::string kind = ((rec.kind == clang::TTK_Struct) ? "struct"
		      : ((rec.kind == clang::TTK_Class) ? "class"
			 : "anonym"));
  int nb_base_classes = rec.inherits->size();
  std::cout << "Register " << kind << " record \"" << rec.name
	    << "\" with " << nb_base_classes << " base classes, "
	    << "defined in file \"" << this->fullPath() << ":"
	    << rec.begin << ":" << rec.end << "\"" << std::endl;
  records->insert(rec);
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

/*
void CallersData::File::add_declared_function(std::string fct_sign, Virtuality fct_virtuality, std::string fct_file, int fct_line, CallersData::Dir *files) const
{
  FctDecl fct(fct_sign, fct_virtuality, fct_file, fct_line);
  this->add_declared_function(&fct, files);
}
*/

void CallersData::File::add_defined_function(CallersData::FctDef* fct, std::string filepath) const
{
  std::cout << "Register function \"" << fct->sign
	    << "\" defined in file \"" << fct->file << ":"
	    << fct->line << "\"" << std::endl;

  // Check whether the defined function belongs to the current file.
  if( filepath == this->fullPath() )
    // the defined function belongs to the current file
    {
      std::cout << "The defined function belongs to the current file, so we add it directly\n" << std::endl;
      defined->insert(*fct);
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
					     std::string fct_file, int fct_line, std::string fct_filepath) const
{
  std::cout << "Create function definition \"" << fct_sign
	    << "\" located in file \"" << fct_file << ":" << fct_line << "\"" << std::endl;
  FctDef fct(fct_mangled, fct_sign, fct_virtuality, fct_file, fct_line);
  this->add_defined_function(&fct, fct_filepath);
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
      add_defined_function(fc->caller_mangled, fc->caller_sign, fc->caller_virtuality, fc->caller_file, fc->caller_line, fc->caller_file);
      // get a reference to the related defined function
      CallersData::FctDef caller_fct(fc->caller_mangled, fc->caller_sign, fc->caller_virtuality, fc->caller_file, fc->caller_line);
      caller = defined->find(caller_fct);
      // ensure caller has really been found
      assert(caller != defined->end());

      // Check whether the callee function belongs to the current file.
      if( fc->callee_decl_file == this->fullPath() )

	// the callee function belongs to the current file
	{
	  std::cout << "The callee function belongs to the current file" << std::endl;

	  // adds the callee function to the declared functions of the current file
          CallersData::FctDecl fctDecl(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line);
          this->add_declared_function(&fctDecl, fc->callee_decl_file, files);

	  // get a reference to the related declared function
	  CallersData::FctDecl callee_fct(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line);
	  callee = declared->find(callee_fct);
	  // ensure callee has really been found
	  assert(callee != declared->end());

	  // add local caller to local callee
	  callee->add_local_caller(fc->caller_mangled, fc->caller_sign);

	  // add local callee to local caller
	  caller->add_local_callee(fc->callee_mangled, fc->callee_sign);
	}
      else
	// the callee function is defined externally
	{
	  std::cout << "The callee function is defined externally" << std::endl;

	  // add the external callee to the local caller
	  if(fc->is_builtin == true)
	    {
	      caller->add_builtin_callee(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line);
	    }
	  else
	    {
	      caller->add_external_callee(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line);
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
      CallersData::FctDef caller_def(fc->caller_mangled, fc->caller_sign, fc->caller_virtuality, fc->caller_file, fc->caller_line);
      caller_file->add_defined_function(&caller_def, fc->caller_file);
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
          CallersData::FctDecl fctDecl(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line);
	  add_declared_function(&fctDecl, fc->callee_decl_file, files);

	  // get a reference to the related defined function
	  CallersData::FctDecl callee_fct(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line);
	  callee = declared->find(callee_fct);
	  // ensure callee has really been found
	  assert(callee != declared->end());

	  // add the external caller to the local callee
	  callee->add_external_caller(fc->caller_mangled, fc->caller_sign, fc->callee_virtuality, fc->caller_file, fc->caller_line);

	  // add the local callee to the external caller
	  caller->add_external_callee(fc->caller_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line);
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
	  CallersData::FctDecl callee_decl(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line);
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
	      callee->add_local_caller(fc->caller_mangled, fc->caller_sign);

	      // add the local callee to the local caller
	      caller->add_local_callee(fc->callee_mangled, fc->callee_sign);
	    }
	  else
	    {
	      std::cout << "The caller and callee functions are located in different files" << std::endl;

	      // add the external caller to the external callee
	      callee->add_external_caller(fc->caller_mangled, fc->caller_sign, fc->callee_virtuality, fc->caller_file, fc->caller_line);

	      // add the external callee to the external caller
	      caller->add_external_callee(fc->callee_mangled, fc->callee_sign, fc->callee_virtuality, fc->callee_decl_file, fc->callee_decl_line);
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
    << "\",\"path\":\"" << path
    << "\",\"id\":\"" << id << "\"";

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
}

CallersData::Record::~Record()
{
  delete inherits;
  delete inherited;
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
}

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

  js << "]}";
}

bool CallersData::operator< (const CallersData::Record& record1, const CallersData::Record& record2)
{
  return record1.name < record2.name;
}

/***************************************** class FctDecl ****************************************/

void CallersData::FctDecl::allocate()
{
  redeclarations = new std::set<ExtFct>;
  definitions = new std::set<ExtFct>;
  redefinitions = new std::set<ExtFct>;
  locallers = new std::set<std::string>;
  extcallers = new std::set<ExtFct>;
}

CallersData::FctDecl::~FctDecl()
{
  delete redeclarations;
  delete definitions;
  delete redefinitions;
  delete locallers;
  delete extcallers;
}

CallersData::FctDecl::FctDecl(const char* mangled, const char* sign, Virtuality is_virtual, const char* filepath, const int line)
  : mangled(mangled),
    sign(sign),
    file(filepath),
    virtuality(is_virtual),
    line(line)
{
  allocate();
  std::cout << "Create function declaration: " << std::endl;
  this->print_cout(sign, is_virtual, file, line);
}

CallersData::FctDecl::FctDecl(MangledName mangled, std::string sign, Virtuality is_virtual, std::string filepath, int line)
  : mangled(mangled),
    sign(sign),
    file(filepath),
    virtuality(is_virtual),
    line(line)
{
  allocate();
  std::cout << "Create function declaration: " << std::endl;
  this->print_cout(sign, is_virtual, file, line);
}

CallersData::FctDecl::FctDecl(const CallersData::FctDecl& copy_from_me)
{
  allocate();
  std::cout << "FctDecl copy constructor" << std::endl;
  mangled = copy_from_me.mangled;
  sign = copy_from_me.sign;
  virtuality = copy_from_me.virtuality;
  line = copy_from_me.line;

  // copy local callers
  std::set<std::string>::const_iterator l;
  for(l=copy_from_me.locallers->begin(); l!=copy_from_me.locallers->end(); ++l)
    {
      locallers->insert(*l);
    };

  // copy redeclarations
  std::set<ExtFct>::const_iterator x;
  for( x=copy_from_me.redeclarations->begin(); x!=copy_from_me.redeclarations->end(); ++x )
    {
      redeclarations->insert(*x);
    };

  // copy definitions
  for(x=copy_from_me.definitions->begin(); x!=copy_from_me.definitions->end(); ++x)
    {
      definitions->insert(*x);
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
    };
}

void CallersData::FctDecl::add_local_caller(MangledName mangled, std::string sign) const
{
  std::cout << "Add local caller \"" << sign << "\" to callee function declaration \"" << sign << "\"" << std::endl;
  locallers->insert(sign);
}

// TODO: filter cases where the redeclaration is already present in the fct_decl
void CallersData::FctDecl::add_redeclaration(MangledName redef_mangled, std::string redef_sign, CallersData::Virtuality redef_virtuality,
                                             std::string redef_decl_file, int redef_decl_line) const
{
  std::string redef_decl_location = redef_decl_file;
  std::ostringstream out;
  out << redef_decl_line;
  redef_decl_location += ":";
  redef_decl_location += out.str();
  // std::cout << "Check whether the redeclaration \"" << redef_sign << "\" is already present or not..." << std::endl;
  std::set<CallersData::ExtFct>::iterator search_result;
  CallersData::ExtFct searched_redecl(redef_mangled, redef_sign, redef_virtuality, redef_decl_location, E_FctDecl);
  search_result = redeclarations->find(searched_redecl);
  if(search_result != redeclarations->end())
    {
      std::cout << "Already present redeclaration \"" << redef_sign
                << "\" of function \"" << sign << "\", so do not add it twice. "
                << "Function is redefined in file: " << redef_decl_file
                << " at line: " << redef_decl_line
                << std::endl;
    }
  else
    {
      std::cout << "Add redeclaration \"" << redef_sign
                << "\" to function \"" << sign << "\". "
                << "Function is redefined in file: " << redef_decl_file
                << " at line: " << redef_decl_line
                << std::endl;
      CallersData::ExtFct redecl (redef_mangled, redef_sign, redef_virtuality, redef_decl_location, E_FctDecl);
      redeclarations->insert(redecl);
      search_result = redeclarations->find(searched_redecl);
      if(search_result != redeclarations->end())
        {
          std::cout << "the redeclaration \"" << redef_sign << "\" is well present now !" << std::endl;
        }
    }
  return;
}

void CallersData::FctDecl::add_definition(MangledName def_mangled, std::string fct_sign, std::string def_sign,
					  Virtuality def_virtuality, std::string def_file_pos) const
{
  std::cout << "Add definition \"" << def_sign
	    << "\" to function declaration \"" << fct_sign << "\"" << std::endl;
  std::cout << "Function definition is located at: " << def_file_pos << std::endl;
  ExtFct definition(def_mangled, def_sign, virtuality, def_file_pos, E_FctDef);
  definitions->insert(definition);
}

void CallersData::FctDecl::add_external_caller(MangledName caller_mangled, std::string caller_sign, Virtuality virtuality,
					       std::string file, int line) const
{
  std::cout << "Add external caller \"" << caller_sign << "\" to callee function declaration \"" << sign << "\"" << std::endl;
  std::cout << "Caller function is located at: " << file << ":" << line << std::endl;
  std::ostringstream pos;
  pos << line;
  std::string caller_file = file + ":" + pos.str();
  ExtFct extfct(caller_mangled, caller_sign, virtuality, caller_file, E_FctDef);
  extcallers->insert(extfct);
}

void CallersData::FctDecl::add_redefinition(MangledName redef_mangled, std::string redef_sign, CallersData::Virtuality redef_virtuality,
                                            std::string redef_decl_file, int redef_decl_line) const
{
  std::string redef_decl_location = redef_decl_file;
  std::ostringstream out;
  out << redef_decl_line;
  redef_decl_location += ":";
  redef_decl_location += out.str();

  std::cout << "Add redefinition \"" << redef_sign
	    << "\" to function \"" << sign << "\". "
	    << "Function is redefined in file: " << redef_decl_file
	    << " at line: " << redef_decl_line
	    << std::endl;

  ExtFct extfct (redef_mangled, redef_sign, redef_virtuality, redef_decl_location, E_FctDef);
  redefinitions->insert(extfct);
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

      std::set<ExtFct>::const_iterator x, extlast;
      //last = std::prev(definitions.end(); // requires C++ 11
      extlast = definitions->empty() ? definitions->end() : --definitions->end();
      for(x=definitions->begin(); x!=definitions->end(); ++x)
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
     << "\", \"mangled\": \"" << mangled
     << "\"";

  this->output_redeclarations(js);
  this->output_definitions(js);
  this->output_redefinitions(js);
  this->output_local_callers(js);
  this->output_external_callers(js);

  js << "}";
}

/* private functions */

void CallersData::FctDecl::print_cout(std::string sign, Virtuality virtuality, std::string file, int line)
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
             << "\"}"
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
  locallers  = new std::set<std::string>;
  locallees  = new std::set<std::string>;
  extcallers = new std::set<ExtFct>;
  extcallees = new std::set<ExtFct>;
}

CallersData::FctDef::~FctDef()
{
  delete locallers;
  delete locallees;
  delete extcallers;
  delete extcallees;
}

CallersData::FctDef::FctDef(const char* mangled, const char* sign, Virtuality is_virtual, const char* filepath, const int line)
  : mangled(mangled),
    sign(sign),
    file(filepath),
    virtuality(is_virtual),
    line(line)
{
  allocate();
  std::cout << "Create function: " << std::endl;
  this->print_cout(sign, is_virtual, file, line);
}

CallersData::FctDef::FctDef(MangledName mangled, std::string sign, Virtuality is_virtual, std::string filepath, int line)
  : mangled(mangled),
    sign(sign),
    file(filepath),
    virtuality(is_virtual),
    line(line)
{
  allocate();
  std::cout << "Create function: " << std::endl;
  this->print_cout(sign, is_virtual, file, line);
}

CallersData::FctDef::FctDef(const CallersData::FctDef& copy_from_me)
{
  allocate();
  std::cout << "FctDef copy constructor" << std::endl;
  mangled = copy_from_me.mangled;
  sign = copy_from_me.sign;
  virtuality = copy_from_me.virtuality;
  line = copy_from_me.line;

  // copy local callers
  std::set<std::string>::const_iterator i;
  for( i=copy_from_me.locallers->begin(); i!=copy_from_me.locallers->end(); ++i )
    {
      locallers->insert(*i);
    };

  // copy local callees
  for( i=copy_from_me.locallees->begin(); i!=copy_from_me.locallees->end(); ++i )
    {
      locallees->insert(*i);
    };

  // copy external callers
  std::set<ExtFct>::const_iterator x;
  for( x=copy_from_me.extcallers->begin(); x!=copy_from_me.extcallers->end(); ++x )
    {
      extcallers->insert(*x);
    };

  // copy external callees
  for( x=copy_from_me.extcallees->begin(); x!=copy_from_me.extcallees->end(); ++x )
    {
      extcallees->insert(*x);
    };
}

// void CallersData::FctDef::add_local_caller(MangledName mangled, std::string sign) const
// {
//   std::cout << "Add local caller \"" << sign << "\" to function \"" << sign << "\"" << std::endl;
//   locallers->insert(sign);
// }

void CallersData::FctDef::add_local_callee(MangledName mangled, std::string sign) const
{
  std::cout << "Add local callee \"" << sign << "\" to function \"" << sign << "\"" << std::endl;
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

void CallersData::FctDef::add_external_callee(MangledName callee_builtin, std::string callee_sign, CallersData::Virtuality callee_virtuality, std::string callee_decl_file, int callee_decl_line) const
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

  ExtFct extfct (callee_builtin, callee_sign, callee_virtuality, callee_decl_location, E_FctDecl);
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

  ExtFct extfct (builtin_mangled, builtin_sign, builtin_virtuality, builtin_decl_location, E_FctDecl);
  extcallees->insert(extfct);
}

void CallersData::FctDef::output_local_callers(std::ofstream &js) const
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

void CallersData::FctDef::output_local_callees(std::ofstream &js) const
{
  if (not locallees->empty())
    {
      js << ", \"locallees\": [";
      std::set<std::string>::const_iterator i, last;
      //last = std::prev(locallees.end(); // requires C++ 11
      last = --locallees->end();
      for( i=locallees->begin(); i!=locallees->end(); ++i )
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

void CallersData::FctDef::output_external_callers(std::ofstream &js) const
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
  out << line;
  js //<< "{\"eClass\":\"" << CALLERS_TYPE_FCT_DEF << "\", \"sign\": \"" << sign
     << "{\"sign\": \"" << sign
     << "\", \"line\": " << out.str()
     << ", \"virtuality\": \""
     << ((virtuality == VNoVirtual) ? "no"
	 : ((virtuality == VVirtualDeclared) ? "declared"
	    : ((virtuality == VVirtualDefined) ? "defined"
	       : /* virtuality == VVirtualPure */ "pure")))
     << "\", \"mangled\": \"" << mangled
     << "\"";

  this->output_local_callers(js);
  this->output_local_callees(js);
  this->output_external_callers(js);
  this->output_external_callees(js);

  js << "}";
}

/* private functions */

void CallersData::FctDef::print_cout(std::string sign, Virtuality virtuality, std::string file, int line)
{
  std::ostringstream loc;
  loc << line;

  std::cout << "{\"sign\":\"" << sign
	    << "\",\"line\":\"" << loc.str()
	    << "\",\"virtuality\":\"" << ((virtuality == CallersData::VNoVirtual) ? "no"
				       : ((virtuality == CallersData::VVirtualDeclared) ? "declared"
					  : ((virtuality == CallersData::VVirtualDefined) ? "defined"
					     : "pure")))
	    << "\",\"file\":\"" << file
             << "\"}"
	    << std::endl;
}

/* public functions */

bool CallersData::operator< (const CallersData::FctDef& fct1, const CallersData::FctDef& fct2)
{
  return fct1.sign < fct2.sign;
}

/**************************************** class FctCall ***************************************/

CallersData::FctCall::FctCall(MangledName caller_mangled, std::string caller_sign, Virtuality is_caller_virtual, std::string caller_file, int caller_line,
			      MangledName callee_mangled, std::string callee_sign, Virtuality is_callee_virtual, std::string callee_decl_file, int callee_decl_line)
  : caller_mangled(caller_mangled),
    caller_sign(caller_sign),
    caller_virtuality(is_caller_virtual),
    caller_file(caller_file),
    caller_line(caller_line),
    callee_mangled(callee_mangled),
    callee_sign(callee_sign),
    callee_virtuality(is_callee_virtual),
    callee_decl_file(callee_decl_file),
    callee_decl_line(callee_decl_line),
    id(caller_sign + " -> " + callee_sign)
{
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

CallersData::ExtFct::ExtFct(MangledName mangled, std::string sign, Virtuality is_virtual, std::string fct, FctKind kind)
  : mangled(mangled),
    sign(sign),
    virtuality(is_virtual),
    fct(fct),
    kind(kind)
{
  std::cout << "Create external function: " << std::endl;
  this->print_cout(mangled, sign, is_virtual, fct, kind);
}

CallersData::ExtFct::ExtFct(const CallersData::ExtFct& copy_from_me)
{
  mangled = copy_from_me.mangled;
  sign = copy_from_me.sign;
  virtuality = copy_from_me.virtuality;
  fct = copy_from_me.fct;
  kind = copy_from_me.kind;
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
