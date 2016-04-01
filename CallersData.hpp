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

#define DEFAULT_ROOT_NAMESPACE "::"

typedef std::string MangledName;

namespace CallersData
{
  class JsonFileWriter
  {
    public:
      JsonFileWriter(std::string jsonLogicalFilePath, std::string jsonPhysicalFilePath);
      ~JsonFileWriter();
      std::string jsonLogicalFilePath = "unknownJsonLogicalFilePath";
      std::string jsonPhysicalFilePath = "unknownJsonPhysicalFilePath";
      std::ofstream out;
  };

  class File;

  class Dir
  {
    public:
      Dir();
      Dir(std::string dir, std::string path);
      ~Dir();
      std::string get_dirpath ();
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
      std::string jsonfilepath = "unknownJsonFilePath";
      std::set<File> files;
   };

  class Namespace;
  class Record;
  class FctDecl;
  class FctDef;
  class FctCall;
  class Thread;

  enum Virtuality { VNoVirtual, VVirtualDeclared, VVirtualDefined, VVirtualPure };
  enum FctKind { E_FctDecl, E_FctDef };
  enum FileKind { E_HeaderFile, E_SourceFile, E_UnknownFileKind };

  class File
  {
    friend bool operator< (const CallersData::File& file1, const CallersData::File& file2);
    public:
      File(std::string file, std::string filepath);
      File(const CallersData::File& copy_from_me);
      ~File();
      static FileKind getKind(std::string filename);
      std::string get_filepath () const;
      bool is_same_file(std::string otherFilePath, std::string otherFileName = CALLERS_NO_FILE_NAME) const;
      void assertSameFile(std::string otherFilePath, std::string otherFileName = CALLERS_NO_FILE_NAME) const;
      void parse_json_file(Dir *files) const;
      void add_namespace(const CallersData::Namespace& nspc) const;
      // void check_or_create_namespace(std::string nspc) const;
      std::set<CallersData::Namespace>::iterator get_or_create_namespace(std::string nspc) const;
      // std::set<CallersData::Namespace>::iterator get_or_create_namespace(std::string qualifiers, const clang::NamespaceDecl& nspc) const;
      std::set<CallersData::FctDecl>::const_iterator get_or_create_declared_function(FctDecl* fct, std::string filepath, Dir *context) const;
      std::set<CallersData::FctDecl>::const_iterator get_or_create_local_declared_function(FctDecl* fct, std::string filepath, Dir *context) const;
      std::set<CallersData::FctDecl>::const_iterator get_declared_function(std::string decl_sign, std::string decl_filepath) const;
      std::set<CallersData::FctDef>::const_iterator get_or_create_defined_function(FctDef* fct, std::string filepath, Dir *context) const;
      std::set<CallersData::FctDef>::const_iterator get_or_create_local_defined_function(FctDef* fct, std::string filepath, Dir *context) const;
      bool add_definition_to_declaration(std::string def_pos, std::string decl_sign, std::string decl_filepath) const;
      bool add_definition_to_declaration(std::string def_pos, std::string decl_sign, std::string decl_filepath, Dir* otherFiles) const;
      std::set<CallersData::Record>::iterator get_or_create_record(CallersData::Record *record, Dir* allJsonFiles) const;
      std::set<CallersData::Record>::iterator get_or_create_local_record(CallersData::Record *record) const;
      // std::set<CallersData::Record>::iterator get_record(std::string recordName, std::string recordFilePath, Dir* allJsonFiles) const;
      // std::set<CallersData::Record>::iterator get_local_record(std::string recordName, std::string recordFilePath) const;
      void try_to_add_redeclared_and_redeclaration_methods(Dir* allJsonFiles) const;
      void try_to_add_redeclared_and_redeclaration_methods(const FctDecl& fct_decl, std::string fct_filepath, Dir* allJsonFiles) const;
      // void add_redeclared_method(FctDecl* fct_decl, std::string fct_filepath, Dir* allJsonFiles) const;
      // void add_redeclaration(FctDecl* fct_decl, std::string fct_filepath, Dir* allJsonFiles) const;
      void add_thread(Thread* thread, Dir *files) const;
      void add_function_call(FctCall* fc, Dir *context) const;
      void output_json_desc() const;
      std::set<Namespace> *namespaces;
      std::set<Record>  *records;
      std::set<Thread>  *threads;
      std::set<FctDecl> *declared;
      std::set<FctDef>  *defined;
  private:
      void add_declared_function(CallersData::FctDecl* fct, std::string fct_filepath, Dir* files) const;
      //void add_declared_function(std::string sign, Virtuality virtuality, std::string file, int line) const;
      void add_defined_function(FctDef* fct, std::string filepath, Dir *otherFiles) const;
      // void add_defined_function(MangledName mangled, std::string sign, Virtuality virtuality, std::string nspc, std::string file,
      //                           int line, std::string filepath, std::string decl_file, int decl_line, std::string record, Dir *context) const;
      void add_record(Record *record) const;
      void add_record(std::string name, clang::TagTypeKind kind, std::string nspc, int begin, int end) const;
      std::set<FctCall> *calls;
      std::string filename = "unknownFileName";
      std::string kind = "unknownFileKind";
      std::string dirpath = "unknownFileDirPath";
      std::string filepath = CALLERS_NO_FILE_PATH;
      std::string jsonLogicalFilePath = "unknownJsonLogicalFilePath";
      std::string jsonPhysicalFilePath = "unknownJsonPhysicalFilePath";
  };

