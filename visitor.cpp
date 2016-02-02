/****
     Copyright (C) 2015 Commissariat à l'Energie Atomique, Thales Communication
       - All Rights Reserved
     coded by Hugues Balp
     initiated by Franck Vedrine
****/

//
// Description:
//   clang -> file containing called functions
//

//#define NOT_USE_BOOST_FILESYSTEM
//#define NOT_USE_BOOST_REGEX

//#include <stdlib.h> // abort()

#include <climits>
#ifndef NOT_USE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
#endif
#ifndef NOT_USE_BOOST_REGEX
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#endif
#ifndef NOT_USE_BOOST_STRING
#include <boost/algorithm/string.hpp>
#endif
#include <cstddef>
#include <fstream>
#include <iostream>
#include <list>
//#include <regex>
#include <sstream>

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTDiagnostic.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Mangle.h"
#include "clang/Basic/Version.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/Builtins.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/Utils.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/Preprocessor.h"

#include "llvm/Support/Path.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/ArrayRef.h"

#include "visitor.h"

#if (CLANG_VERSION_MAJOR > 3)                                 \
    || (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR >= 5)
#define CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
#endif

// #ifndef NOT_USE_BOOST_REGEX
// #ifndef NOT_USE_BOOST
// void boost::throw_exception(std::exception const&)
// {
//   std::cerr << "HBDBG EXCEPTION TBC" << std::endl;
// }
// #endif
// #endif

std::string
getCanonicalAbsolutePath(const std::string& path)
{
   std::string absolutePath = clang::tooling::getAbsolutePath(path);
   //std::string absolutePath = llvm::sys::fs::make_absolute(path);
   boost::filesystem::path p(absolutePath);
   std::string canonicalPath = boost::filesystem::canonical(p).string();
   return canonicalPath;
}

#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_7
std::unique_ptr<clang::ASTConsumer>
#else
clang::ASTConsumer*
#endif
CallersAction::CreateASTConsumer(clang::CompilerInstance& compilerInstance,
				 clang::StringRef inputFile)
{
  std::cout << "file: " << inputFile.str() << std::endl;
  boost::filesystem::path p(inputFile);
  std::string basename = p.filename().string();
  std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());

  if(dirpath == "")
    {
      boost::filesystem::path current_path = boost::filesystem::current_path();
      std::string working_directory = boost::filesystem::canonical(current_path).string();
      std::cout << "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" << std::endl;
      std::cout << "Input JSON file path is empty. So we will use the current working directory: " << current_path << std::endl;
      std::cout << "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" << std::endl;
      dirpath = working_directory;
    }

  std::cout << "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII" << std::endl;
  std::cout << "current working directory: " << dirpath << std::endl;
  std::cout << "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII" << std::endl;

     /*{
      std::cerr << "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" << std::endl;
      std::cerr << "Input JSON file path is empty. This is probably a Usage Error..." << std::endl;
      std::cerr << "file: " << file << std::endl;
      std::cerr << "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" << std::endl;
      abort();
      }*/

  if(compilerInstance.hasDiagnostics())
  {
    clang::DiagnosticsEngine& diag_eng = compilerInstance.getDiagnostics();

#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_7
  return llvm::make_unique<Visitor>(inputFile.str(), basename, dirpath, fOut, compilerInstance, diag_eng);
#else
  return new Visitor(inputFile.str(), basename, dirpath, fOut, compilerInstance, diag_eng);
#endif
  }
  else
  {
    std::cerr << "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" << std::endl;
    std::cerr << "visitor.cpp:ERROR: No diagnostic engine" << std::endl;
    std::cerr << "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" << std::endl;
    return NULL;
  }
}

// std::unique_ptr<ASTConsumer> RenamingAction::newASTConsumer() {
//   return llvm::make_unique<RenamingASTConsumer>(NewName, PrevName, USRs,
//                                                 Replaces, PrintLocations);
// }

void
CallersAction::Visitor::HandleTranslationUnit(clang::ASTContext &context) {
   int numberOfErrors = ciCompilerInstance.getDiagnostics().getClient()->getNumErrors();
   if (numberOfErrors > 0) {
      if (numberOfErrors == 1)
         std::cerr << "callers generation aborted due to one compilation error";
      else
         std::cerr << "callers generation aborted due to compilation errors";
      std::cerr << std::endl;
      exit(2);
   };
   psSources = &context.getSourceManager();
   //diag_eng_ = &context.getDiagnostics();
   mangle_context_ = clang::ItaniumMangleContext::create(context, diag_eng_);

   TraverseDecl(context.getTranslationUnitDecl());
}

std::string
CallersAction::Visitor::printLocation(const clang::SourceRange& rangeLocation) const {
   assert(psSources);
   auto start = psSources->getPresumedLoc(rangeLocation.getBegin());
   int startLine = 0;
   std::string startFile = this->printFilePath(rangeLocation);
   if (startFile != "<unknown>")
     {
      startLine = start.getLine();
      startFile += ':';
      std::ostringstream out;
      out << startLine;
      startFile += out.str();
     }
   return startFile;
}

std::string
CallersAction::Visitor::printFileName(const clang::SourceRange& rangeLocation) const {
   assert(psSources);
   auto start = psSources->getPresumedLoc(rangeLocation.getBegin());
   const char* startFile = start.getFilename();
   if (!startFile)
     startFile = "<unknown>";
   std::string result = startFile;
   return result;
}

std::string
CallersAction::Visitor::printFilePath(const clang::SourceRange& rangeLocation) const {
   assert(psSources);
   //bool isValid = rangeLocation.isValid();
   auto start = psSources->getPresumedLoc(rangeLocation.getBegin());
   const char* startFile = start.getFilename();
   std::string path = "unknownFilePath";
   if (startFile)
     path = ::getCanonicalAbsolutePath(startFile);
   return path;
}

std::string CallersAction::Visitor::printCurrentPath() const
{  
  boost::filesystem::path current_path = boost::filesystem::current_path();
  std::string path = boost::filesystem::canonical(current_path).string();
  return path;
}

int
CallersAction::Visitor::printLine(const clang::SourceRange& rangeLocation) const {
   assert(psSources);
   auto start = psSources->getPresumedLoc(rangeLocation.getBegin());
   return start.getLine();
}

std::string
CallersAction::Visitor::printTemplateExtension(const clang::TemplateArgumentList& arguments) const {
   std::string result = "<";
   bool isFirst = true;
   int sizeArguments = arguments.size();
   for (int index = 0; index < sizeArguments; ++index) {
      const auto& templateArgument = arguments[index];
      if (isFirst)
         isFirst = false;
      else
         result += ",";
      switch (templateArgument.getKind()) {
         case clang::TemplateArgument::Null:
            // an empty template argument, e.g., one that has not been deduced
            break;
         case clang::TemplateArgument::Type: // argument is a type.
            result += printType(templateArgument.getAsType());
            break;
         case clang::TemplateArgument::Declaration:
            result += printQualifiedName(*templateArgument.getAsDecl());
            break;
         case clang::TemplateArgument::NullPtr:
            result += "0";
            break;
         case clang::TemplateArgument::Integral:
            {  std::ostringstream temp;
               temp << templateArgument.getAsIntegral().getLimitedValue(UINT64_MAX);
               result += temp.str();
            };
            break;
         case clang::TemplateArgument::Template:
            // templateArgument is a template name that was provided for a 
            // template template parameter.
            break;
         case clang::TemplateArgument::TemplateExpansion:
            // templateArgument is a pack expansion of a template name that was 
            // provided for a template template parameter.
            break;
         case clang::TemplateArgument::Expression:
            // templateArgument is a value- or type-dependent expression
            // stored in an Expr*.
           break;
         case clang::TemplateArgument::Pack:
            // templateArgument is actually a parameter pack. Arguments are stored
            // in the Args struct.
           break;
         default:
            result += "0";
            break;
      };
   };
   result += ">";
   return result;
}

std::string
CallersAction::Visitor::printTemplateKind(const clang::FunctionDecl& function) const {
   if (function.getTemplateSpecializationKind() >= clang::TSK_ImplicitInstantiation) {
      if (auto* info = function.getTemplateSpecializationInfo())
         return printTemplateExtension(*info->TemplateArguments);
   };
   return "";
}

std::string
CallersAction::Visitor::printTemplateKind(const clang::RecordDecl& decl) const {
   assert(llvm::dyn_cast<clang::CXXRecordDecl>(&decl));
   const clang::CXXRecordDecl *RD = llvm::dyn_cast<clang::CXXRecordDecl>(&decl);
   const clang::ClassTemplateSpecializationDecl* TSD = NULL;
   if (decl.getKind() == clang::Decl::ClassTemplateSpecialization) // do not handle partial specialization !
      TSD = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(&decl);
   if (RD && TSD /* RD->getTemplateSpecializationKind() >= clang::TSK_ImplicitInstantiation */) {
      assert(llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(RD));
      return printTemplateExtension(static_cast<const clang
                  ::ClassTemplateSpecializationDecl*>(RD)->getTemplateArgs());
   }
   return "";
}

