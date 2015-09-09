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

#include "CallersData.hpp"

/***************************************** class JsonFileWriter ****************************************/

CallersData::JsonFileWriter::JsonFileWriter(std::string jsonFileName)
  : fileName(jsonFileName), out()
{ 
  std::cout << "Open file \"" << jsonFileName << "\" in write mode." << std::endl;
  out.open(jsonFileName.c_str());
}

CallersData::JsonFileWriter::~JsonFileWriter() 
{
  out.close();
}

/***************************************** class Dir ****************************************/

CallersData::Dir::Dir(std::string dir, std::string path)
  : dir(dir), 
    path(path),
    jsonfilename(path + "/" + dir + "/" + dir + ".dir.callers.gen.json")
{}

CallersData::Dir::~Dir() {}

std::string CallersData::Dir::fullPath()
{
  std::string fullPath = path + "/" + dir;
  return fullPath;
}

void CallersData::Dir::add_file(std::string file)
{
  std::cout << "Register file \"" << file << "\" in directory \"" << this->fullPath() << "\"" << std::endl;
  files.push_back(file);
}

void CallersData::Dir::output_json_desc()
{
  CallersData::JsonFileWriter js(this->jsonfilename);
  js.out << "{\"dir\":\"" << dir << "\",\"path\":\"" << path << "\",\"files\":[";

  std::list<std::string>::const_iterator i, last;
  last = files.empty() ? files.end() : --files.end();
  for(i=files.begin(); i!=files.end(); ++i)
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
    jsonfilename(path + "/" + file + ".file.callers.gen.json")
{}

void CallersData::File::parse_json_file()
{
  /* Check whether the related json file does already exists or not. */
  //std::string jsonfilename = this->js.fileName;
  std::string jsonfilename = this->path + "/" + file + ".file.callers.gen.json";
  FILE* pFile = fopen(jsonfilename.c_str(), "rb");
  // Always check to see if file opening succeeded
  if (pFile == NULL)
    std::cout << "WARNING: Could not open file \"" << jsonfilename << "\"\n";
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
	  assert(file.HasMember("path"));
	  assert(file.HasMember("defined"));

	  std::string filename(file["file"].GetString());
	  std::string dirpath(file["path"].GetString());
	  std::string filepath(dirpath + "/" + filename);

	  std::cout << "file: " << filename << std::endl;
	  std::cout << "path: " << dirpath << std::endl;

	  const rapidjson::Value& defined = file["defined"];
	  assert(defined.IsArray());

	  // rapidjson uses SizeType instead of size_t.
	  for (rapidjson::SizeType s = 0; s < defined.Size(); s++)
	    {
	      const rapidjson::Value& symb = defined[s];
	      const rapidjson::Value& sign = symb["sign"];
	      const rapidjson::Value& line = symb["line"];
	      std::string symbol = sign.GetString();
	      int pos = line.GetInt();
	      std::ostringstream spos;
	      spos << pos;
	      std::string location(filepath + ":" + spos.str());
	      //symbol_location.insert(SymbLoc::value_type(symbol, location));
	      this->add_defined_function(symbol, this->file, pos);
	      std::cout << "Parsed symbol s[" << s << "]:\"" << symbol << "\"" << std::endl;
	    }
	}
      else
	{
	  std::cout << "WARNING: Empty or Malformed file \"" << jsonfilename << "\"\n";
	}
    }
}


CallersData::File::~File() {}

std::string CallersData::File::fullPath()
{
  std::string fullPath = path + "/" + file;
  return fullPath;
}

void CallersData::File::add_defined_function(CallersData::Fct* fct)
{
  std::cout << "Register function \"" << fct->sign
	    << "\" defined in file \"" << this->fullPath() << ":" 
	    << fct->line << "\"" << std::endl;
  defined.insert(*fct);
}

void CallersData::File::add_defined_function(std::string sign, std::string filepath, int line)
{
  std::cout << "Create function \"" << sign
	    << "\" located in file \"" << this->fullPath() << ":" << line << "\"" << std::endl;
  Fct *fct = new Fct(sign, filepath, line);
  defined.insert(*fct);
}