  /* Namespace class */
  class Namespace
  {
    friend std::ostream &operator<<(std::ostream &output, const Namespace &nspc);
    friend bool operator< (const CallersData::Namespace& nspc1, const CallersData::Namespace& nspc2);

    public:
      Namespace(std::string name);
      // Namespace(std::string qualifiers, const clang::NamespaceDecl& nspc);
      Namespace(const Namespace& copy_from_me);
      ~Namespace();
      void allocate();
      // void add_namespace(Namespace nspc) const;
      // void add_record(Record record) const;
      // void add_record(std::string name, clang::TagTypeKind kind, int loc) const;
      void add_record(std::string name) const;
      void add_namespace_calls(std::string caller_nspc) const;
      void add_namespace_called(std::string callee_nspc) const;
      std::string get_name() const;
      bool isSameNamespace(std::string identifier) const;
      bool get_namespaces(std::string identifier, std::string& root_namespace, std::string &namespaces) const;
      void output_json_desc(std::ofstream &js) const;
      void print_cout() const;
      // std::string get_qualifiers() const;
      // std::set<Namespace> *namespaces;
      // std::set<Record> *records;
    private:
      std::string name = "unknownNamespaceName"; // CALLERS_DEFAULT_NO_NAMESPACE_NAME
      // std::string qualifiers;
      std::set<std::string> *records;
      std::set<std::string> *calls;
      std::set<std::string> *called;
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

  class ExtFctDecl;

  /* Record class to store "class" or "struct" definitions */
  class Record
  {
    friend class File;
    friend std::ostream &operator<<(std::ostream &output, const Record &rec);
    friend bool operator< (const CallersData::Record& rec1, const CallersData::Record& rec2);

    public:
      // Record(const char* name, clang::TagTypeKind kind, const std::string file, const int begin, const int end);
      Record(std::string name, clang::TagTypeKind kind, std::string nspc, std::string file, int begin, int end);
      Record(std::string name, std::string nspc, std::string file);
      Record(const Record& copy_from_me);
      ~Record();
      void allocate();
      // The three methods below are currently useless because I didn't yet found how to check whether a CXXMethodDecl is declared public, private or friend
      // void add_public_method(std::string name) const;
      // void add_private_method(std::string name) const;
      // void add_friend_method(std::string name) const;
      // For the moment, all methods are considered the same.
      void add_method(std::string name) const;
      void add_member(std::string member, std::string type) const;
      void add_record_call(std::string otherRecordName) const;
      void add_record_called(std::string otherRecordName) const;
      void add_redeclared_method(std::string base_class, ExtFctDecl redecl_method) const;
      void add_redeclaration(std::string base_class, ExtFctDecl redeclaration) const;
      std::set<std::pair<std::string, CallersData::ExtFctDecl>>::const_iterator get_redeclared_method(std::string method_sign) const;
      std::set<std::pair<std::string, CallersData::ExtFctDecl>>::const_iterator get_redeclaration(std::string method_sign) const;
      void add_base_class(Inheritance inheritance) const;
      void add_child_class(Inheritance inheritance) const;
      void output_json_desc(std::ofstream &js) const;
      void print_cout() const;
      std::string name = "unknownRecordName";
      clang::TagTypeKind kind = clang::TTK_Class;
      std::string nspc;
      std::string file;
      int begin = -1;
      int end = -1;
      std::set<Inheritance> *inherited;
    protected:
      std::set<std::pair<std::string, ExtFctDecl>> *redeclared_methods;
      std::set<std::pair<std::string, ExtFctDecl>> *redeclarations;
    private:
      std::set<Inheritance> *inherits;
      std::set<std::string> *methods;
      std::set<std::pair<std::string, std::string>> *members;
      std::set<std::string> *calls;
      std::set<std::string> *called;
      // std::set<std::string> *public_methods;
      // std::set<std::string> *private_methods;
      // std::set<std::string> *friend_methods;
  };