std::string
CallersAction::Visitor::printBuiltinType(const clang::BuiltinType* type) const {
   switch(type->getKind()) {
      case clang::BuiltinType::Void: return "void";
      case clang::BuiltinType::Bool: return "bool";
      case clang::BuiltinType::Char_U: return "char";
      case clang::BuiltinType::Char_S: return "char";
      case clang::BuiltinType::WChar_U: return "wchar_t";
      case clang::BuiltinType::WChar_S: return "wchar_t";
      case clang::BuiltinType::UChar: return "unsigned char";
      case clang::BuiltinType::SChar: return "signed char";
      case clang::BuiltinType::Short: return "short";
      case clang::BuiltinType::UShort: return "unsigned short";
      case clang::BuiltinType::Int: return "int";
      case clang::BuiltinType::UInt: return "unsigned int";
      case clang::BuiltinType::Long: return "long";
      case clang::BuiltinType::ULong: return "unsigned long";
      case clang::BuiltinType::LongLong: return "long long";
      case clang::BuiltinType::ULongLong: return "unsigned long long";
      case clang::BuiltinType::Float: return "float";
      case clang::BuiltinType::Double: return "double";
      case clang::BuiltinType::LongDouble: return "long double";
      case clang::BuiltinType::Int128: return "int_128_t";
      case clang::BuiltinType::UInt128: return "uint_128_t";
      default:
         return "";
         // std::cerr << "Unsupported Builtin-type: " 
         //       << typ->getName(_context->getPrintingPolicy()).str()
         //       << "\nAborting\n";
         // exit(2);
   }
}

std::string
CallersAction::Visitor::printArithmeticType(const clang::Type* type) const {
   switch(type->getTypeClass()) {
      case clang::Type::Builtin:
         assert(llvm::dyn_cast<const clang::BuiltinType>(type));
         return printBuiltinType(static_cast<clang::BuiltinType const*>(type));
      case clang::Type::Enum:
         assert(llvm::dyn_cast<const clang::EnumType>(type));
         return std::string("enum ") + static_cast<clang::EnumType const*>
            (type)->getDecl()->getName().str();
      case clang::Type::Auto:
         {  assert(llvm::dyn_cast<const clang::AutoType>(type));
            const auto* autotype = static_cast<clang::AutoType const*>(type);
            return autotype->isDeduced() ? printArithmeticType(
                  autotype->getDeducedType().getTypePtr()) : std::string("");
         }
      case clang::Type::SubstTemplateTypeParm:
         {  assert(llvm::dyn_cast<const clang::SubstTemplateTypeParmType>(type));
            const auto* replacementType = static_cast<clang::SubstTemplateTypeParmType const*>(type);
            return printArithmeticType(replacementType->getReplacementType().getTypePtr());
         };
      case clang::Type::TemplateSpecialization:
         {  assert(llvm::dyn_cast<const clang::TemplateSpecializationType>(type));
            const auto* specializationType = static_cast<clang::TemplateSpecializationType const*>(type);
            return specializationType->isSugared()
               ? printArithmeticType(specializationType->desugar().getTypePtr())
               : std::string("");
         }
      case clang::Type::Typedef:
         {  assert(llvm::dyn_cast<const clang::TypedefType>(type));
            const auto* replacementType = static_cast<clang::TypedefType const*>(type);
            return replacementType->isSugared()
               ? printArithmeticType(replacementType->desugar().getTypePtr())
               : std::string("");
         };
      case clang::Type::Elaborated:
         {  assert(llvm::dyn_cast<const clang::ElaboratedType>(type));
            const auto* elaborated = static_cast<clang::ElaboratedType const*>(type);
            return elaborated->isSugared()
               ? printArithmeticType(elaborated->desugar().getTypePtr())
               : std::string("");
         }
      default: break;
   }
   return "";
}

