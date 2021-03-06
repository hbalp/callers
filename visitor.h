/****
     Copyright (C) 2015
       - All Rights Reserved
     coded by Franck Vedrine and Hugues Balp
****/

//
// Description:
//   clang -> file containing called functions
//

#ifndef CLANG_VisitorH
#define CLANG_VisitorH

#include "libgen.h"
#include "CallersConfig.hpp"
#include "CallersData.hpp"
#include "clang/AST/GlobalDecl.h"
#include "clang/Basic/Version.h"

#if (CLANG_VERSION_MAJOR > 3)                                 \
    || (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR >= 4)
#define CLANG_VERSION_GREATER_OR_EQUAL_3_4
#endif

#if (CLANG_VERSION_MAJOR > 3)                                 \
    || (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR >= 5)
#define CLANG_VERSION_GREATER_OR_EQUAL_3_5
#endif

#if (CLANG_VERSION_MAJOR > 3)                                 \
    || (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR >= 7)
#define CLANG_VERSION_GREATER_OR_EQUAL_3_7
#endif

class CallersAction : public clang::ASTFrontendAction {
  private:
   std::ofstream fOut;
   clang::CompilerInstance& ciCompilerInstance;
   bool _doesGenerateImplicitMethods;

  public:
   CallersAction(const std::string& out, 
		 clang::CompilerInstance& compilerInstance)
     : fOut(out),
     ciCompilerInstance(compilerInstance), _doesGenerateImplicitMethods(false) {}
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_7
   virtual std::unique_ptr<clang::ASTConsumer> 
#else
   virtual clang::ASTConsumer*
#endif
     CreateASTConsumer(clang::CompilerInstance& compilerInstance,
		       clang::StringRef inputFile);
   void setGenerateImplicitMethods() { _doesGenerateImplicitMethods = true; }
 private:
   class Visitor;
};

class CallersAction::Visitor : public clang::ASTConsumer, public clang::RecursiveASTVisitor<Visitor> {

 private:
  typedef clang::RecursiveASTVisitor<Visitor> Parent;
  std::string inputFile;
  std::ostream& osOut;
  std::set<CallersData::File>::iterator currentJsonFile;
  CallersData::Dir otherJsonFiles;
  clang::CompilerInstance& ciCompilerInstance;
  const clang::FunctionDecl* pfdParent;
  mutable std::string sParent;
  clang::DiagnosticsEngine& diag_eng_;
  clang::MangleContext* mangle_context_;
  clang::SourceManager* psSources;

  std::string writeFunction(const clang::FunctionDecl& function, bool isUnqualified=false) const;
  void getMangledName(clang::MangleContext* ctx,
                      const clang::FunctionDecl* decl,
                      MangledName* result);

  int getStartLine(const clang::SourceRange& rangeLocation) const;
  int getEndLine(const clang::SourceRange& rangeLocation) const;
  int getNbLines(const clang::SourceRange& rangeLocation) const;

  // get the basename of a file from its unix-like full path
  std::string getBasename(const clang::StringRef& filename) const;
  // convert function signature to a json compatible identifier
  std::string getJsonIdentifier(const std::string& name) const;
  std::string printLocation(const clang::SourceRange& rangeLocation, std::string defaultFilePath = CALLERS_NO_FILE_PATH) const;
  std::string printNumber(int number) const;
  std::string printFileName(const clang::SourceRange& rangeLocation) const;
  std::string printFilePath(const clang::SourceRange& rangeLocation, std::string defaultFilePath = CALLERS_NO_FILE_PATH) const;
  std::string printCurrentPath(const clang::SourceRange& rangeLocation) const;
  std::string printTemplateExtension(const clang::TemplateArgumentList& arguments) const;
  std::string printTemplateKind(const clang::FunctionDecl& function) const;
  std::string printTemplateKind(const clang::RecordDecl& decl) const;
  std::string printBuiltinType(const clang::BuiltinType* type) const;
  std::string printArithmeticType(const clang::Type* type) const;
  std::string printPlainType(clang::QualType const& qt) const;
  std::string printType(clang::QualType const& qt) const;
  std::string printCompoundType(clang::Type const* type) const;
  std::string printArgumentSignature(const clang::FunctionDecl& function) const;
  std::string printRecordName(const clang::CXXRecordDecl* record) const;
  std::string printResultSignature(const clang::FunctionDecl& function) const;
  std::string printQualification(const clang::DeclContext* context) const;
  std::string printNamespaces(const clang::NamedDecl& context) const;
  std::string printRootQualification(const clang::DeclContext* context) const;
  std::string printRootNamespace(const clang::NamedDecl& context, std::string defaultRootNamespace, std::string recordName) const;
  // void parseNamespaces(const clang::DeclContext* context, std::set<CallersData::File>::iterator *file) const;
  std::string parseQualification(const clang::DeclContext* context, std::set<CallersData::File>::iterator *file);
  std::string parseQualifiedName(const clang::NamedDecl& namedDecl, bool* isEmpty=nullptr);
  std::string printQualifiedName(const clang::NamedDecl& namedDecl, bool* isEmpty=nullptr) const;

