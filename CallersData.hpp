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

  enum Virtuality { VNoVirtual, VVirtualDeclared, VVirtualDefined, VVirtualPure };

  class File
  {
    friend bool operator< (const CallersData::File& file1, const CallersData::File& file2);

    public:
      File(std::string file, std::string path);
      ~File();
      std::string fullPath () const;
      void parse_json_file() const;
      void add_defined_function(Fct* fct) const;
      void add_defined_function(std::string func, Virtuality virtuality, std::string filepath, int sign) const;
      void add_record(Record record) const;
      void add_record(std::string name, clang::TagTypeKind kind, int loc) const;
      void add_function_call(FctCall* fc, Dir *context) const;
      void output_json_desc() const;
      std::set<Fct> *defined;
      std::set<Record> *records;
  private:
      std::set<FctCall> *calls;
      std::string file = "unknownFileName";
      std::string path = "unknownFilePath";
      std::string jsonfilename = "unknownJsonFileName";
      //std::list<std::string> defined;
      //JsonFileWriter js;
  };

  /* Inheritance class to reference base classes*/
  class Inheritance
  {
    friend bool operator< (const CallersData::Inheritance& inheritance1, const CallersData::Inheritance& inheritance2);

  public:
    Inheritance(const char* name, const char* decl);
    Inheritance(std::string name, std::string decl);
    Inheritance(const CallersData::Inheritance& copy_from_me);
    ~Inheritance() {}
    void allocate();
    void output_json_desc(std::ofstream &js) const;
    inline void print_cout() const;
    std::string name = "unknownBaseClass";
    std::string decl = "unknownBaseClassLocation";
  private:
  };

  /* Record class to store "class" or "struct" definitions */
  class Record
  {
    friend std::ostream &operator<<(std::ostream &output, const Record &rec);
    friend bool operator< (const CallersData::Record& rec1, const CallersData::Record& rec2);

    public:
      Record(const char* name, clang::TagTypeKind kind, const int loc);
      Record(std::string name, clang::TagTypeKind kind, int loc);
      Record(const Record& copy_from_me);
      ~Record();
      void allocate();
      void add_inheritance(Inheritance inheritance) const;
      void add_inheritance(std::string name, std::string decl) const;
      void output_json_desc(std::ofstream &js) const;
      void print_cout() const;
      std::string name = "unknownRecordName";
      clang::TagTypeKind kind = clang::TTK_Struct;
      int loc = -1;
      std::set<Inheritance> *inherits;
    private:
  };

  bool operator< (const Record& record1, const Record& record2);

  /* Used to store function call before knowing if it's a local or an external call */
  class FctCall
  {
    friend bool operator< (const CallersData::FctCall& fc1, const CallersData::FctCall& fc2);
    friend void File::add_function_call(FctCall* fc, Dir* context) const;

    public:
      FctCall(std::string caller_sign, Virtuality caller_virtuality, std::string caller_file,
	int caller_line, std::string callee_sign, Virtuality callee_virtuality,
	std::string callee_decl_file, int callee_decl_line);
      //FctCall(const FctCall& copy_from_me);
      ~FctCall() {}
      bool is_builtin = false;
    protected:
      std::string caller_sign = "unknownCallerSign";
      Virtuality caller_virtuality = VNoVirtual;
      std::string caller_file = "unknownCallerFile";
      int caller_line = -1;
      std::string callee_sign = "unknownCalleeSign";
      Virtuality callee_virtuality = VNoVirtual;
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
      ExtFct(std::string sign, Virtuality is_virtual, std::string decl);
      ExtFct(std::string sign, Virtuality is_virtual, std::string decl, std::string def);
      ExtFct(const ExtFct& copy_from_me);
      ~ExtFct() {}
      //void set_file(std::string file);
    private:
      inline void print_cout(std::string sign, Virtuality is_virtual, std::string decl, std::string def);
      std::string sign = "unknownExtFctSign";
      Virtuality virtuality = VNoVirtual;
      std::string decl = "unknownExtFctDeclLoc"; 
      std::string def  = "unknownExtFctDefLoc";
  };

  std::ostream &operator<<(std::ostream &output, const ExtFct &fct);

  class Fct
  {
    public:
      Fct(const char* sign, Virtuality is_virtual, const char* filepath, const int line);
      Fct(std::string sign, Virtuality is_virtual, std::string filepath, int line);
      Fct(const Fct& copy_from_me);
      ~Fct();

      void add_local_caller(std::string caller) const;
      void add_local_callee(std::string callee) const;
      void add_external_caller(std::string caller_sign, Virtuality virtuality, std::string caller_decl) const;
      void add_external_callee(std::string callee_sign, Virtuality virtuality, std::string callee_decl_file, int callee_decl_line) const;
      void add_builtin_callee(std::string callee_sign, Virtuality builtin_virtuality, std::string builtin_decl_file, int builtin_decl_line) const;

      void output_local_callers(std::ofstream &js) const;
      void output_local_callees(std::ofstream &js) const;
      void output_external_callers(std::ofstream &js) const;
      void output_external_callees(std::ofstream &js) const;
      void output_json_desc(std::ofstream &js) const;

      std::string sign = "unknownFctSign";
      std::string file = "unknownFctFile";
      Virtuality virtuality = VNoVirtual;
      int line = -1;
      std::set<std::string> *locallers;
      std::set<std::string> *locallees;
      std::set<ExtFct> *extcallers;
      std::set<ExtFct> *extcallees;

    private:
      inline void print_cout(std::string sign, Virtuality is_virtual, std::string file, int line);
      void allocate();
  };

  bool operator< (const Fct& fct1, const Fct& fct2);
}