std::string
CallersAction::Visitor::printPlainType(clang::QualType const& qt) const {
   const clang::Type* type = qt.getTypePtr();
   switch(type->getTypeClass()) {
      case clang::Type::Builtin:
         {  assert(llvm::dyn_cast<const clang::BuiltinType>(type));
            return printBuiltinType(static_cast<clang::BuiltinType const*>(type));
         };
      case clang::Type::Pointer:
         {  assert(llvm::dyn_cast<const clang::PointerType>(type));
            const auto* pointerType = static_cast<clang::PointerType const*>(type);
            clang::QualType subPointerType = pointerType->getPointeeType();
            std::string subType = printPlainType(subPointerType);
            if (subPointerType.isLocalRestrictQualified())
               subType = "restrict " + subType;
            if (subPointerType.isLocalVolatileQualified())
               subType = "volatile " + subType;
            if (subPointerType.isLocalConstQualified())
               subType = "const " + subType;
            if (subType.length() > 0 && subType[subType.length()-1] == '&')
               subType[subType.length()-1] = '*';
            else
               subType = std::string("(") + subType + std::string(")*");
            return subType;
         };
      case clang::Type::LValueReference:
         {  assert(llvm::dyn_cast<const clang::LValueReferenceType>(type));
            const auto* referenceType = static_cast<clang::PointerType const*>(type);
            clang::QualType subReferenceType = referenceType->getPointeeType();
            std::string subType = printPlainType(subReferenceType);
            if (subReferenceType.isLocalRestrictQualified())
               subType = "restrict " + subType;
            if (subReferenceType.isLocalVolatileQualified())
               subType = "volatile " + subType;
            if (subReferenceType.isLocalConstQualified())
               subType = "const " + subType;
            subType = std::string("(") + subType + std::string(")&");
            return subType;
         };
      case clang::Type::MemberPointer:
         {  assert(llvm::dyn_cast<const clang::MemberPointerType>(type));
            const auto* memberPointerType = static_cast<clang::MemberPointerType const*>(type);
            std::string className = printQualifiedName(*memberPointerType->getMostRecentCXXRecordDecl());
            if (memberPointerType->isMemberFunctionPointer()) {
               const auto* prototype = (memberPointerType->getPointeeType()
                     .getTypePtr()->getAs<clang::FunctionProtoType>());
               assert(prototype);
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
               std::string result = printType(prototype->getReturnType());
#else
               std::string result = printType(prototype->getResultType());
#endif
               result += " (";
               result += className;
               result += "::*)(";
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
               auto paramIter = prototype->param_type_begin(),
                    paramIterEnd = prototype->param_type_end();
#else
               auto paramIter = prototype->arg_type_begin(),
                    paramIterEnd = prototype->arg_type_end();
#endif
               bool isFirst = true;
               for (; paramIter != paramIterEnd; ++paramIter) {
                  if (!isFirst)
                     result += ", ";
                  result += printType(*paramIter);
                  isFirst = false;
               };
               result += ')';
               if (prototype->isConst())
                  result += " const";
               if (prototype->isVolatile())
                  result += " volatile";
               if (prototype->isRestrict())
                  result += " restrict";
               return result;
            }
            else { // memberPointerType->isMemberDataPointer()
               std::string result = printType(memberPointerType->getPointeeType());
               result += ' ';
               result += className;
               result += "::*";
               return result;
            };
         };
      case clang::Type::ConstantArray:
         {  assert(llvm::dyn_cast<const clang::ConstantArrayType>(type));
            const auto* arrayType = static_cast<clang::ConstantArrayType const*>(type);
            clang::QualType subArrayType = arrayType->getElementType();
            std::string subType = printPlainType(subArrayType);
            if (subArrayType.isLocalRestrictQualified())
               subType = "restrict " + subType;
            if (subArrayType.isLocalVolatileQualified())
               subType = "volatile " + subType;
            if (subArrayType.isLocalConstQualified())
               subType = "const " + subType;
            subType = std::string("(") + subType + std::string(")[");
            {  std::ostringstream out;
               out << arrayType->getSize().getLimitedValue(UINT64_MAX);
               subType += out.str();
               subType += ']';
            };
            return subType;
         };
      case clang::Type::IncompleteArray:
         {  assert (llvm::dyn_cast<const clang::IncompleteArrayType>(type));
            const auto* arrayType = static_cast<clang::IncompleteArrayType const*>(type);
            clang::QualType subArrayType = arrayType->getElementType();
            std::string subType = printPlainType(subArrayType);
            if (subArrayType.isLocalRestrictQualified())
               subType = "restrict " + subType;
            if (subArrayType.isLocalVolatileQualified())
               subType = "volatile " + subType;
            if (subArrayType.isLocalConstQualified())
               subType = "const " + subType;
            subType = std::string("(") + subType + std::string(")[]");
            return subType;
         }
      case clang::Type::FunctionProto:
         {  assert(llvm::dyn_cast<const clang::FunctionProtoType>(type));
            const auto* functionType = static_cast<clang::FunctionProtoType const*>(type);
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
            std::string result = printType(functionType->getReturnType());
#else
            std::string result = printType(functionType->getResultType());
#endif
            result += " (*)(";
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
            auto paramIter = functionType->param_type_begin(),
                 paramIterEnd = functionType->param_type_end();
#else
            auto paramIter = functionType->arg_type_begin(),
                 paramIterEnd = functionType->arg_type_end();
#endif
            bool isFirst = true;
            for (; paramIter != paramIterEnd; ++paramIter) {
               if (!isFirst)
                  result += ", ";
               result += printType(*paramIter);
               isFirst = false;
            };
            result += ')';
            if (functionType->isConst())
               result += " const";
            if (functionType->isVolatile())
               result += " volatile";
            if (functionType->isRestrict())
               result += " restrict";
            return result;
         };
      case clang::Type::FunctionNoProto:
         return "";
         // std::cerr << "Unsupported K&R Declaration Function Type:"
         //       << qt.getAsString ()
         //       << "\nAborting\n";
         // exit(2);
      case clang::Type::Record:
         {  assert(llvm::dyn_cast<const clang::RecordType>(type));
            const auto* recordType = static_cast<clang::RecordType const*>(type);
            return printQualifiedName(*recordType->getDecl());
         };
      case clang::Type::Enum:
         {  assert(llvm::dyn_cast<const clang::EnumType>(type));
            const auto* enumType = static_cast<clang::EnumType const*>(type);
            return printQualifiedName(*enumType->getDecl());
         };
      case clang::Type::Auto:
         {  assert(llvm::dyn_cast<const clang::AutoType>(type));
            const auto* autotype = static_cast<clang::AutoType const*>(type);
            if (autotype->isDeduced())
               return printPlainType(autotype->getDeducedType());
            else { 
               return "";
               // std::cerr << "Unresolved auto type" << "\nAborting\n";
               // exit(2);
            }
         }
      case clang::Type::SubstTemplateTypeParm:
         {  assert(llvm::dyn_cast<const clang::SubstTemplateTypeParmType>(type));
            const auto* replacementType = static_cast<clang::SubstTemplateTypeParmType const*>(type);
            return printPlainType(replacementType->getReplacementType());
         };
      case clang::Type::TemplateSpecialization:
         {  assert(llvm::dyn_cast<const clang::TemplateSpecializationType>(type));
            const auto* sp_type = static_cast<clang::TemplateSpecializationType const*>(type);
            if (sp_type->isSugared())
               return printPlainType(sp_type->desugar());
            else {
               return "";
               // std::cerr << "uninstantiated template specialization" << "\nAborting\n";
               // exit(2);
            }
         }
      case clang::Type::Typedef:
         {  assert(llvm::dyn_cast<const clang::TypedefType>(type));
            const auto* replacementType = static_cast<clang::TypedefType const*>(type);
            return printPlainType(replacementType->getDecl()->getUnderlyingType());
         };
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
      case clang::Type::Decayed:
      case clang::Type::Adjusted:
         {  assert(llvm::dyn_cast<const clang::AdjustedType>(type));
            const auto* adjustedType = static_cast<clang::AdjustedType const*>(type);
            return printPlainType(adjustedType->getOriginalType());
         };
#else
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_4
      // Adjusted was apparently introduced in 3.5, but Decayed already exists
      // in 3.4
      case clang::Type::Decayed:
         { assert(llvm::dyn_cast<const clang::DecayedType>(type));
            const auto* decayed = static_cast<clang::DecayedType const*>(type);
            return printPlainType(decayed->getOriginalType());
         }
#else
    // no alternative for the moment. Decayed types have been introduced
    // in September 2013, for the quoted following reason:
    /* The goal of this sugar node is to be able to look at an arbitrary
         FunctionType and tell if any of the parameters was decayed from an array
         or function type. Ultimately this is necessary to implement Microsoft's
         C++ name mangling scheme, which mangles decayed arrays differently from
         normal pointers.
    */
#endif //3_3_4
#endif //3_3_5

      case clang::Type::Paren:
         {  assert(llvm::dyn_cast<const clang::ParenType>(type));
            const auto* parentype = static_cast<clang::ParenType const*>(type);
            return printPlainType(parentype->getInnerType());
         };
      case clang::Type::Elaborated:
         {  assert(llvm::dyn_cast<const clang::ElaboratedType>(type));
            const auto* elaborated = static_cast<clang::ElaboratedType const*>(type);
            if (elaborated->isSugared())
               return printPlainType(elaborated->desugar());
            else {
               return "";
               // std::cerr << "unresolved elaborated type" << "\nAborting\n";
               // exit(2);
            }
         }
      case clang::Type::Decltype:
         {  assert(llvm::dyn_cast<const clang::DecltypeType>(type));
            const auto* decltypeType = static_cast<clang::DecltypeType const*>(type);
            if (decltypeType->isSugared())
               return printPlainType(decltypeType->desugar());
            else {
               return "";
               // std::cerr << "unresolved decltype type" << "\nAborting\n";
               // exit(2);
            }
         };
      case clang::Type::InjectedClassName:
         {  assert(llvm::dyn_cast<const clang::InjectedClassNameType>(type));
            const auto* injectedType = static_cast<clang::InjectedClassNameType const*>(type);
            return printPlainType(injectedType->getInjectedSpecializationType());
         }
      case clang::Type::Attributed: 
      case clang::Type::Atomic:
      case clang::Type::VariableArray:
      case clang::Type::RValueReference:
      case clang::Type::BlockPointer:
      case clang::Type::Complex:
      case clang::Type::TemplateTypeParm:
      case clang::Type::SubstTemplateTypeParmPack:
         return "";
      default:
         return "";
   }
   // all successful cases have already returned.
}

std::string
CallersAction::Visitor::printType(clang::QualType const& qt) const {
   std::string result = printPlainType(qt);
   if (qt.isLocalRestrictQualified())
      result = "restrict " + result;
   if (qt.isLocalVolatileQualified())
      result = "volatile " + result;
   if (qt.isLocalConstQualified())
      result = "const " + result;
   return result;
}

std::string
CallersAction::Visitor::printCompoundType(clang::Type const* type) const {
   switch(type->getTypeClass()) {
      case clang::Type::Record:
         {  assert(llvm::dyn_cast<const clang::RecordType>(type));
            const auto* recordType = static_cast<clang::RecordType const*>(type);
            return printQualifiedName(*recordType->getDecl()) + printTemplateKind(*recordType->getDecl());
         }
      case clang::Type::LValueReference:
      case clang::Type::RValueReference:
         {  assert(llvm::dyn_cast<const clang::ReferenceType>(type));
            const auto* referenceType = static_cast<clang::ReferenceType const*>(type);
            return printCompoundType(referenceType->getPointeeType().getTypePtr());
         }
      case clang::Type::Auto:
         {  assert(llvm::dyn_cast<const clang::AutoType>(type));
            const auto* autotype = static_cast<clang::AutoType const*>(type);
            return autotype->isDeduced()
                  ? printCompoundType(autotype->getDeducedType().getTypePtr())
                  : std::string("");
         }
      case clang::Type::SubstTemplateTypeParm:
         {  assert(llvm::dyn_cast<const clang::SubstTemplateTypeParmType>(type));
            const auto* replacementType = static_cast<clang::SubstTemplateTypeParmType const*>(type);
            return printCompoundType(replacementType->getReplacementType().getTypePtr());
         };
      case clang::Type::TemplateSpecialization:
         {  assert(llvm::dyn_cast<const clang::TemplateSpecializationType>(type));
            const auto* specializationType = static_cast<clang::TemplateSpecializationType const*>(type);
            return specializationType->isSugared()
               ? printCompoundType(specializationType->desugar().getTypePtr())
               : std::string("");
         }
      case clang::Type::Typedef:
         {  assert(llvm::dyn_cast<const clang::TypedefType>(type));
            const auto* replacementType = static_cast<clang::TypedefType const*>(type);
            return replacementType->isSugared()
               ? printCompoundType(replacementType->desugar().getTypePtr())
               : std::string("");
         };
      case clang::Type::Elaborated:
         {  assert(llvm::dyn_cast<const clang::ElaboratedType>(type));
            const auto* elaborated = static_cast<clang::ElaboratedType const*>(type);
            return elaborated->isSugared()
               ? printCompoundType(elaborated->desugar().getTypePtr())
               : std::string("");
         }
      default: break;
   }
   return NULL;
}

