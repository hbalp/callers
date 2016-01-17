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

typedef std::string MangledName;

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
      void add_file(File file);
      std::set<File>::iterator create_or_get_file(std::string filename, std::string dirpath);
      void output_json_files();
      void output_json_dir();
    private:
      std::string dir = "unknownDirName";
      std::string path = "unknownDirPath";
      std::list<std::string> filenames;
      std::list<std::string> childrens;
      std::string jsonfilename = "unknownJsonFileName";
      std::set<File> files;
   };

  class Namespace;
  class Record;
  class FctDecl;
  class FctDef;
  class FctCall;

  enum Virtuality { VNoVirtual, VVirtualDeclared, VVirtualDefined, VVirtualPure };
  enum FctKind { E_FctDecl, E_FctDef };

  class File
  {
    friend bool operator< (const CallersData::File& file1, const CallersData::File& file2);
    public:
      File(std::string file, std::string path);
      File(const CallersData::File& copy_from_me);
      ~File();
      std::string fullPath () const;
      void parse_json_file(CallersData::Dir *files) const;
      std::set<CallersData::Namespace>::iterator create_or_get_namespace(std::string qualifiers, const clang::NamespaceDecl* nspc);
      //void add_declared_function(std::string sign, Virtuality virtuality, std::string file, int line) const;
      void add_declared_function(FctDecl* fct, std::string filepath, Dir *context) const;
      void add_defined_function(MangledName mangled, std::string sign, Virtuality virtuality, std::string file, int line, std::string filepath) const;
      void add_defined_function(FctDef* fct, std::string filepath) const;
      void add_namespace(Namespace nspc) const;
      void add_record(Record record) const;
      void add_record(std::string name, clang::TagTypeKind kind, int begin, int end) const;
      void add_function_call(FctCall* fc, Dir *context) const;
      void output_json_desc() const;
      std::set<Namespace> *namespaces;
      std::set<Record>  *records;
      std::set<FctDecl> *declared;
      std::set<FctDef>  *defined;
  private:
      std::set<FctCall> *calls;
      std::string file = "unknownFileName";
      std::string path = "unknownFilePath";
      std::string fullpath = "unknownFilePath";
      std::string jsonfilename = "unknownJsonFileName";
  };

  /* Namespace class */
  class Namespace
  {
    friend std::ostream &operator<<(std::ostream &output, const Namespace &nspc);
    friend bool operator< (const CallersData::Namespace& nspc1, const CallersData::Namespace& nspc2);

    public:
      Namespace(std::string qualifier);
      Namespace(std::string qualifier, const clang::NamespaceDecl& nspc);
      Namespace(const Namespace& copy_from_me);
      ~Namespace();
      void allocate();
      void add_namespace(Namespace nspc) const;
      //void add_namespace(std::string qualifier, const clang::NamespaceDecl& namespc) const;
      void add_record(Record record) const;
      void add_record(std::string name, clang::TagTypeKind kind, int loc) const;
      void output_json_desc(std::ofstream &js) const;
      void print_cout() const;
      std::string get_qualifier() const;
      std::string name = "unknownNamespaceName";
      // std::set<Namespace> *namespaces;
      // std::set<Record> *records;
      std::string qualifier;
    private:
  };

  bool operator< (const Namespace& nspc1, const Namespace& nspc2);

  /* Inheritance class to reference base classes*/
  class Inheritance
  {
    friend bool operator< (const CallersData::Inheritance& inheritance1, const CallersData::Inheritance& inheritance2);

  public:
    Inheritance(const char* name, const char* file, const int begin, const int end);
    Inheritance(std::string name, std::string file, int begin, int end);
    Inheritance(const CallersData::Inheritance& copy_from_me);
    ~Inheritance() {}
    void allocate();
    void output_json_desc(std::ofstream &js) const;
    inline void print_cout() const;
    std::string name = "unknownClassName";
    std::string file = "unknownClassFile";
    int begin = -1;
    int end = -1;
  private:
  };

  /* Record class to store "class" or "struct" definitions */
  class Record
  {
    friend std::ostream &operator<<(std::ostream &output, const Record &rec);
    friend bool operator< (const CallersData::Record& rec1, const CallersData::Record& rec2);

    public:
      Record(const char* name, clang::TagTypeKind kind, const std::string file, const int begin, const int end);
      Record(std::string name, clang::TagTypeKind kind, std::string file, int begin, int end);
      Record(const Record& copy_from_me);
      ~Record();
      void allocate();
      void add_base_class(std::string name, std::string file, int deb, int fin) const;
      void add_base_class(Inheritance inheritance) const;
      // void add_child_class(std::string name, std::string file, int deb, int fin) const;
      // void add_child_class(Inheritance inheritance) const;
      void output_json_desc(std::ofstream &js) const;
      void print_cout() const;
      std::string name = "unknownRecordName";
      clang::TagTypeKind kind = clang::TTK_Struct;
      std::string file;
      int begin = -1;
      int end = -1;
      std::set<Inheritance> *inherits;
      std::set<Inheritance> *inherited;
    private:
  };

  bool operator< (const Record& record1, const Record& record2);

  /* Used to store function call before knowing if it's a local or an external call */
  class FctCall
  {
    friend bool operator< (const CallersData::FctCall& fc1, const CallersData::FctCall& fc2);
    friend void File::add_function_call(FctCall* fc, Dir* context) const;

    public:
      FctCall(MangledName caller_mangled, std::string caller_sign, Virtuality caller_virtuality, std::string caller_file,
            int caller_line, MangledName callee_mangled, std::string callee_sign, Virtuality callee_virtuality,
	std::string callee_decl_file, int callee_decl_line);
      //FctCall(const FctCall& copy_from_me);
      ~FctCall() {}
      bool is_builtin = false;
    protected:
      std::string caller_mangled = "unknownCallerMangled";
      std::string caller_sign = "unknownCallerSign";
      Virtuality caller_virtuality = VNoVirtual;
      std::string caller_file = "unknownCallerFile";
      int caller_line = -1;
      std::string callee_mangled = "unknownCalleeMangled";
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
      ExtFct(MangledName mangled, std::string sign, Virtuality is_virtual, std::string fct_loc, FctKind kind);
      ExtFct(const ExtFct& copy_from_me);
      ~ExtFct() {}
      //void set_file(std::string file);
    private:
      inline void print_cout(MangledName mangled, std::string sign, Virtuality is_virtual, std::string fct, FctKind kind);
      MangledName mangled = "unknownExtFctMangled";
      std::string sign = "unknownExtFctSign";
      Virtuality virtuality = VNoVirtual;
      std::string fct = "unknownExtFctDeclLoc";
      FctKind kind = E_FctDecl;
  };

  std::ostream &operator<<(std::ostream &output, const ExtFct &fct);

  class FctDecl
  {
    public:
      FctDecl(const char* mangled, const char* sign, Virtuality is_virtual, const char* filepath, const int line);
      FctDecl(std::string mangled, std::string sign, Virtuality is_virtual, std::string filepath, int line);
      FctDecl(const FctDecl& copy_from_me);
      ~FctDecl();

      void add_local_caller(MangledName caller_mangled, std::string caller_sign) const;
      void add_external_caller(MangledName mangled, std::string sign, Virtuality virtuality, std::string file, int line) const;
      void add_redeclaration(MangledName fct_mangled, std::string fct_sign, Virtuality redecl_virtuality, std::string redecl_file, int redecl_line) const;
      void add_definition(MangledName fct_mangled, std::string fct_sign, std::string def_sign, Virtuality def_virtuality, std::string def_file_pos) const;
      void add_redefinition(MangledName fct_mangled, std::string fct_sign, Virtuality redef_virtuality, std::string redef_file, int redef_line) const;

      void output_local_callers(std::ofstream &js) const;
      void output_external_callers(std::ofstream &js) const;
      void output_redeclarations(std::ofstream &js) const;
      void output_definitions(std::ofstream &js) const;
      void output_redefinitions(std::ofstream &js) const;
      void output_json_desc(std::ofstream &js) const;

      MangledName mangled = "unknownFctDeclMangledName";
      std::string sign = "unknownFctDeclSign";
      std::string file = "unknownFctDeclFile";
      Virtuality virtuality = VNoVirtual;
      int line = -1;
      std::set<ExtFct> *redeclarations;
      std::set<ExtFct> *definitions;
      std::set<ExtFct> *redefinitions;
      std::set<std::string> *locallers;
      std::set<ExtFct> *extcallers;

    private:
      inline void print_cout(std::string sign, Virtuality is_virtual, std::string file, int line);
      void allocate();
  };

  bool operator< (const FctDecl& fct1, const FctDecl& fct2);

  class FctDef
  {
    public:
      FctDef(const char* mangled, const char* sign, Virtuality is_virtual, const char* filepath, const int line);
      FctDef(MangledName mangled, std::string sign, Virtuality is_virtual, std::string filepath, int line);
      FctDef(const FctDef& copy_from_me);
      ~FctDef();

      //void add_local_caller(std::string caller) const;
      void add_local_callee(MangledName callee_mangled, std::string callee_sign) const;
      //void add_external_caller(MangledName fct_mangled, std::string sign, Virtuality virtuality, std::string file, int line) const;
      void add_external_callee(MangledName mangled, std::string sign, Virtuality virtuality, std::string file, int line) const;
      void add_builtin_callee(MangledName mangled, std::string sign, Virtuality builtin_virtuality, std::string builtin_decl_file, int builtin_decl_line) const;

      void output_local_callers(std::ofstream &js) const;
      void output_local_callees(std::ofstream &js) const;
      void output_external_callers(std::ofstream &js) const;
      void output_external_callees(std::ofstream &js) const;
      void output_json_desc(std::ofstream &js) const;

      MangledName mangled = "unknownFctDefMangledName";
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

  bool operator< (const FctDef& fct1, const FctDef& fct2);
}