void
CallersData::File::add_function_call(CallersData::FctCall* fc)
{ 
  std::cout << "Register function call from caller \"" << fc->caller_file << ":" << fc->caller_sign
            << "\" to callee \"" << fc->callee_decl_file << ":" << fc->callee_sign
	    << "\" in file \"" << this->fullPath() << "\"" << std::endl;

  calls.insert(*fc);

  std::set<CallersData::Fct>::const_iterator caller, callee;

  // Check whether the caller function belongs to the current file.
  if( fc->caller_file == this->fullPath() )

    // the caller function belongs to the current file
    {
      // adds the caller function to the defined functions of the current file
      add_defined_function(fc->caller_sign, fc->caller_file, fc->caller_line);
      // get a reference to the related defined function
      CallersData::Fct caller_fct(fc->caller_sign, fc->caller_file, fc->caller_line);
      caller = defined.find(caller_fct);

      // Check whether the callee function belongs to the current file.
      if( fc->callee_decl_file == this->fullPath() )

	// the callee function belongs to the current file
	{
	  // adds the callee function to the defined functions of the current file
	  add_defined_function(fc->callee_sign, fc->callee_decl_file, fc->callee_decl_line);
	  // get a reference to the related defined function
	  CallersData::Fct callee_fct(fc->callee_sign, fc->callee_decl_file, fc->callee_decl_line);
	  callee = defined.find(callee_fct);

	  // add local caller to local callee
	  callee->add_local_caller(fc->caller_sign);

	  // add local callee to local caller
	  caller->add_local_callee(fc->callee_sign);
	}
      else
	// the callee function is defined externally
	{
	  // add the external callee to the local caller
	  std::string callee_decl_location = fc->callee_decl_file;
	  std::ostringstream out;
	  out << fc->callee_decl_line;
	  callee_decl_location += ":";
	  callee_decl_location += out.str();
	  caller->add_external_callee(fc->callee_sign, callee_decl_location);
	}
    }

  else
    // the caller function does not belong to the current file
    {
      // check whether the callee function does really belong to the current file as expected
      if( fc->callee_decl_file == this->fullPath() )

	// the callee function belongs to the current file
	{
	  // adds the callee function to the defined functions of the current file
	  add_defined_function(fc->callee_sign, fc->callee_decl_file, fc->callee_decl_line);
	  // get a reference to the related defined function
	  CallersData::Fct callee_fct(fc->callee_sign, fc->callee_decl_file, fc->callee_decl_line);
	  callee = defined.find(callee_fct);

	  // add the external caller to the local callee
	  callee->add_external_caller(fc->caller_sign, fc->caller_file);
	}
      else
	// the callee function is defined externally as the caller !!!
	{
	  std::cerr << "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww" << std::endl;
	  std::cerr << "The callee function is defined externally as the caller, so we ignore both of them !!!" << std::endl;
	  std::cerr << "current file: " << this->fullPath() << std::endl;
	  std::cerr << "caller file: " << fc->caller_file << std::endl;
	  std::cerr << "callee decl file: " << fc->callee_decl_file << std::endl;
	  std::cerr << "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww" << std::endl;
	  //abort();
	}
    }
}

void CallersData::File::output_json_desc()
{
  CallersData::JsonFileWriter js(this->jsonfilename);
  js.out << "{\"file\":\"" << file << "\",\"path\":\"" << path << "\",\"defined\":[";
  
  std::set<Fct>::const_iterator i, last;
  last = defined.empty() ? defined.end() : --defined.end();
  for(i=defined.begin(); i!=defined.end(); ++i)
    {
      if(i != last)
	{
	  i->output_json_desc(js.out);
	  js.out << ",";
	}
      else
	{
	  i->output_json_desc(js.out);
	}
    }

  js.out << "]}" << std::endl;
}

/***************************************** class Fct ****************************************/

void CallersData::Fct::allocate() 
{
  locallers  = new std::set<std::string>;
  locallees  = new std::set<std::string>;
  extcallers = new std::set<ExtFct>;
  extcallees = new std::set<ExtFct>;
}

CallersData::Fct::~Fct() 
{
  delete locallers;
  delete locallees;
  delete extcallers;
  delete extcallees;
}

CallersData::Fct::Fct(const char* sign, const char* filepath, const int line)
  : sign(sign),
    file(filepath),
    line(line)
{
  allocate();
}