std::string
CallersAction::Visitor::printArgumentSignature(const clang::FunctionDecl& function) const {
   std::string result = "(";
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
   auto paramIter = function.param_begin(),
        paramIterEnd = function.param_end();
#else
   auto paramIter = function.arg_begin(),
        paramIterEnd = function.arg_end();
#endif
   bool isFirst = true;

   for (; paramIter != paramIterEnd; ++paramIter) {
      if (!isFirst)
         result += ", ";
      result += printType((*paramIter)->getType());
      isFirst = false;
   };
   result += ')';
   if (llvm::dyn_cast<clang::CXXMethodDecl>(&function)) {
      const auto& method = static_cast<const clang::CXXMethodDecl&>(function);
      if (method.isConst())
         result += " const";
      if (method.isVolatile())
         result += " volatile";
   };
   return result;
}

std::string
CallersAction::Visitor::printResultSignature(const clang::FunctionDecl& function) const {
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
   return printType(function.getReturnType());
#else
   return printType(function.getResultType());
#endif
}

std::string CallersAction::Visitor::parseQualification(const clang::DeclContext* context,
						       std::set<CallersData::File>::iterator *file) {
  if (context) {
    std::string qualifiers = parseQualification(context->getParent(), file);
    std::string result = qualifiers;
    const clang::Decl::Kind kind = context->getDeclKind();

    if (kind == clang::Decl::Namespace) {
      const clang::NamespaceDecl* nspc = static_cast<const clang::NamespaceDecl*>(context);
      //std::set<CallersData::Namespace>::iterator namespc = (*file)->create_or_get_namespace(qualifiers, *nspc);
      CallersData::Namespace c_namespace(qualifiers, *nspc);
      (*file)->add_namespace(c_namespace);
      if (result.length() > 0) {
	//std::cerr << "nspc: " << c_namespace.name << ", qualifiers: " << qualifiers << std::endl;
	result += "::";
      }
      else {
	//result += "::";
      }
      //result += static_cast<const clang::NamespaceDecl*>(context)->getName().str();
      result += nspc->getName().str();
    }
    else if (kind >= clang::Decl::firstTag && kind <= clang::Decl::lastTag) {
      if (result.length() > 0)
	result += "::";
      result += static_cast<const clang::TagDecl*>(context)->getName().str();
      if (kind >= clang::Decl::firstCXXRecord && kind <= clang::Decl::lastCXXRecord)
	result += printTemplateKind(*static_cast<const clang::CXXRecordDecl*>(context));
    };
    return result;
  };
  return "";
}

std::string
CallersAction::Visitor::printQualification(const clang::DeclContext* context) const {
  if (context) {
    std::string result = printQualification(context->getParent());
    const clang::Decl::Kind kind = context->getDeclKind();
    if (kind == clang::Decl::Namespace) 
      {
	if (result.length() > 0)
	  result += "::";
	else
	  result += "::";
	  
	result += static_cast<const clang::NamespaceDecl*>(context)->getName().str();
      }
    else if (kind >= clang::Decl::firstTag && kind <= clang::Decl::lastTag) {
      if (result.length() > 0)
	result += "::";
      result += static_cast<const clang::TagDecl*>(context)->getName().str();
      if (kind >= clang::Decl::firstCXXRecord && kind <= clang::Decl::lastCXXRecord)
	result += printTemplateKind(*static_cast<const clang::CXXRecordDecl*>(context));
    };
    return result;
  };
  return "";
}

std::string
CallersAction::Visitor::parseQualifiedName(const clang::NamedDecl& namedDecl, bool* isEmpty) {

  const clang::DeclContext *context = namedDecl.getDeclContext();

  std::string filepath = printFilePath(namedDecl.getSourceRange());
  boost::filesystem::path p(filepath);
  std::string basename = p.filename().string();
  std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
  std::set<CallersData::File>::iterator file = otherJsonFiles.create_or_get_file(basename, dirpath);

  std::string result = parseQualification(context, &file);

   if (result.length() > 0)
      result += "::";
   std::string pureName = namedDecl.getNameAsString();
   if (isEmpty)
      *isEmpty = pureName.length() == 0;
   result += pureName;
   return result;
}

std::string
CallersAction::Visitor::printQualifiedName(const clang::NamedDecl& namedDecl, bool* isEmpty) const {

  const clang::DeclContext *context = namedDecl.getDeclContext();
  std::string result = printQualification(context);
   if (result.length() > 0)
      result += "::";
   std::string pureName = namedDecl.getNameAsString();
   if (isEmpty)
      *isEmpty = pureName.length() == 0;
   result += pureName;
   return result;
}

std::string
CallersAction::Visitor::getJsonIdentifier(const std::string& name) const {
  std::string s = std::string("\"") + name + "\"";
  return s;
}

std::string
CallersAction::Visitor::getBasename(const clang::StringRef& filename) const {

  std::pair<clang::StringRef, clang::StringRef> fullName = filename.rsplit('/');
  return fullName.second.str();
}

void
CallersAction::Visitor::getMangledName(clang::MangleContext* ctx,
                                       const clang::FunctionDecl* nd,
                                       MangledName* result)
{
  llvm::SmallVector<char, 512> output;
  llvm::raw_svector_ostream out(output);
  switch(nd->getKind())
  {
  case clang::Decl::CXXConstructor:
    {
      const auto& ct = llvm::cast<clang::CXXConstructorDecl>(nd);
      clang::GlobalDecl gd = clang::GlobalDecl(ct, clang::Ctor_Base);
      ctx->mangleCXXCtor(ct, gd.getCtorType(), out);
      break;
    }
  case clang::Decl::CXXDestructor:
    {
      const auto& dt = llvm::cast<clang::CXXDestructorDecl>(nd);
      clang::GlobalDecl gd = clang::GlobalDecl(dt, clang::Dtor_Base);
      ctx->mangleCXXDtor(dt, gd.getDtorType(), out);
      break;
    }
  default:
    {
      ctx->mangleName(nd, out);
      break;
    }
  }
  *result = out.str().str();
}

std::string
CallersAction::Visitor::writeFunction(const clang::FunctionDecl& function, bool isUnqualified) const {
   std::string result = printResultSignature(function);
   result += ' ';
   bool isEmptyName = false;
   std::string name;
   if (!isUnqualified)
      name = printQualifiedName(function, &isEmptyName);
   else {
      name = function.getName().str();
      isEmptyName = name.length() == 0;
   };
   if (isEmptyName) {
      if (function.getKind() == clang::Decl::CXXConstructor)
         name += "constructor-special";
      else if (function.getKind() == clang::Decl::CXXDestructor)
         name += "destructor-special";
      else {
         // see build-llvm/tools/clang/include/clang/AST/DeclNodes.inc
      };
   };
   result += name;
   result += printTemplateKind(function);
   result += printArgumentSignature(function);
   return result;
}

bool
CallersAction::Visitor::VisitCXXConstructExpr(const clang::CXXConstructExpr* constructor) {
   clang::CXXConstructorDecl *constr = constructor->getConstructor();
   assert(llvm::dyn_cast<clang::FunctionDecl>(constr));
   const auto& function = *static_cast<const clang ::FunctionDecl*>(constr);
   std::string name = printQualifiedName(function);
   MangledName fct_mangledName;
   this->getMangledName(mangle_context_, &function, &fct_mangledName);
   if (name.length() == 0)
      name = "constructor-special";
   std::string result = name;
   result += printTemplateKind(function);
   result += printArgumentSignature(function);
   osOut << inputFile << ": " << printParentFunction() << " -1-> " << result << '\n';
   std::string callee_sign = writeFunction(function);
   std::string callee_filepath = printFilePath(function.getSourceRange());
   int callee_filepos = printLine(function.getSourceRange());
   if(callee_filepath == "unknownFilePath")
     {
       osOut << inputFile << ":WARNING: unknownFilePath for callee: " << callee_sign << std::endl;
       callee_filepos = -1;
     }
   assert(pfdParent != NULL);
   auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);
   std::string caller_record = CALLERS_DEFAULT_NO_RECORD_NAME;
   if((constr != NULL) && (constr->getParent() != NULL))
   {
     caller_record = constr->getParent()->getQualifiedNameAsString();
   }
   std::string callee_record = caller_record;
   CallersData::FctCall fc(fct_mangledName, printParentFunction(),
                           (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual,
                           printParentFunctionFilePath(), printParentFunctionLine(),
                           fct_mangledName, callee_sign, CallersData::VNoVirtual, callee_filepath, callee_filepos, caller_record, callee_record);
   currentJsonFile->add_function_call(&fc, &otherJsonFiles);
   return true;
}