  bool operator< (const Record& record1, const Record& record2);

  /* Thread class */
  class Thread
  {
    friend std::ostream &operator<<(std::ostream &output, const Thread &thr);
    friend bool operator< (const CallersData::Thread& thr1, const CallersData::Thread& thr2);
    public:
    Thread(std::string inst_name,
           std::string routine_name,
           std::string routine_sign,
           std::string routine_mangled,
           Virtuality  routine_virtuality,
           std::string routine_nspc,
           std::string routine_file,
           int routine_line,
           std::string routine_recordName,
           std::string routine_recordFilePath,
           std::string create_location,
           std::string caller_mangled,
           std::string caller_sign,
           Virtuality  caller_virtuality,
           std::string caller_nspc,
           std::string caller_file,
           int caller_line,
           std::string caller_decl_file,
           int caller_decl_line,
           std::string caller_recordName,
           std::string caller_recordFilePath
      );
      Thread(const Thread& copy_from_me);
      ~Thread();
      // void allocate();
      void output_json_desc(std::ofstream &js) const;
      void print_cout() const;
      std::string id = "unknownThreadId";
      std::string inst_name = "unknownThreadInstanceName";
      std::string routine_name = "unknownThreadRoutineName";
      std::string routine_sign = "unknownThreadRoutineSign";
      std::string routine_mangled = "unknownThreadRoutineMangled";
      Virtuality routine_virtuality = CallersData::VNoVirtual;
      std::string routine_nspc = CALLERS_DEFAULT_NO_NAMESPACE_NAME;
      std::string routine_file = "unknownThreadRoutineDeclFile";
      int routine_line = -1;
      // Virtuality routine_def_virtuality = CallersData::VNoVirtual;
      // std::string routine_def_file = "unknownThreadRoutineDefFile";
      // int routine_def_line = -1;
      std::string routine_recordName = "unknownThreadRoutineRecordName";
      std::string routine_recordFilePath = "unknownThreadRoutineRecordFilePath";
      std::string create_location = "unknownThreadCreateLocation";
      std::string caller_mangled = "unknownThreadCallerMangled";
      std::string caller_sign = "unknownThreadCallerSign";
      Virtuality caller_virtuality = CallersData::VNoVirtual;
      std::string caller_nspc = CALLERS_DEFAULT_NO_NAMESPACE_NAME;
      std::string caller_file = "unknownThreadCallerFile";
      int caller_line = -1;
      std::string caller_decl_file = CALLERS_NO_FCT_DECL_FILE;
      int caller_decl_line = -1;
      std::string caller_recordName = "unknownThreadCallerRecordName";
      std::string caller_recordFilePath = "unknownThreadCallerRecordFilePath";
    private:
      //
  };

  bool operator< (const Thread& thread1, const Thread& thread2);

  class ExtFct
  {
    public:
      ExtFct(MangledName mangled, std::string sign, std::string fct_loc);
      ExtFct(const ExtFct& copy_from_me);
      ~ExtFct() {}
    // protected:
      MangledName mangled = "unknownExtFctMangled";
      std::string sign = "unknownExtFctSign";
      std::string fctLoc = "unknownExtFctDeclLoc";
  };

