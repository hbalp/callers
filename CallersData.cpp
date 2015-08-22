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

#include <iostream>
#include <boost/filesystem.hpp>
#include <rapidjson/filereadstream.h>

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
    js(path + "/" + dir + "/" + dir + ".dir.callers.gen.json")
{ 
  // jsonFileName = path + "/" + dir + "/" + dir + ".dir.callers.gen.json";
  // std::cout << "Open file \"" << jsonFileName << "\" in write mode." << std::endl;
  // jOut.open(jsonFileName.c_str());
}

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
    js(path + "/" + file + ".file.callers.gen.json")
{}

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

void CallersData::File::add_defined_function(std::string sign, int line)
{
  std::cout << "Create function \"" << sign
	    << "\" located in file \"" << this->fullPath() << ":" << line << "\"" << std::endl;
  Fct *fct = new Fct(sign, line);
  defined.insert(*fct);
}

/* We use the defined_symbols to decide whether the caller and/or 
   callee function belongs to the current file. */
void
CallersData::File::add_function_call(CallersData::FctCall* fc, 
				     const CallersData::Symbols& defined_symbols)
{ 
  // Retrieve the filename of the caller function
  std::string caller_filename = defined_symbols.get_filename(fc->caller_sign);

  // Retrieve the filename of the callee function
  std::string callee_filename = defined_symbols.get_filename(fc->callee_sign);

  std::cout << "Register function call from caller \"" << caller_filename << ":" << fc->caller_sign
            << "\" to callee \"" << callee_filename << ":" << fc->callee_sign
	    << "\" in file \"" << this->fullPath() << "\"" << std::endl;

  calls.insert(*fc);

  std::set<CallersData::Fct>::const_iterator caller, callee;

  // Check whether the caller function belongs to the current file.
  if( caller_filename == file )

    // the caller function belongs to the current file
    {
      // adds the caller function to the defined functions of the current file
      add_defined_function(fc->caller_sign, fc->caller_line);
      // get a reference to the related defined function
      CallersData::Fct caller_fct(fc->caller_sign, fc->caller_line);
      caller = defined.find(caller_fct);

      // Check whether the callee function belongs to the current file.
      if( callee_filename == file )

	// the callee function belongs to the current file
	{
	  // adds the callee function to the defined functions of the current file
	  add_defined_function(fc->callee_sign, fc->callee_line);
	  // get a reference to the related defined function
	  CallersData::Fct callee_fct(fc->callee_sign, fc->callee_line);
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
	  std::string external_file = defined_symbols.get_filepath(fc->callee_sign);
	  caller->add_external_callee(fc->callee_sign, external_file);
	}
    }

  else
    // the caller function does not belong to the current file
    {
      // check whether the callee function does really belong to the current file as expected
      if( callee_filename == file )

	// the callee function belongs to the current file
	{
	  // adds the callee function to the defined functions of the current file
	  add_defined_function(fc->callee_sign, fc->callee_line);
	  // get a reference to the related defined function
	  CallersData::Fct callee_fct(fc->callee_sign, fc->callee_line);
	  callee = defined.find(callee_fct);

	  // add the external caller to the local callee
	  std::string external_file = defined_symbols.get_filepath(fc->caller_sign);
	  callee->add_external_caller(fc->caller_sign, external_file);
	}
      else
	// the callee function is defined externally as the caller !!!
	{
	  std::cerr << "The callee function is defined externally as the caller !!!" << std::endl;
	  abort();
	}
    }
}