bool
CallersAction::Visitor::VisitCXXDeleteExpr(const clang::CXXDeleteExpr* deleteExpr) {
   if (deleteExpr->getOperatorDelete()
         && !deleteExpr->getOperatorDelete()->isImplicit()) {
      const clang::FunctionDecl& function = *deleteExpr->getOperatorDelete();
      std::string callee_sign = printResultSignature(function);
      MangledName fct_mangledName;
      this->getMangledName(mangle_context_, &function, &fct_mangledName);
      callee_sign += ' ';
      callee_sign += printQualifiedName(function);
      callee_sign += printArgumentSignature(function);
      osOut << inputFile << ": " << printParentFunction() << " -2-> " << callee_sign << '\n';
      std::string callee_filepath = printFilePath(function.getSourceRange());
      int callee_filepos = printLine(function.getSourceRange());
      if(callee_filepath == "unknownFilePath")
	{
	  osOut << inputFile << ":WARNING: unknownFilePath for callee: " << callee_sign << std::endl;
	  callee_filepos = -1;
	}
      auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);
      std::string caller_record = CALLERS_DEFAULT_NO_RECORD_NAME;
      if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
      {
        caller_record = parentMethod->getParent()->getQualifiedNameAsString();
      }
      auto calleeMethod = llvm::dyn_cast<clang::CXXMethodDecl>(&function);
      std::string callee_record = CALLERS_DEFAULT_NO_RECORD_NAME;
      if((calleeMethod != NULL) && (calleeMethod->getParent() != NULL))
      {
        callee_record = calleeMethod->getParent()->getQualifiedNameAsString();
      }
      CallersData::FctCall fc(fct_mangledName, printParentFunction(),
          (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual,
          printParentFunctionFilePath(), printParentFunctionLine(),
          fct_mangledName, callee_sign,
          (!calleeMethod || !calleeMethod->isVirtual()) ? CallersData::VNoVirtual
            : (calleeMethod->isPure() ? CallersData::VVirtualPure
            : (calleeMethod->isThisDeclarationADefinition() ? CallersData::VVirtualDefined : CallersData::VVirtualDeclared)),
          callee_filepath, callee_filepos, caller_record, callee_record);
      currentJsonFile->add_function_call(&fc, &otherJsonFiles);
      return true;
   };
   const auto* recordDecl = deleteExpr->getType()->getPointeeCXXRecordDecl();
   if (!recordDecl)
      recordDecl = deleteExpr->getType()->getCanonicalTypeInternal().getTypePtr()
         ->getAsCXXRecordDecl();
   if (recordDecl) {
      clang::CXXDestructorDecl* destructor = recordDecl->getDestructor();
      MangledName fct_mangledName;
      this->getMangledName(mangle_context_, destructor, &fct_mangledName);
      auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);
      auto calleeMethod = llvm::dyn_cast<clang::CXXMethodDecl>(destructor);
      if (destructor) {
         std::string callee_sign = printQualifiedName(*destructor);
         callee_sign += "()";
         osOut << inputFile << ": " << printParentFunction() << " -3-> " << callee_sign << '\n';
	 std::string callee_filepath = printFilePath(destructor->getSourceRange());
	 int callee_filepos = printLine(destructor->getSourceRange());
	 if(callee_filepath == "unknownFilePath")
	   {
	     osOut << inputFile << ":WARNING: unknownFilePath for callee: " << callee_sign << std::endl;
	     callee_filepos = -1;
	   }
         std::string caller_record = CALLERS_DEFAULT_NO_RECORD_NAME;
         if((destructor != NULL) && (destructor->getParent() != NULL))
         {
           caller_record = destructor->getParent()->getQualifiedNameAsString();
         }
         std::string callee_record = caller_record;
	 CallersData::FctCall fc(fct_mangledName, printParentFunction(),
                                 (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual,
                                 printParentFunctionFilePath(), printParentFunctionLine(),
                                 fct_mangledName, callee_sign,
                                 (!calleeMethod || !calleeMethod->isVirtual()) ? CallersData::VNoVirtual
                                 : (calleeMethod->isPure() ? CallersData::VVirtualPure
                                 : (calleeMethod->isThisDeclarationADefinition() ? CallersData::VVirtualDefined : CallersData::VVirtualDeclared)),
                                 callee_filepath, callee_filepos,
                                 caller_record, callee_record);
	 currentJsonFile->add_function_call(&fc, &otherJsonFiles);
      };
   };
   return true;
}

