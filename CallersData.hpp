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

#include <list>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <rapidjson/document.h>

namespace CallersData 
{
  class JsonFileWriter
  {
    public:
      JsonFileWriter(std::string jsonFileName);
      ~JsonFileWriter();
      std::string fileName;
      std::ofstream out;
  };

  class Dir
  {
    public:
      Dir(std::string dir, std::string path);
      ~Dir();
      std::string fullPath ();
      void add_children(std::string dir);
      void add_file(std::string file);
      void output_json_desc();
    private:
      std::string dir;
      std::string path;
      std::list<std::string> files;
      std::list<std::string> childrens;
      JsonFileWriter js;
  };

  class Fct;
  class FctCall;
  class Symbols;

  class File
  {
    public:
      File(std::string file, std::string path);
      ~File();
      std::string fullPath ();
      void add_defined_function(Fct* fct);
      void add_defined_function(std::string func, int sign);
      void add_function_call(FctCall* fc, const CallersData::Symbols& defined_symbols);
      //void sort_local_and_external_function_calls(const CallersData::Symbols& defined_symbols);
      void output_json_desc();
      std::set<Fct> defined;
  private:
      std::set<FctCall> calls;
      std::string file;
      std::string path;
      //std::list<std::string> defined;
      JsonFileWriter js;
  };

  /* Used to store function call before knowing if it's a local or an external call */
  class FctCall
  {
    friend bool operator< (const CallersData::FctCall& fc1, const CallersData::FctCall& fc2);
    friend void File::add_function_call(FctCall* fc, const CallersData::Symbols& defined_symbols);
    //friend void File::sort_local_and_external_function_calls(const CallersData::Symbols& defined_symbols);

    public:
    FctCall(std::string caller_sign, int caller_line, 
	    std::string callee_sign, int callee_line);
      //FctCall(const FctCall& copy_from_me);
      ~FctCall() {}
    protected:
      std::string caller_sign;
      int caller_line;
      std::string callee_sign;
      int callee_line;
    private:
      std::string id;
  };

  class ExtFct
  {
    friend std::ostream &operator<<(std::ostream &output, const ExtFct &fct);
    friend bool operator< (const CallersData::ExtFct& fct1, const CallersData::ExtFct& fct2);

    public:
      ExtFct(std::string sign, std::string file);
      ExtFct(const ExtFct& copy_from_me);
      ~ExtFct() {}
      //void set_file(std::string file);
    private:
      std::string sign;
      std::string file;
  };

  std::ostream &operator<<(std::ostream &output, const ExtFct &fct);

  class Fct
  {
    public:
      Fct(const char* sign, const int line);
      Fct(std::string sign, int line);
      Fct(const Fct& copy_from_me);
      ~Fct();

      //std::list<Fct>& operator=( const std::list<Fct>& other );
      //std::ostream& operator<<(const Fct& __f) {}; // return _M_insert(__f); }
      void add_local_caller(std::string caller) const;
      void add_local_callee(std::string callee) const;
      void add_external_caller(std::string caller_sign, std::string caller_file) const;
      void add_external_callee(std::string callee_sign, std::string callee_file) const;

      void output_local_callers(std::ofstream &js) const;
      void output_local_callees(std::ofstream &js) const;
      void output_external_callers(std::ofstream &js) const;
      void output_external_callees(std::ofstream &js) const;

      void output_json_desc(std::ofstream &js) const;

      std::string sign;
      int line;
      std::set<std::string> *locallers;
      std::set<std::string> *locallees;
      std::set<ExtFct> *extcallers;
      std::set<ExtFct> *extcallees;

    private:
      void allocate();
  };

  bool operator< (const Fct& fct1, const Fct& fct2);

  typedef std::map<std::string, std::string> SymbLoc;

  class Symbols
  {
  public:
    Symbols(std::string defined_symbols_jsonfilename);
    std::string get_filename(std::string symbol) const;
    std::string get_filepath(std::string symbol) const;
    ~Symbols();
  private:
    std::string defined_symbols_jsonfilename;
    //rapidjson::Document document;    
    SymbLoc symbol_location;
  };  
}