  class ExtFctDecl : public ExtFct
  {
    friend std::ostream &operator<<(std::ostream &output, const ExtFctDecl &fct);
    public:
      ExtFctDecl(MangledName mangled, std::string sign, std::string fct_loc) : ExtFct(mangled, sign, fct_loc) { print_cout(mangled, sign, fctLoc); };
      ExtFctDecl(const ExtFct& copy_from_me) : ExtFct(copy_from_me) { print_cout(mangled, sign, fctLoc); };
      ~ExtFctDecl() {}
      void output_json_desc(std::ostream &js) const;
    private:
      void print_cout(MangledName mangled, std::string sign, std::string fct);
  };

  class ExtFctDef : public ExtFct
  {
    friend std::ostream &operator<<(std::ostream &output, const ExtFctDef &fct);
    public:
      ExtFctDef(MangledName mangled, std::string sign, std::string fct_loc) : ExtFct(mangled, sign, fct_loc) { print_cout(mangled, sign, fctLoc); };
      ExtFctDef(const ExtFct& copy_from_me) : ExtFct(copy_from_me) { print_cout(mangled, sign, fctLoc); };
      ~ExtFctDef() {}
    private:
      void print_cout(MangledName mangled, std::string sign, std::string fct);
  };

  std::ostream &operator<<(std::ostream &output, const ExtFct &fct);

  class Fct
  {
    public:
      Fct(std::string sign);
      Fct(MangledName mangled, std::string sign, Virtuality is_virtual, std::string nspc);
      Fct(const Fct& copy_from_me);
      ~Fct();
      MangledName mangled = "unknownFctMangledName";
      std::string sign = "unknownFctSign";
      Virtuality virtuality = VNoVirtual;
      std::string nspc = CALLERS_DEFAULT_NO_NAMESPACE_NAME;
  };

  class Parameter;

  class FctDecl : public Fct
  {
    friend class Visitor;
    friend class File;
    public:
      // FctDecl(const char* mangled, const char* sign, Virtuality is_virtual, const char* filepath, const int line, const char* record);
      FctDecl(std::string mangled, std::string sign, Virtuality is_virtual, std::string nspc, std::string filepath, int line, bool is_builtin = false);
      FctDecl(std::string mangled, std::string sign, Virtuality is_virtual, std::string nspc, std::string filepath, int line, std::string recordName, std::string recordFilePath, bool is_builtin = false);
      FctDecl(std::string sign, std::string filepath);
      FctDecl(const FctDecl& copy_from_me);
      ~FctDecl();

      void add_parameter(const CallersData::Parameter& parameter) const;
      void add_local_caller(std::string caller_sign) const;
      void add_external_caller(MangledName mangled, std::string sign, std::string file_pos) const;
      void add_external_caller(MangledName mangled, std::string sign, std::string file, int line) const;
      void add_redeclared_method(const ExtFctDecl& redeclared_method) const;
      void add_redeclaration(const ExtFctDecl& redeclaration) const;
      // void add_redeclaration(MangledName fct_mangled, std::string fct_sign, Virtuality redecl_virtuality, std::string redecl_file, int redecl_line, std::string redecl_record) const;
      void add_definition(std::string fct_sign, std::string def_file_pos) const;
      // void add_definition(MangledName fct_mangled, std::string fct_sign, std::string def_sign, Virtuality def_virtuality, std::string def_file_pos, std::string record) const;
      void add_redefinition(MangledName fct_mangled, std::string fct_sign, Virtuality redef_virtuality, std::string redef_file, int redef_line, std::string record) const;
      void add_thread(std::string thread) const;

      void output_parameters(std::ostream &js) const;
      void output_threads(std::ostream &js) const;
      void output_local_callers(std::ostream &js) const;
      void output_external_callers(std::ostream &js) const;
      void output_redeclared_methods(std::ostream &js) const;
      void output_redeclarations(std::ostream &js) const;
      void output_definitions(std::ostream &js) const;
      void output_redefinitions(std::ostream &js) const;
      void output_json_desc(std::ostream &js) const;