bool
CallersAction::Visitor::VisitCXXNewExpr(const clang::CXXNewExpr* newExpr) {
  auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);
  if (newExpr->getOperatorNew() && !newExpr->getOperatorNew()->isImplicit())
    {
      const auto& operatorNew = *newExpr->getOperatorNew();
      std::string callee_sign = printResultSignature(operatorNew);
      MangledName fct_mangledName;
      this->getMangledName(mangle_context_, &operatorNew, &fct_mangledName);
      callee_sign += ' ';
      callee_sign += printQualifiedName(operatorNew);
      if (newExpr->isArray())
	callee_sign += " [] ";
      callee_sign += printArgumentSignature(operatorNew);
      osOut << inputFile << ": " << printParentFunction() << " -4-> " << callee_sign << '\n';
      std::string callee_filepath = printFilePath(operatorNew.getSourceRange());
      int callee_filepos = printLine(operatorNew.getSourceRange());
      if(callee_filepath == "unknownFilePath")
	{
	  osOut << inputFile << ":WARNING: unknownFilePath for callee: " << callee_sign << std::endl;
	  callee_filepos = -1;
	}
      std::string caller_record = CALLERS_DEFAULT_NO_RECORD_NAME;
      if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
      {
        caller_record = parentMethod->getParent()->getQualifiedNameAsString();
      }
      std::string callee_record = caller_record;
      CallersData::FctCall fc(fct_mangledName, printParentFunction(),
          (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual,
          printParentFunctionFilePath(), printParentFunctionLine(),
          fct_mangledName, callee_sign, CallersData::VNoVirtual, callee_filepath, callee_filepos, caller_record, callee_record);
      currentJsonFile->add_function_call(&fc, &otherJsonFiles);
    }
  else
    {
      std::string malloc_sign = "malloc";
      //std::string malloc_sign = "void *malloc(size_t)";
      std::cout << "WARNING VisitCXXNewExpr: implicit operator new, replace it by an explicit call to \"" << malloc_sign << "\"" << std::endl;
      osOut << inputFile << ": " << printParentFunction() << " -5-> " << malloc_sign << '\n';
      MangledName malloc_mangled = "builtin_malloc";
      std::string malloc_filepath = "/usr/include/malloc.h";
      //int malloc_filepos = -1; //unknown_position
      int malloc_filepos = 38; //line position on my ubuntu
      //int callee_filepos = 51;

      // this bloc is inspired from function CallersAction::Visitor::VisitBuiltinFunction
      // It checks whether a json file is already present for the malloc builtin function
      // if true, parse it and add the defined function only when necessary
      // if false, create this json file and add the defined function
      std::string malloc_record(CALLERS_DEFAULT_RECORD_BUILTIN);
      {
	boost::filesystem::path p(malloc_filepath);
	std::string basename = p.filename().string();
	std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
	std::set<CallersData::File>::iterator file = otherJsonFiles.create_or_get_file(basename, dirpath);
	CallersData::FctDecl fct(malloc_mangled, malloc_sign, CallersData::VNoVirtual, malloc_filepath, malloc_filepos, malloc_record);
	file->add_declared_function(&fct, malloc_filepath, &otherJsonFiles);
      }

      std::string caller_record = CALLERS_DEFAULT_NO_RECORD_NAME;
      if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
      {
        caller_record = parentMethod->getParent()->getQualifiedNameAsString();
      }
      CallersData::FctCall fc(malloc_mangled, printParentFunction(),
                              (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual,
                              printParentFunctionFilePath(), printParentFunctionLine(),
                              malloc_mangled, malloc_sign, CallersData::VNoVirtual,
                              malloc_filepath, malloc_filepos, caller_record, malloc_record);
      fc.is_builtin = true;
      currentJsonFile->add_function_call(&fc, &otherJsonFiles);
    }
   return true;
}

bool
CallersAction::Visitor::VisitBuiltinFunction(const clang::FunctionDecl* fd) {

  unsigned builtinID = fd->getBuiltinID();
  std::string builtinName = "notFoundBuiltinName";
  std::string headerName = "notFoundBuiltinImpl";
#define BUILTIN(ID, TYPE, ATTRS) case clang::Builtin::BI##ID: builtinName = #ID; break;
  // TODO: tries to get the header file absolute path. This doesn't work with the getCanonicalAbsolutePath() function
  // which concatenates the input path or filename with the current working path; and not the path to the system repository
  // where the file is really located
  //#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER, BUILTIN_LANG) case clang::Builtin::BI##ID: builtinName = #ID; headerName = ::getCanonicalAbsolutePath(HEADER); break;
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER, BUILTIN_LANG) case clang::Builtin::BI##ID: builtinName = #ID; headerName = HEADER; break;
  switch( builtinID) {
#include "clang/Basic/Builtins.def"
  };

  std::string builtinFile = printFilePath(fd->getSourceRange());
  MangledName builtin_mangled;
  this->getMangledName(mangle_context_, fd, &builtin_mangled);
  int builtinPos = printLine(fd->getSourceRange());
  assert(builtinName != "notFoundBuiltinName");
  std::cout << "DEBUG: builtin name: \"" << builtinName << "\"" << std::endl;
  std::cout << "DEBUG: builtin decl location: " << builtinFile << ":" << builtinPos << std::endl;
  std::cout << "DEBUG: builtin def location (headerName): " << headerName << std::endl;
  osOut << inputFile << ":builtin: " << printParentFunction() << " -6-> " << builtinName << ", defined in: " << headerName << ":" << builtinPos << std::endl;
  if(headerName == "notFoundBuiltinImpl")
    {
      // std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
      // std::cerr << "ERROR: visitor.cpp : not found implem of builtin: \"" << builtinName << "\", headerName: \"" << headerName << "\"" << std::endl;
      // std::cerr << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
      //std::cout << "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" << std::endl;
      std::cout << "WARNING: visitor.cpp : not found implementation of builtin: \"" << builtinName << "\", headerName: \"" << headerName << "\"" << std::endl;
      //std::cout << "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" << std::endl;
      headerName = builtinFile;
      // CallersData::FctCall fc = CallersData::FctCall(printParentFunction(), printParentFunctionFilePath(), printParentFunctionLine(),
      // 						     builtinName, printFilePath(fd->getSourceRange()), builtinPos);
      // fc.is_builtin = true;
      // currentJsonFile->add_function_call(&fc, &otherJsonFiles);
      //exit(5);
    }
  else
    {
      // Tries to get the full path of the builtin implementation file
      //if(headerName.length() > 0)

      clang::SourceLocation FilenameLoc;
      llvm::StringRef Filename(headerName);
      bool isAngled = true;
      const clang::DirectoryLookup * FromDir = NULL;
      const clang::FileID FromFileID = ciCompilerInstance.getSourceManager().getMainFileID();
      const clang::FileEntry * FromFile = ciCompilerInstance.getSourceManager().getFileEntryForID(FromFileID);
      const clang::DirectoryLookup * CurDir = NULL;
      llvm::SmallVectorImpl<char>* SearchPath = NULL;
      llvm::SmallVectorImpl<char>* RelativePath = NULL;
      clang::ModuleMap::KnownHeader *SuggestedModule = NULL;
      bool skipPath = false;
      const clang::FileEntry * result = ciCompilerInstance.getPreprocessor().LookupFile(FilenameLoc, Filename, isAngled, FromDir, FromFile, CurDir, SearchPath, RelativePath, SuggestedModule, skipPath);
      if(result)
	headerName = result->getName();
    }

  std::cout << "DEBUG: builtin location (headerName): " << headerName << std::endl;

  // check whether a json file is already present for the builtin function
  // if true, parse it and add the defined function only when necessary
  // if false, create this json file and add the defined function
  {
    boost::filesystem::path p(headerName);
    std::string basename = p.filename().string();
    std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
    std::set<CallersData::File>::iterator file = otherJsonFiles.create_or_get_file(basename, dirpath);
    std::string builtin_record(CALLERS_DEFAULT_RECORD_BUILTIN);
    CallersData::FctDecl fct(builtin_mangled, builtinName, CallersData::VNoVirtual, headerName, builtinPos, builtin_record);
    file->add_declared_function(&fct, headerName, &otherJsonFiles);
    auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);
    std::string caller_record(CALLERS_DEFAULT_NO_RECORD_NAME);
    if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
    {
      caller_record = parentMethod->getParent()->getQualifiedNameAsString();
    }
    CallersData::FctCall fc = CallersData::FctCall(builtin_mangled, printParentFunction(),
						   (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual,
						   printParentFunctionFilePath(), printParentFunctionLine(),
						   builtin_mangled, builtinName, CallersData::VNoVirtual, headerName,
                                                   builtinPos, caller_record, builtin_record);
    fc.is_builtin = true;
    currentJsonFile->add_function_call(&fc, &otherJsonFiles);
  }
  return true;
}

bool
CallersAction::Visitor::VisitCallExpr(const clang::CallExpr* callExpr) {
   const clang::FunctionDecl* callee = callExpr->getDirectCallee();
   MangledName callee_mangled;
   this->getMangledName(mangle_context_, callee, &callee_mangled);
   if (callee) {
      if (callee->getBuiltinID() > 0)
       {
	  this->VisitBuiltinFunction(callee);
	  return true;
       }
      if (callee->isThisDeclarationADefinition())
       {
         callee = callee->getCanonicalDecl();
       }
      std::string calleeName = writeFunction(*callee);
      osOut << inputFile << ": " << printParentFunction() << " -7-> " << calleeName << '\n';
      std::string callee_filepath = printFilePath(callee->getSourceRange());
      int callee_filepos = printLine(callee->getSourceRange());
      if(callee_filepath == "unknownFilePath")
	{
	  callee_filepos = -1;
	}
      auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);
      auto calleeMethod = llvm::dyn_cast<clang::CXXMethodDecl>(callee);
      std::string caller_record = CALLERS_DEFAULT_NO_RECORD_NAME;
      if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
      {
        caller_record = parentMethod->getParent()->getQualifiedNameAsString();
      }
      std::string callee_record = CALLERS_DEFAULT_NO_RECORD_NAME;
      if((calleeMethod != NULL) && (calleeMethod->getParent() != NULL))
      {
        callee_record = calleeMethod->getParent()->getQualifiedNameAsString();
      }
      CallersData::FctCall fc(callee_mangled, printParentFunction(),
          (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual,
          printParentFunctionFilePath(), printParentFunctionLine(),
          callee_mangled, calleeName,
          (!calleeMethod || !calleeMethod->isVirtual()) ? CallersData::VNoVirtual
            : (calleeMethod->isPure() ? CallersData::VVirtualPure
            : (calleeMethod->isThisDeclarationADefinition() ? CallersData::VVirtualDefined : CallersData::VVirtualDeclared)),
          callee_filepath, callee_filepos, caller_record, callee_record);
      currentJsonFile->add_function_call(&fc, &otherJsonFiles);
      return true;
   }

   if (callExpr->getCallee()->getStmtClass() == clang::Stmt::CXXPseudoDestructorExprClass)
      return true;
   osOut << "  " << printParentFunction() << " -> dynamic call\n";
   return true;
}

bool
CallersAction::Visitor::VisitMemberCallExpr(const clang::CXXMemberCallExpr* callExpr) {
   const clang::Expr* callee = callExpr->getCallee();
   bool isVirtualCall = callExpr->getMethodDecl()->isVirtual();
   if (isVirtualCall && callee) {
      assert(llvm::dyn_cast<const clang::MemberExpr>(callee));
      isVirtualCall = !static_cast<const clang::MemberExpr*>(callee)->hasQualifier();
   }
   const clang::FunctionDecl* directCall = callExpr->getMethodDecl();
   if (isVirtualCall) {
      clang::CXXRecordDecl* baseClass = callExpr->getMethodDecl()->getParent();
      // _callersAction->registerDerivedCalls(sParent, baseClass, callExpr->getMethodDecl());
      std::string calleeName = writeFunction(*directCall);
      osOut << printParentFunction() << " -> derived of class "
         << printQualifiedName(*baseClass) << printTemplateKind(*baseClass)
         << " method " << calleeName << '\n';
   }
   else {
      std::string calleeName = writeFunction(*directCall);
      osOut << "  " << printParentFunction() << " -8-> " << calleeName << '\n';
   };
   return true;
}

static inline bool
isTemplateInstance(clang::TemplateSpecializationKind kind) {
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
   return clang::isTemplateInstantiation(kind);
#else
   return kind != clang::TSK_Undeclared && kind != clang::TSK_ExplicitSpecialization;
#endif
}

