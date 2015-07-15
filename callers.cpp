/****
     Copyright (C) 2015 Commissariat à l'Energie Atomique, Thales Communication
       - All Rights Reserved
     coded by Franck Vedrine, Hugues Balp
****/

//
// Description:
//   clang -> file containing called functions
//

#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstddef>
#include <list>

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/Utils.h"
#include "clang/Basic/Version.h"

#include "llvm/Support/Path.h"
#include "llvm/Support/Host.h"
#include "llvm/ADT/ArrayRef.h"

#include "visitor.h"

class ProcessArguments {
  private:
   bool _isValid;
   // clang::CompilerInvocation* _invocation;
   std::string _output;
   std::string _dotfilename;
   std::vector<std::string> _includes;
   bool _doesGenerateImplicitMethods;

  public:
   ProcessArguments(/* clang::CompilerInvocation* invocation, */
         int argc, char** argv)
      : _isValid(true), /* _invocation(invocation), */ _doesGenerateImplicitMethods(false)
      { int uArg = argc-2;
         while (_isValid && (uArg >= 0))
            _isValid = process(argv + (argc - uArg - 1), uArg);
         // _isValid = _isValid
         //    && _invocation->getFrontendOpts().Inputs.size() > 0;
      }

   bool process(char** argument, int& currentArgument);
   void printUsage(std::ostream& osOut) const
      { std::cout << "Usage of callers :\n"
            << "callers [option]+ input-files -o callers-outfile \n"
            << "   where option can be one of the following options:\n";
         std::cout << "      --version or\n";
         std::cout << "      -Idirectory\tfor specifying an include directory\n";
         std::cout << "\nexample: callers -I. myfile.cpp -o myfile.fir\n";
         std::cout << std::endl;
      }
   bool isValid() const { return _isValid; }
   bool doesGenerateImplicitMethods() const { return _doesGenerateImplicitMethods; }
   const std::string& getOutputFile() const { return _output; }
   const std::string& getDotOutputFile() const { return _dotfilename; }
};

bool
ProcessArguments::process(char** argument, int& currentArgument) {
   if (argument[0][0] == '-') {
      switch (argument[0][1]) {
         case 'o':
	   {
	     char dotfilename[150];
	     dotfilename[0] = '\0';
	     if (currentArgument == 0 || _output != "") {
               printUsage(std::cout);
               return false;
	     };
	     currentArgument -= 2;
	     _output = argument[1];
	     strcat(dotfilename, argument[1]);
	     strcat(dotfilename, ".dot");
	     _dotfilename = dotfilename;
	     _isValid = true;
	     return true;
	   }
         case '-':
	   {
	     //TBC
	   }
         default:
            --currentArgument;
            return true;
      };
   }
   else
      --currentArgument;
   return true;
}

int
main(int argc, char** argv) {
   clang::CompilerInstance compiler;
#ifndef _WIN32
   compiler.createDiagnostics();
#else
   compiler.createDiagnostics(argc, argv);
#endif
   compiler.createFileManager();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

   ProcessArguments processArgument(argc, argv);
   if (argc == 1) {
      processArgument.printUsage(std::cout);
      return 0;
   };
   if (!processArgument.isValid()) {
      processArgument.printUsage(std::cout);
      return 0;
   };

   int clang_argc = argc;
   llvm::MutableArrayRef<const char*> Argv((const char**)argv, clang_argc+1);
   for (int i = 0; i<argc; i++)
      Argv[i] = argv[i];
#pragma GCC diagnostic pop
   Argv[clang_argc]="-Qunused-arguments";
   llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine>
      Diag(&compiler.getDiagnostics());
   clang::CompilerInvocation*
      invocation = clang::createInvocationFromCommandLine(Argv,Diag);

   if (!invocation) {
      std::cerr << 
         "Could not create clang invocation; Aborting";
      exit(2);
   }

   invocation->getLangOpts()->CPlusPlus = true;
   invocation->getLangOpts()->Bool = true;
   invocation->getLangOpts()->WChar = true;
   invocation->getLangOpts()->Exceptions = true;
   invocation->getLangOpts()->CXXExceptions = true;
   invocation->getLangOpts()->EmitAllDecls = true;
   invocation->getLangOpts()->GNUInline = true;
   invocation->getLangOpts()->Deprecated = true;
   invocation->getLangOpts()->ShortWChar = true;
   invocation->getLangOpts()->ImplicitInt = false;
   invocation->getFrontendOpts().ProgramAction = clang::frontend::ParseSyntaxOnly;
   compiler.setInvocation(invocation);

   CallersAction callersAction(processArgument.getOutputFile(), 
			       processArgument.getDotOutputFile(), 
			       compiler);
   if (processArgument.doesGenerateImplicitMethods())
      callersAction.setGenerateImplicitMethods();
   compiler.ExecuteAction(callersAction);
   return 0;
}