      std::string file = CALLERS_NO_FCT_DECL_FILE;
      int line = -1;
      std::set<Parameter> *parameters;
      std::set<std::string> *threads;
      std::set<ExtFctDecl> *redeclared;
      std::set<ExtFctDecl> *redeclarations;
      std::set<std::string> *definitions;
      // std::set<ExtFctDef> *definitions;
      std::set<ExtFctDef> *redefinitions;
      std::set<std::string> *locallers;
      std::set<ExtFctDef> *extcallers;
    protected:
      std::string recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
      std::string recordFilePath = CALLERS_DEFAULT_NO_RECORD_PATH;
      bool is_builtin = false;
    private:
      inline void print_cout() const;
      //inline void print_cout(std::string sign, Virtuality is_virtual, std::string file, int line, std::string record);
      void allocate();
  };

  bool operator< (const FctDecl& fct1, const FctDecl& fct2);

  class FctDef : public Fct
  {
    friend class File;
    public:
      // FctDef(const char* mangled, const char* sign, Virtuality is_virtual,
      //        const char* def_filepath, const int def_line,
      //        const char* decl_filepath, const int decl_line, const char* record = CALLERS_DEFAULT_NO_RECORD_NAME);
      FctDef(MangledName mangled, std::string sign, Virtuality is_virtual, std::string nspc,
             std::string def_filepath, int def_line,
             std::string decl_file, int decl_line, std::string record = CALLERS_DEFAULT_NO_RECORD_NAME);
      FctDef(std::string sign, std::string filepath);
      FctDef(const FctDef& copy_from_me);
      ~FctDef();

      void add_local_callee(std::string callee_sign) const;
      void add_external_callee(MangledName mangled, std::string sign, std::string file_pos) const;
      void add_external_callee(MangledName mangled, std::string sign, std::string file, int line) const;
      void add_builtin_callee(MangledName mangled, std::string sign, Virtuality builtin_virtuality, std::string builtin_decl_file, int builtin_decl_line) const;
      void add_thread(std::string thread) const;

      void output_local_callers(std::ofstream &js) const;
      void output_threads(std::ofstream &js) const;
      void output_local_callees(std::ofstream &js) const;
      void output_external_callers(std::ofstream &js) const;
      void output_external_callees(std::ofstream &js) const;
      void output_json_desc(std::ofstream &js) const;

      std::string def_file = CALLERS_NO_FCT_DEF_FILE;
      int def_line = -1;
      std::string decl_file = CALLERS_NO_FCT_DECL_FILE;
      int decl_line = -1;
      std::set<std::string> *threads;
      std::set<std::string> *locallees;
      std::set<ExtFctDecl>  *extcallees;
    protected:
      std::string record = CALLERS_DEFAULT_NO_RECORD_NAME;
    private:
      inline void print_cout(std::string sign, Virtuality is_virtual, std::string file, int line, std::string record);
      void allocate();
  };

  bool operator< (const FctDef& fct1, const FctDef& fct2);

  class FctCall
  {
    friend bool operator< (const CallersData::FctCall& fc1, const CallersData::FctCall& fc2);
    friend void File::add_function_call(FctCall* fc, Dir* context) const;
    public:
      FctCall(MangledName caller_mangled, std::string caller_sign, Virtuality caller_virtuality, std::string caller_nspc,
              std::string caller_file, int caller_line, std::string caller_decl_file, int caller_decl_line,
              MangledName callee_mangled, std::string callee_sign, Virtuality callee_virtuality, std::string callee_nspc,
              std::string callee_decl_file, int callee_decl_line,
              std::string caller_record, std::string callee_record);
      FctCall(FctDef caller_fct, FctDecl callee_fct);
      //FctCall(const FctCall& copy_from_me);
      ~FctCall() {}
    protected:
      FctDef caller;
      FctDecl callee;
    private:
      std::string id;
  };

  class Data
  {
    public:
      Data();
      Data(const Data& copy_from_me);
      ~Data();
  };

  class NamedData : public Data
  {
    public:
      NamedData(std::string name);
      NamedData(const NamedData& copy_from_me);
      ~NamedData();
      std::string name;
  };

  class Parameter : public NamedData
  {
    public:
      Parameter(std::string name, std::string type);
      Parameter(const Parameter& copy_from_me);
      ~Parameter();
      std::string type;
  };

}