void CallersData::File::output_json_desc()
{
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

CallersData::Fct::Fct(const char* sign, const int line)
  : sign(sign),
    line(line)
{
  allocate();
}

CallersData::Fct::Fct(std::string sign, int line)
  : sign(sign),
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

void CallersData::Fct::add_external_caller(std::string caller_sign, std::string caller_file) const
{
  std::cout << "Add external caller \"" << caller_sign << "\" to function \"" << sign << "\"" << std::endl;
  ExtFct *extfct = new ExtFct(caller_sign, caller_file);
  extcallers->insert(*extfct);
}

void CallersData::Fct::add_external_callee(std::string callee_sign, std::string callee_file) const
{
  std::cout << "Add external callee \"" << callee_sign << "\" to function \"" << sign << "\"" << std::endl;
  ExtFct *extfct = new ExtFct(callee_sign, callee_file);
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
  js << "{\"sign\": \"" << sign
     << "\", \"line\": " << line << "";

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

CallersData::FctCall::FctCall(std::string caller_sign, int caller_line,
			      std::string callee_sign, int callee_line)
  : caller_sign(caller_sign),
    caller_line(caller_line),
    callee_sign(callee_sign),
    callee_line(callee_line),
    id(caller_sign + " -> " + callee_sign)
{}

bool CallersData::operator< (const CallersData::FctCall& fc1, const CallersData::FctCall& fc2)
{
  return fc1.id < fc2.id;
}

/**************************************** class ExtFct ***************************************/

CallersData::ExtFct::ExtFct(std::string sign, std::string file)
  : sign(sign), file(file)
{
  std::cout << "ExtFct constructor" << std::endl;
}

CallersData::ExtFct::ExtFct(const CallersData::ExtFct& copy_from_me)
{
  std::cout << "ExtFct copy constructor" << std::endl;
  sign = copy_from_me.sign;
  file = copy_from_me.file;
}

std::ostream &CallersData::operator<<(std::ostream &output, const ExtFct &fct)
{
  output << "{\"sign\":\"" << fct.sign << "\",\"file\":\"" << fct.file << "\"}";
  return output;
}

bool CallersData::operator< (const CallersData::ExtFct& fct1, const CallersData::ExtFct& fct2)
{
  return fct1.sign < fct2.sign;
}

/**************************************** class Symbols ***************************************/

CallersData::Symbols::Symbols(std::string defined_symbols_jsonfilename)
  : defined_symbols_jsonfilename(defined_symbols_jsonfilename)
{
  FILE* pFile = fopen(defined_symbols_jsonfilename.c_str(), "rb");
  // Always check to see if file opening succeeded
  if (pFile == NULL)
    {
      std::cout << "ERROR: Could not open file \"" << defined_symbols_jsonfilename << "\" !\n";
      exit(-1);
    }
  else
    // Parse symbol locations
    {
      char buffer[65536];
      rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
      rapidjson::Document document;    
      document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);
      assert(document.IsObject());    // Document is a JSON value that represents the root of DOM. Root can be either an object or array.
      assert(document.HasMember("defined_symbols"));
      const rapidjson::Value& files = document["defined_symbols"]; // Using a reference for consecutive access is handy and faster.
      assert(files.IsArray());
      
      for (rapidjson::SizeType f = 0; f < files.Size(); f++) // rapidjson uses SizeType instead of size_t.
	{
	  const rapidjson::Value& file = files[f];

	  std::string filename(file["file"].GetString());
	  std::string dirpath(file["path"].GetString());
	  std::string filepath(dirpath + "/" + filename);

          // std::cout << "file: " << dirpath << "/" << filename << std::endl;
          // std::cout << "file: " << filename << std::endl;
          // std::cout << "path: " << dirpath << std::endl;

          const rapidjson::Value& defined = file["defined"];
          assert(defined.IsArray());

          // rapidjson uses SizeType instead of size_t.
          for (rapidjson::SizeType s = 0; s < defined.Size(); s++)
            {
              const rapidjson::Value& symb = defined[s];
              const rapidjson::Value& sign = symb["sign"];
              std::string symbol = sign.GetString();
              symbol_location.insert(SymbLoc::value_type(symbol, filepath));
              // std::cout << "s[" << s << "]:\"" << symbol << "\"" << std::endl;
            }  
	}
    }
}

std::string CallersData::Symbols::get_filepath(std::string searched_symbol) const
{
  std::string filepath("unknown_location");

  std::cout << "Check in which file is defined the symbol \"" << searched_symbol << "\"..." << std::endl;

  // Get target symbol location
  std::map<std::string, std::string>::const_iterator symb;
  for(symb=symbol_location.begin(); symb!=symbol_location.end(); ++symb)
    {
      if(searched_symbol.find(symb->first) != std::string::npos)
	{
	  std::cout << "Object symbol \"" << symb->first << "\" is a substring of searched symbol \"" << searched_symbol << "\" and is located in file: \"" << symb->second << "\"" << std::endl;
	  filepath = symb->second;
	}
    }

  if(filepath == "unknown_location")
    {
      std::cout << "Not found symbol \"" << searched_symbol << "\"" << std::endl;
    }

  return filepath;
}

std::string CallersData::Symbols::get_filename(std::string searched_symbol) const
{
  std::string filepath = get_filepath(searched_symbol);
  boost::filesystem::path p(filepath);
  std::string filename = p.filename().string();
  return filename;
}

CallersData::Symbols::~Symbols() {}