bool
CallersAction::Visitor::isTemplate(clang::CXXRecordDecl* RD) const {
   const clang::ClassTemplateSpecializationDecl* TSD = NULL;
   if (RD->getKind() == clang::Decl::ClassTemplateSpecialization)
      TSD = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(RD);
   return (RD &&
         (((RD->getDescribedClassTemplate() && RD->getDescribedClassTemplate()->getTemplatedDecl() == RD)
         || (RD->getKind() == clang::Decl::ClassTemplatePartialSpecialization))
               && (!RD || !TSD || !isTemplateInstance(RD->getTemplateSpecializationKind()))));
}

bool
CallersAction::Visitor::TraverseFunctionDecl(clang::FunctionDecl* Decl) {
   if (Decl->getTemplatedKind() == clang::FunctionDecl::TK_FunctionTemplate)
      return true;
   if (Decl->getBuiltinID())
      return true;
   clang::DeclContext* context = Decl->getDeclContext();
   auto kind = context->getDeclKind();
   while (kind >= clang::Decl::firstRecord && kind <= clang::Decl::lastRecord) {
      if ((kind >= clang::Decl::firstCXXRecord && kind <= clang::Decl::lastCXXRecord)
            && isTemplate(static_cast<clang::CXXRecordDecl*>(context)))
         return true;
      context = static_cast<clang::RecordDecl*>(context)->getDeclContext();
      kind = context->getDeclKind();
   };
   return Parent::TraverseFunctionDecl(Decl);
}

bool
CallersAction::Visitor::TraverseVarDecl(clang::VarDecl* Decl) {
   clang::DeclContext* context = Decl->getDeclContext();
   auto kind = context->getDeclKind();
   while (kind >= clang::Decl::firstRecord && kind <= clang::Decl::lastRecord) {
      if ((kind >= clang::Decl::firstCXXRecord && kind <= clang::Decl::lastCXXRecord)
            && isTemplate(static_cast<clang::CXXRecordDecl*>(context)))
         return true;
      context = static_cast<clang::RecordDecl*>(context)->getDeclContext();
      kind = context->getDeclKind();
   };
   return Parent::TraverseVarDecl(Decl);
}

bool
CallersAction::Visitor::TraverseCXXMethodDecl(clang::CXXMethodDecl* Decl) {
   if (Decl->getTemplatedKind() == clang::FunctionDecl::TK_FunctionTemplate)
      return true;
   clang::DeclContext* context = Decl->getDeclContext();
   auto kind = context->getDeclKind();
   while (kind >= clang::Decl::firstRecord && kind <= clang::Decl::lastRecord) {
      if ((kind >= clang::Decl::firstCXXRecord && kind <= clang::Decl::lastCXXRecord)
            && isTemplate(static_cast<clang::CXXRecordDecl*>(context)))
         return true;
      context = static_cast<clang::RecordDecl*>(context)->getDeclContext();
      kind = context->getDeclKind();
   };
   return Parent::TraverseCXXMethodDecl(Decl);
}

bool
CallersAction::Visitor::TraverseCXXConstructorDecl(clang::CXXConstructorDecl* Decl) {
   if (Decl->getTemplatedKind() == clang::FunctionDecl::TK_FunctionTemplate)
      return true;
   clang::DeclContext* context = Decl->getDeclContext();
   auto kind = context->getDeclKind();
   while (kind >= clang::Decl::firstRecord && kind <= clang::Decl::lastRecord) {
      if ((kind >= clang::Decl::firstCXXRecord && kind <= clang::Decl::lastCXXRecord)
            && isTemplate(static_cast<clang::CXXRecordDecl*>(context)))
         return true;
      context = static_cast<clang::RecordDecl*>(context)->getDeclContext();
      kind = context->getDeclKind();
   };
   return Parent::TraverseCXXConstructorDecl(Decl);
}

bool
CallersAction::Visitor::TraverseCXXConversionDecl(clang::CXXConversionDecl* Decl) {
   if (Decl->getTemplatedKind() == clang::FunctionDecl::TK_FunctionTemplate)
      return true;
   clang::DeclContext* context = Decl->getDeclContext();
   auto kind = context->getDeclKind();
   while (kind >= clang::Decl::firstRecord && kind <= clang::Decl::lastRecord) {
      if ((kind >= clang::Decl::firstCXXRecord && kind <= clang::Decl::lastCXXRecord)
            && isTemplate(static_cast<clang::CXXRecordDecl*>(context)))
         return true;
      context = static_cast<clang::RecordDecl*>(context)->getDeclContext();
      kind = context->getDeclKind();
   };
   return Parent::TraverseCXXConversionDecl(Decl);
}

bool
CallersAction::Visitor::TraverseCXXDestructorDecl(clang::CXXDestructorDecl* Decl) {
   if (Decl->getTemplatedKind() == clang::FunctionDecl::TK_FunctionTemplate)
      return true;
   clang::DeclContext* context = Decl->getDeclContext();
   auto kind = context->getDeclKind();
   while (kind >= clang::Decl::firstRecord && kind <= clang::Decl::lastRecord) {
      if ((kind >= clang::Decl::firstCXXRecord && kind <= clang::Decl::lastCXXRecord)
            && isTemplate(static_cast<clang::CXXRecordDecl*>(context)))
         return true;
      context = static_cast<clang::RecordDecl*>(context)->getDeclContext();
      kind = context->getDeclKind();
   };
   return Parent::TraverseCXXDestructorDecl(Decl);
}

// add the function to the current json file only if it is really defined in this file
bool
// precondition: (function->isThisDeclarationADefinition()==true)
CallersAction::Visitor::VisitFunctionDefinition(clang::FunctionDecl* function) {

      std::string fct_sign = writeFunction(*function);
      std::string fct_filepath = printFilePath(function->getSourceRange());
      int fct_line = printLine(function->getSourceRange());

        // check the function virtuality
        auto methodDef = llvm::dyn_cast<clang::CXXMethodDecl>(function);
        auto virtualityDef = (methodDef && methodDef->isVirtual()) ?
          (methodDef->isPure() ? CallersData::VVirtualPure : CallersData::VVirtualDefined)
          : CallersData::VNoVirtual;

        MangledName fct_mangledName;
        this->getMangledName(mangle_context_, function, &fct_mangledName);

        std::string record = CALLERS_DEFAULT_NO_RECORD_NAME;
        if(methodDef != NULL)
        {
          auto rec = methodDef->getParent();
          if(rec != NULL)
          {
            record = rec->getQualifiedNameAsString();
          }
        }
        CallersData::FctDef fctDef(fct_mangledName, fct_sign, virtualityDef, fct_filepath, fct_line, record);

        // get the function declaration and check it's position
        clang::FunctionDecl* functionDecl = function->getCanonicalDecl();
        std::string fctDecl_file = printFilePath(functionDecl->getSourceRange());
        int fctDecl_line = printLine(functionDecl->getSourceRange());

        auto methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(functionDecl);
        auto virtualityDecl = (methodDecl && methodDecl->isVirtual()) ?
          (methodDecl->isPure() ? CallersData::VVirtualPure : CallersData::VVirtualDeclared)
          : CallersData::VNoVirtual;

        record = CALLERS_DEFAULT_NO_RECORD_NAME;
        if(methodDecl != NULL)
        {
          auto rec_decl = methodDecl->getParent();
          if(rec_decl != NULL)
          {
            record = rec_decl->getQualifiedNameAsString();
          }
        }
        CallersData::FctDecl fctDecl(fct_mangledName, fct_sign, virtualityDecl, fctDecl_file, fctDecl_line, record);

        // Complete the function definition with a new "declaration" entry

        osOut << "visiting function " << sParent
              << " defined at file " << fct_filepath << ':' << fct_line
              << " and declared at file " << fctDecl_file << ':' << fctDecl_line << std::endl;

        // check whether the function is really defined in this file
        if(fct_filepath == currentJsonFile->fullPath())
          // if true, add the function definition to the current json file
          {
            currentJsonFile->add_defined_function(&fctDef, fct_filepath);
          }
        else
          // otherwise, check whether a json file is already present for the visited defined function
          // if true, parse it and add the defined function only when necessary
          // if false, create this json file and add the defined function
          {
            boost::filesystem::path p(fct_filepath);
            std::string basename = p.filename().string();
            std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
            std::set<CallersData::File>::iterator file = otherJsonFiles.create_or_get_file(basename, dirpath);
            file->add_defined_function(&fctDef, fct_filepath);
          }

        // check whether the function is really declared in this file
        if(fctDecl_file == currentJsonFile->fullPath())
          // if true, add the function declaration to the current json file
          {
            currentJsonFile->add_declared_function(&fctDecl, fctDecl_file, &otherJsonFiles);
          }
        else
          // otherwise, check whether a json file is already present for the visited function
          // if true, parse it and add the defined function only when necessary
          // if false, create this json file and add the defined function
          {
            boost::filesystem::path p(fctDecl_file);
            std::string basename = p.filename().string();
            std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
            std::set<CallersData::File>::iterator file = otherJsonFiles.create_or_get_file(basename, dirpath);
            file->add_declared_function(&fctDecl, fctDecl_file, &otherJsonFiles);
          }
   return true;
}

