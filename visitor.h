/****
     Copyright (C) 2015 Commissariat à l'Energie Atomique, Thales Communication
       - All Rights Reserved
     coded by Franck Vedrine, Hugues Balp
****/

//
// Description:
//   clang -> file containing called functions
//

#ifndef CLANG_VisitorH
#define CLANG_VisitorH

#include <boost/filesystem.hpp>
#include "libgen.h"
#include "CallersData.hpp"
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
   std::ofstream dOut;
   std::string dOutFname;
   std::ofstream jOut;
   std::string jOutFname;
   clang::CompilerInstance& ciCompilerInstance;
   bool _doesGenerateImplicitMethods;

  public:
   CallersAction(const std::string& out, 
		 const std::string& dout, 
		 const std::string& jout, 
		 clang::CompilerInstance& compilerInstance)
     : fOut(out),
     dOut(dout), dOutFname(dout), 
     jOut(jout), jOutFname(jout), 
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
  std::ostream& dotOut;
  std::string dotOutFname;
  std::ostream& jsonOut;
  std::string jsonOutFname;
  CallersData::File jsonFile;
  clang::CompilerInstance& ciCompilerInstance;
  const clang::FunctionDecl* pfdParent;
  mutable std::string sParent;
  clang::SourceManager* psSources;

  std::string writeFunction(const clang::FunctionDecl& function, bool isUnqualified=false) const;
  
  const std::string& printParentFunction() const
  {  
    if (pfdParent && sParent.length() == 0)
      sParent = writeFunction(*pfdParent);
    return sParent;
  };

  // get the basename of a file from its unix-like full path
  std::string getBasename(const clang::StringRef& filename) const;
  // convert function signature to a dot identifier
  std::string getDotIdentifier(const std::string& name) const;
  std::string getJsonIdentifier(const std::string& name) const;
  std::string printLocation(const clang::SourceRange& rangeLocation) const;
  std::string printTemplateExtension(const clang::TemplateArgumentList& arguments) const;
  std::string printTemplateKind(const clang::FunctionDecl& function) const;
  std::string printTemplateKind(const clang::RecordDecl& decl) const;
  std::string printBuiltinType(const clang::BuiltinType* type) const;
  std::string printArithmeticType(const clang::Type* type) const;
  std::string printPlainType(clang::QualType const& qt) const;
  std::string printType(clang::QualType const& qt) const;
  std::string printCompoundType(clang::Type const* type) const;
  std::string printArgumentSignature(const clang::FunctionDecl& function) const;
  std::string printResultSignature(const clang::FunctionDecl& function) const;
  std::string printQualification(const clang::DeclContext* context) const;
  std::string printQualifiedName(const clang::NamedDecl& namedDecl, bool* isEmpty=nullptr) const;

  void VisitInheritanceList(clang::CXXRecordDecl* cxxDecl);
  bool isTemplate(clang::CXXRecordDecl* RD) const;

 public:
 Visitor(const std::string& in,
	 const std::string& file,
	 const std::string& path,
	 std::ostream& sout, 
	 std::ostream& dout, std::string doutfname, 
	 std::ostream& jout, std::string joutfname,
	 clang::CompilerInstance& compilerInstance)
   : inputFile(in), osOut(sout), 
    dotOut(dout), dotOutFname(doutfname), 
    jsonOut(jout), jsonOutFname(joutfname), 
    jsonFile(file, path),
    ciCompilerInstance(compilerInstance), 
    pfdParent(nullptr), psSources(nullptr)
  {}

  ~Visitor()
    {
      jsonFile.sort_local_and_external_function_calls();
      jsonFile.output_json_desc();
    }

  virtual bool VisitCXXConstructExpr(const clang::CXXConstructExpr* constructor);
  virtual bool VisitCXXDeleteExpr(const clang::CXXDeleteExpr* deleteExpr);
  virtual bool VisitCXXNewExpr(const clang::CXXNewExpr* newExpr);
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
  virtual bool VisitRecordDecl(clang::RecordDecl* Decl);

  virtual bool shouldVisitTemplateInstantiations() const { return true; }
  virtual bool shouldVisitImplicitCode() const { return true; }
  virtual bool TraverseCXXRecordDecl(clang::CXXRecordDecl* Decl);
  virtual bool TraverseFriendDecl(clang::FriendDecl* Decl) { return true; }
  virtual bool TraverseFriendTemplateDecl(clang::FriendTemplateDecl* Decl) { return true; }
};

#endif // CLANG_VisitorH