CallersData::Fct::Fct(std::string sign, std::string filepath, int line)
  : sign(sign),
    file(filepath),
    line(line)
{
  allocate();
}

CallersData::Fct::Fct(const CallersData::Fct& copy_from_me)
{
  allocate();
  std::cout << "Fct copy constructor" << std::endl;
  sign = copy_from_me.sign;
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

void CallersData::Fct::add_local_caller(std::string caller) const
{
  std::cout << "Add local caller \"" << caller << "\" to function \"" << sign << "\"" << std::endl;
  locallers->insert(caller);
}

void CallersData::Fct::add_local_callee(std::string callee) const
{
  std::cout << "Add local callee \"" << callee << "\" to function \"" << sign << "\"" << std::endl;
  locallees->insert(callee);
}

void CallersData::Fct::add_external_caller(std::string caller_sign, std::string caller_decl) const
{
  std::cout << "Add external caller \"" << caller_sign << "\" to function \"" << sign << "\"" << std::endl;
  ExtFct *extfct = new ExtFct(caller_sign, caller_decl);
  extcallers->insert(*extfct);
}

void CallersData::Fct::add_external_callee(std::string callee_sign, std::string callee_decl) const
{
  std::cout << "Add external callee \"" << callee_sign
	    << "\" to function \"" << sign << "\". " 
	    << "Callee is declared in file: " << callee_decl
	    << std::endl;
  ExtFct *extfct = new ExtFct(callee_sign, callee_decl);
  extcallees->insert(*extfct);
}

void CallersData::Fct::output_local_callers(std::ofstream &js) const
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

void CallersData::Fct::output_local_callees(std::ofstream &js) const
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

void CallersData::Fct::output_external_callers(std::ofstream &js) const
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

void CallersData::Fct::output_external_callees(std::ofstream &js) const
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

void CallersData::Fct::output_json_desc(std::ofstream &js) const
{
  std::ostringstream out;
  out << line;
  js << "{\"sign\": \"" << sign
     << "\", \"line\": " << out.str() << "";

  this->output_local_callers(js);

  this->output_local_callees(js);

  this->output_external_callers(js);

  this->output_external_callees(js);
  
  js << "}";
}

bool CallersData::operator< (const CallersData::Fct& fct1, const CallersData::Fct& fct2)
{
  return fct1.sign < fct2.sign;
}

/**************************************** class FctCall ***************************************/

CallersData::FctCall::FctCall(std::string caller_sign, std::string caller_file, int caller_line,
			      std::string callee_sign, std::string callee_decl_file, int callee_decl_line)
  : caller_sign(caller_sign),
    caller_file(caller_file),
    caller_line(caller_line),
    callee_sign(callee_sign),
    callee_decl_file(callee_decl_file),
    callee_decl_line(callee_decl_line),
    id(caller_sign + " -> " + callee_sign)
{}

bool CallersData::operator< (const CallersData::FctCall& fc1, const CallersData::FctCall& fc2)
{
  return fc1.id < fc2.id;
}

/**************************************** class ExtFct ***************************************/

CallersData::ExtFct::ExtFct(std::string sign, std::string decl)
  : sign(sign), decl(decl), def("unknownExtFctDef")
{
  std::cout << "Create external function: "
	    << "{\"sign\":\"" << sign
	    << "\",\"decl\":\"" << decl
	    << "\",\"def\":\"" << def << "\"}"
	    << std::endl;
}

CallersData::ExtFct::ExtFct(const CallersData::ExtFct& copy_from_me)
{
  sign = copy_from_me.sign;
  decl = copy_from_me.decl;
  def = copy_from_me.def;
  std::cout << "Copy external function: "
	    << "{\"sign\":\"" << sign
	    << "\",\"decl\":\"" << decl
	    << "\",\"def\":\"" << def << "\"}"
	    << std::endl;
}

std::ostream &CallersData::operator<<(std::ostream &output, const ExtFct &fct)
{
  output << "{\"sign\":\"" << fct.sign
	 << "\",\"decl\":\"" << fct.decl
	 << "\",\"def\":\"" << fct.def << "\"}";
  return output;
}

bool CallersData::operator< (const CallersData::ExtFct& fct1, const CallersData::ExtFct& fct2)
{
  return fct1.sign < fct2.sign;
}