bool
CallersAction::Visitor::VisitFunctionDeclaration(clang::FunctionDecl* function) {

      std::string fct_sign = writeFunction(*function);
      std::string fct_filepath = printFilePath(function->getSourceRange());
      int fct_line = printLine(function->getSourceRange());

        auto methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(function);
        auto virtualityDecl = (methodDecl && methodDecl->isVirtual()) ?
          (methodDecl->isPure() ? CallersData::VVirtualPure : CallersData::VVirtualDeclared)
          : CallersData::VNoVirtual;

		if (methodDecl->isVirtual()) {
		osOut << "visiting virtual declared method " << fct_sign
			  << " at " << fct_filepath << ':' << fct_line << '\n';
		  }
		  else if (methodDecl->isPure()) {
		osOut << "visiting virtual pure declared method " << fct_sign
			  << " at " << fct_filepath << ':' << fct_line << '\n';
		  }
		  else {
		osOut << "visiting non-virtual declared method " << fct_sign
			  << " at " << fct_filepath << ':' << fct_line << '\n';
		  }

        MangledName fct_mangledName;
        this->getMangledName(mangle_context_, function, &fct_mangledName);
        osOut << "Debug mangledName ok: " << fct_mangledName << '\n';

        std::string fct_record = CALLERS_DEFAULT_NO_RECORD_NAME;
        if((methodDecl != NULL) && (methodDecl->getParent() != NULL))
        {
          fct_record = methodDecl->getParent()->getQualifiedNameAsString();
        }

        CallersData::FctDecl fctDecl(fct_mangledName, fct_sign, virtualityDecl, fct_filepath, fct_line, fct_record);

        // check whether the function is really defined in this file
        if(fct_filepath == currentJsonFile->fullPath())
          // if true, add the function to the current json file
          {
            currentJsonFile->add_declared_function(&fctDecl, fct_filepath, &otherJsonFiles);
          }
        else
          // otherwise, check whether a json file is already present for the visited function
          // if true, parse it and add the defined function only when necessary
          // if false, create this json file and add the defined function
          {
            boost::filesystem::path p(fct_filepath);
            std::string basename = p.filename().string();
            std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
            std::set<CallersData::File>::iterator file = otherJsonFiles.create_or_get_file(basename, dirpath);
            file->add_declared_function(&fctDecl, fct_filepath, &otherJsonFiles);
          }
    return true;
}

// global variable:out: pfdParent
bool
CallersAction::Visitor::VisitFunctionDecl(clang::FunctionDecl* Decl) {

   pfdParent = Decl;
   std::string fct_filepath = printFilePath(Decl->getSourceRange());
   int fct_line = printLine(Decl->getSourceRange());
   sParent = writeFunction(*Decl);

   bool isDefinition = Decl->isThisDeclarationADefinition();
   auto methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(Decl);
   bool isDeclarationOfInterest = !isDefinition && methodDecl;
   // && (methodDecl->isVirtual() || methodDecl->isPure());

   if (isDefinition) {

    osOut << "visiting function \"" << sParent
           << "\" defined at " << fct_filepath << ':' << fct_line << std::endl;

     this->VisitFunctionDefinition(Decl);
   }
   else if (isDeclarationOfInterest) {

    osOut << "visiting method \"" << sParent
           << "\" declared at " << fct_filepath << ':' << fct_line << std::endl;

     this->VisitFunctionDeclaration(Decl);
   }
   else {
     osOut << "ignore function \"" << sParent
            << "\" declared at " << fct_filepath << ':' << fct_line << std::endl;
   }
   return true;
}

bool
CallersAction::Visitor::TraverseCXXRecordDecl(clang::CXXRecordDecl* Decl) {
   if (isTemplate(Decl))
      return true;
   clang::DeclContext* context = Decl->getDeclContext();
   auto kind = context->getDeclKind();
   while (kind >= clang::Decl::firstRecord && kind <= clang::Decl::lastRecord) {
      if ((kind >= clang::Decl::firstCXXRecord && kind <= clang::Decl::lastCXXRecord)
            && isTemplate(static_cast<clang::CXXRecordDecl*>(context)))
         return true;
      context = static_cast<clang::RecordDecl*>(context)->getDeclContext();
      kind = context->getDeclKind();
   };
   return Parent::TraverseCXXRecordDecl(Decl);
}

void
CallersAction::Visitor::VisitInheritanceList(clang::CXXRecordDecl* cxxDecl,
					     CallersData::Record* record)
{
   clang::CXXRecordDecl::base_class_iterator endBase = cxxDecl->bases_end();
   bool isFirst = true;
   for (clang::CXXRecordDecl::base_class_iterator
         iterBase = cxxDecl->bases_begin(); iterBase != endBase; ++iterBase)
   {
      const clang::Type* type = iterBase->getType().getTypePtr();
      const clang::CXXRecordDecl* base = type->getAsCXXRecordDecl();

      if (!base) {
         continue;
         // std::cerr << "Unsupported Inheritance Type:"
         //       << iterBase->getType().getAsString ()
         //       << "\nAborting\n";
         // exit(2);
      };
      if (!isFirst)
         osOut << ",\n";
      else
         isFirst = false;

      //std::string baseName = printQualifiedName(*base);
      std::string baseName = base->getQualifiedNameAsString();
      std::string baseFile = printFilePath(base->getSourceRange());
      int baseBegin = printLine(base->getLocStart());
      int baseEnd = printLine(base->getLocEnd());
      record->add_base_class(baseName, baseFile, baseBegin, baseEnd);

      osOut << baseName;
      osOut << printTemplateKind(*base);
   };
}

bool
CallersAction::Visitor::VisitRecordDecl(clang::RecordDecl* Decl) {
   clang::Decl::Kind declKind = Decl->getDeclContext()->getDeclKind();
   if ((declKind >= clang::Decl::firstFunction && declKind <= clang::Decl::lastFunction))
      return true;

   clang::CXXRecordDecl *RD = llvm::dyn_cast<clang::CXXRecordDecl>(Decl);
   if (Decl->isThisDeclarationADefinition() && RD && RD->isCompleteDefinition()) {
     clang::TagTypeKind tagKind = Decl->getTagKind();
      if (tagKind == clang::TTK_Struct || tagKind == clang::TTK_Class) { // avoid unions
         bool isAnonymousRecord = false;
         //std::string recordName = printQualifiedName(*Decl, &isAnonymousRecord);
         std::string recordName = Decl->getQualifiedNameAsString();
         std::string recordFile = printFilePath(Decl->getSourceRange());
         int recordBegin = printLine(Decl->getLocStart());
         int recordEnd = printLine(Decl->getLocEnd());

         recordName += printTemplateKind(*Decl);
         if (!isAnonymousRecord) {

            osOut << "visiting record " << recordName
                  << " at " << printLocation(Decl->getSourceRange()) << '\n';

            CallersData::Record record(recordName, tagKind, recordFile, recordBegin, recordEnd);

            osOut << " the record \"" << recordName << "\" inherits from ";
            VisitInheritanceList(RD, &record);
            osOut << std::endl;

            osOut << " the record \"" << recordName << "\" declares the following methods:" << std::endl;

            for(clang::CXXRecordDecl::method_iterator m = RD->method_begin();
                m != RD->method_end();
                m++)
            {
              clang::FunctionDecl* f = (clang::FunctionDecl*)(*m);
              std::string method_sign = writeFunction(*f);
              osOut << " - " << method_sign << std::endl;
              record.add_method(method_sign);
            }

            // check whether the record is really defined in this file
            if(recordFile == currentJsonFile->fullPath())
              // if true, add the record to the current json file
              {
                osOut << "the record \"" << recordName << "\" is well defined in current file \""
                      << recordFile << "\"" << std::endl;
                currentJsonFile->add_record(record);
              }
            else
              // otherwise, check whether a json file is already present for the visited record
              // if true, parse it and add the defined record only when necessary
              // if false, create this json file and add the defined record
              {
                osOut << "the record \"" << recordName
                      << "\" is not defined in current file \"" << currentJsonFile->fullPath()
                      << "\" but in file \"" << recordFile << "\"" << std::endl;

                boost::filesystem::path p(recordFile);
                std::string basename = p.filename().string();
                std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
                std::set<CallersData::File>::iterator file = otherJsonFiles.create_or_get_file(basename, dirpath);

                file->add_record(record);
              }

            osOut << '\n';
         };
      }
   };

   return true;
}
