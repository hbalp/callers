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

class CallersAction : public clang::ASTFrontendAction {
  private:
   std::ofstream fOut;
   clang::CompilerInstance& ciCompilerInstance;
   bool _doesGenerateImplicitMethods;

  public:
   CallersAction(const std::string& out, clang::CompilerInstance& compilerInstance)
      :  fOut(out), ciCompilerInstance(compilerInstance), _doesGenerateImplicitMethods(false) {}
   virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance& compilerInstance,
      clang::StringRef inputFile);
   void setGenerateImplicitMethods() { _doesGenerateImplicitMethods = true; }

  private:
   class Visitor;
};

class CallersAction::Visitor : public clang::ASTConsumer, public clang::RecursiveASTVisitor<Visitor> {
  private:
   typedef clang::RecursiveASTVisitor<Visitor> Parent;
   std::ostream& osOut;
   clang::CompilerInstance& ciCompilerInstance;
   const clang::FunctionDecl* pfdParent;
   mutable std::string sParent;

   std::string writeFunction(const clang::FunctionDecl& function) const;
   const std::string& printParentFunction() const
      {  if (pfdParent && sParent.length() == 0)
            sParent = writeFunction(*pfdParent);
         return sParent;
      };
      
   std::string printTemplateExtension(const clang::TemplateArgumentList& arguments) const;
   std::string printTemplateKind(const clang::FunctionDecl& function) const;
   std::string printTemplateKind(const clang::RecordDecl& decl) const;
   std::string printBuiltinType(const clang::Type* type) const;
   std::string printArithmeticType(const clang::Type* type) const;
   std::string printPlainType(clang::QualType const& qt) const;
   std::string printType(clang::QualType const& qt) const;
   std::string printCompoundType(clang::Type const* type) const;
   std::string printArgumentSignature(const clang::FunctionDecl& function) const;
   std::string printResultSignature(const clang::FunctionDecl& function) const;
   std::string printQualification(const clang::DeclContext* context) const;
   std::string printQualifiedName(const clang::NamedDecl& namedDecl) const;

  public:
   Visitor(std::ostream& out, clang::CompilerInstance& compilerInstance)
      : osOut(out), ciCompilerInstance(compilerInstance), pfdParent(nullptr) {}

   virtual bool VisitCXXConstructExpr(const clang::CXXConstructExpr* constructor);
   virtual bool VisitCXXDeleteExpr(const clang::CXXDeleteExpr* deleteExpr);
   virtual bool VisitCXXNewExpr(const clang::CXXNewExpr* newExpr);
   virtual bool VisitCallExpr(const clang::CallExpr* callExpr);
   virtual bool VisitMemberCallExpr(const clang::CXXMemberCallExpr* callExpr);

   virtual void HandleTranslationUnit(clang::ASTContext &context);
   virtual bool TraverseFunctionDecl(clang::FunctionDecl* Decl);
   virtual bool VisitFunctionDecl(clang::FunctionDecl* Decl);
   virtual bool shouldVisitTemplateInstantiations() const { return true; }
   virtual bool shouldVisitImplicitCode() const { return true; }
   virtual bool TraverseRecordDecl(clang::RecordDecl* Decl);
   virtual bool TraverseFriendDecl(clang::FriendDecl* Decl) { return true; }
   virtual bool TraverseFriendTemplateDecl(clang::FriendTemplateDecl* Decl) { return true; }
};

#endif // CLANG_VisitorH