  const std::string& printParentFunction() const
  {
    if (pfdParent && sParent.length() == 0)
      sParent = writeFunction(*pfdParent);
    return sParent;
  };

  std::string printParentFunctionFileName() const
  {
    return printFileName(pfdParent->getSourceRange());
  };

  std::string printParentFunctionFilePath() const
  {
    return printFilePath(pfdParent->getSourceRange());
  };

  std::string printCurrentPath() const;

  inline int getParentFunctionStartLine() const
  {
    if (pfdParent)
      return getStartLine(pfdParent->getSourceRange());
    else
      return 0;
  };

  inline std::string printParentFunctionStartLine() const
  {
    return printNumber(this->getParentFunctionStartLine());
  };

  void VisitInheritanceList(clang::CXXRecordDecl* cxxDecl, std::set<CallersData::Record>::iterator& record);
  bool isTemplate(clang::CXXRecordDecl* RD) const;

 public:
 Visitor(const std::string& in,
	 const std::string& file,
	 const std::string& path,
	 std::ostream& sout,
	 clang::CompilerInstance& compilerInstance,
         clang::DiagnosticsEngine& diag_eng)
   : inputFile(in), osOut(sout),
     otherJsonFiles("callers", CALLERS_ROOTDIR_PREFIX),
     ciCompilerInstance(compilerInstance),
     pfdParent(nullptr),
     diag_eng_(diag_eng),
     psSources(nullptr)
  {
    currentJsonFile = otherJsonFiles.create_or_get_file(file, path);
  }

  ~Visitor()
    {
      CallersData::DirMetrics metrics;
      otherJsonFiles.update_metrics(metrics);
      otherJsonFiles.output_json_files();
      otherJsonFiles.output_metrics(metrics, osOut);
    }

  virtual bool VisitCXXConstructExpr(const clang::CXXConstructExpr* constructor);
  virtual bool VisitCXXDeleteExpr(const clang::CXXDeleteExpr* deleteExpr);
  virtual bool VisitCXXNewExpr(const clang::CXXNewExpr* newExpr);
  virtual bool VisitBuiltinFunction(const clang::FunctionDecl* fd);
  virtual bool VisitCallExpr(const clang::CallExpr* callExpr);
  virtual bool VisitMemberCallExpr(const clang::CXXMemberCallExpr* callExpr);

  virtual void HandleTranslationUnit(clang::ASTContext &context);
  virtual bool TraverseVarDecl(clang::VarDecl* Decl);
  virtual bool TraverseParmVarDecl(clang::VarDecl* Decl) { return true; }
  virtual bool TraverseFunctionDecl(clang::FunctionDecl* Decl);
  virtual bool TraverseCXXMethodDecl(clang::CXXMethodDecl* Decl);
  virtual bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl* Decl);
  virtual bool TraverseCXXConversionDecl(clang::CXXConversionDecl* Decl);
  virtual bool TraverseCXXDestructorDecl(clang::CXXDestructorDecl* Decl);
  virtual bool VisitFunctionDecl(clang::FunctionDecl* Decl);
  virtual bool VisitFunctionDeclaration(clang::FunctionDecl* Decl);
  virtual bool isDeclarationOfInterest(const clang::NamedDecl& Decl);
  virtual bool VisitFunctionDefinition(clang::FunctionDecl* Decl);
  virtual bool VisitFunctionParameters(const clang::FunctionDecl& function, const CallersData::FctDecl& fct_decl);
  virtual bool VisitMethodDeclaration(clang::CXXMethodDecl* Decl);
  virtual bool VisitRecordDecl(clang::RecordDecl* Decl);

  virtual bool shouldVisitTemplateInstantiations() const { return true; }
  virtual bool shouldVisitImplicitCode() const { return true; }
  virtual bool TraverseCXXRecordDecl(clang::CXXRecordDecl* Decl);
  virtual bool TraverseFriendDecl(clang::FriendDecl* Decl) { return true; }
  virtual bool TraverseFriendTemplateDecl(clang::FriendTemplateDecl* Decl) { return true; }
};

#endif // CLANG_VisitorH

