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
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

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

  class File;

  class Dir
  {
    public:
      Dir();
      Dir(std::string dir, std::string path);
      ~Dir();
      std::string fullPath ();
      void add_children(std::string dir);
      void add_file(std::string file);
      void add_file(File *file);
      std::set<File>::iterator get_file(std::string filename, std::string dirpath);
      void output_json_files();
      void output_json_dir();
    private:
      std::string dir = "unknownDirName";
      std::string path = "unknownDirPath";
      std::list<std::string> filenames;
      std::list<std::string> childrens;
      std::string jsonfilename = "unknownJsonFileName";
      std::set<File> files;
      //JsonFileWriter js;
  };

  class Record;
  class Fct;
  class FctCall;

  class File
  {
    friend bool operator< (const CallersData::File& file1, const CallersData::File& file2);

    public:
      File(std::string file, std::string path);
      ~File();
      std::string fullPath () const;
      void parse_json_file() const;
      void add_record(Record* record) const;
      void add_declared_function(Fct* fct) const;
      void add_declared_function(std::string func, std::string filepath, int sign) const;
      void add_defined_function(Fct* fct) const;
      void add_defined_function(std::string func, std::string filepath, int sign) const;
      void add_function_call(FctCall* fc, Dir *context) const;
      void output_json_desc() const;
      std::set<Fct> *declared;
      std::set<Fct> *defined;
      std::set<Record> *records;
  private:
      std::set<FctCall> *calls;
      std::string file = "unknownFileName";
      std::string path = "unknownFilePath";
      std::string jsonfilename = "unknownJsonFileName";
  };

  /* Used to store function call before knowing if it's a local or an external call */
  class FctCall
  {
    friend bool operator< (const CallersData::FctCall& fc1, const CallersData::FctCall& fc2);
    friend void File::add_function_call(FctCall* fc, Dir* context) const;

    public:
      FctCall(std::string caller_sign, std::string caller_file, int caller_line, 
	    std::string callee_sign, std::string callee_decl_file, int callee_decl_line);
      //FctCall(const FctCall& copy_from_me);
      ~FctCall() {}
      bool is_builtin = false;
    protected:
      std::string caller_sign = "unknownCallerSign";
      std::string caller_file = "unknownCallerFile";
      int caller_line = -1;
      std::string callee_sign = "unknownCalleeSign";
      std::string callee_decl_file = "unknownCalleeDeclFile";
      int callee_decl_line = -1;
      std::string callee_def_file = "unknownCalleeDefFile";
      int callee_def_line = -1;
    private:
      std::string id;
  };

  class ExtFct
  {
    friend std::ostream &operator<<(std::ostream &output, const ExtFct &fct);
    friend bool operator< (const CallersData::ExtFct& fct1, const CallersData::ExtFct& fct2);

    public:
      ExtFct(std::string sign, std::string decl);
      ExtFct(std::string sign, std::string decl, std::string def);
      ExtFct(const ExtFct& copy_from_me);
      ~ExtFct() {}
      //void set_file(std::string file);
    private:
      std::string sign = "unknownExtFctSign";
      std::string decl = "unknownExtFctDeclLoc";
      std::string def  = "unknownExtFctDefLoc";
  };

  std::ostream &operator<<(std::ostream &output, const ExtFct &fct);

  enum RecordType { RStruct = 0, RClass = 1 };

  class Record
  {
    public:
      Record(const char* name, const char* filepath, const int line, const int kind = RClass, const bool is_abstract = false);
      Record(std::string name, std::string filepath, int line, int kind = RClass, bool is_abstract = false);
      Record(const Record& copy_from_me);
      ~Record();

      void add_base_class(std::string base_class) const;
      void add_child_class(std::string child_class) const;

      void output_base_classes(std::ofstream &js) const;
      void output_child_classes(std::ofstream &js) const;

      void output_json_desc(std::ofstream &js) const;

      std::string name = "unknownRecordName";
      std::string file = "unknownRecordFile";
      int line = -1;
      std::set<std::string> *inherits;
      std::set<std::string> *inherited;

    private:
      void allocate();
      int kind;
      bool is_abstract;
      static const char* RecordKind[];
  };

  bool operator< (const Record& rec1, const Record& rec2);

  class Fct
  {
    public:
      Fct(const char* sign, const char* filepath, const int line);
      Fct(std::string sign, std::string filepath, int line);
      Fct(const Fct& copy_from_me);
      ~Fct();

      void add_local_caller(std::string caller) const;
      void add_local_callee(std::string callee) const;
      void add_external_caller(std::string caller_sign, std::string caller_decl) const;
      void add_external_callee(std::string callee_sign, std::string callee_decl_file, int callee_decl_line) const;
      void add_builtin_callee(std::string callee_sign, std::string builtin_decl_file, int builtin_decl_line) const;

      void output_local_callers(std::ofstream &js) const;
      void output_local_callees(std::ofstream &js) const;
      void output_external_callers(std::ofstream &js) const;
      void output_external_callees(std::ofstream &js) const;

      void output_json_desc(std::ofstream &js) const;

      std::string sign = "unknownFctSign";
      std::string file = "unknownFctFile";
      int line = -1;
      std::set<std::string> *locallers;
      std::set<std::string> *locallees;
      std::set<ExtFct> *extcallers;
      std::set<ExtFct> *extcallees;

      void set_defined();
      void set_declared();

      bool is_declared();
      bool is_defined();

    private:
      void allocate();
      bool defined = false;
  };

  bool operator< (const Fct& fct1, const Fct& fct2);
}
