/****
     Copyright (C) 2015
       - All Rights Reserved
     coded by Hugues Balp (Thales Communication & Security)
****/

//
// Description:
//   clang -> file containing called functions
//

//#include <stdlib.h> // abort()

#include <climits>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
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

  // Check whether the callers includes header files path does already exists or not in the filesystem
  if(!(boost::filesystem::exists(CALLERS_INCLUDES_FULL_DIR)))
  {
    std::cout << "Creating tmp directory: " << CALLERS_INCLUDES_FULL_DIR << std::endl;
    boost::filesystem::create_directories(CALLERS_INCLUDES_FULL_DIR);
  }

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
CallersAction::Visitor::printLocation(const clang::SourceRange& rangeLocation, std::string defaultFilePath) const {
   ASSERT(psSources);
   auto start = psSources->getPresumedLoc(rangeLocation.getBegin());
   auto end = psSources->getPresumedLoc(rangeLocation.getEnd());
   int startLine = 0;
   int endLine = 0;
   std::string loc = this->printFilePath(rangeLocation, defaultFilePath);
   if (loc != "<unknown>")
     {
       startLine = start.getLine();
       loc += ':';
       std::ostringstream start_pos;
       start_pos << startLine;
       loc += start_pos.str();
       endLine = end.getLine();
       loc += ':';
       std::ostringstream end_pos;
       end_pos << endLine;
       loc += end_pos.str();
     }
   else
     {
       loc += ":-2:-2";
     }
   return loc;
}

std::string
CallersAction::Visitor::printFileName(const clang::SourceRange& rangeLocation) const {
   ASSERT(psSources);
   auto start = psSources->getPresumedLoc(rangeLocation.getBegin());
   const char* startFile = start.getFilename();
   if (!startFile)
     startFile = "<unknown>";
   std::string result = startFile;
   return result;
}

std::string
CallersAction::Visitor::printFilePath(const clang::SourceRange& rangeLocation, std::string defaultFilePath) const {
   ASSERT(psSources);
   std::string path = defaultFilePath;
   if(rangeLocation.isValid())
   {
     auto start = psSources->getPresumedLoc(rangeLocation.getBegin());
     const char* startFile = start.getFilename();
     if (startFile)
       path = ::getCanonicalAbsolutePath(startFile);
   }
   return path;
}

std::string CallersAction::Visitor::printCurrentPath() const
{
  boost::filesystem::path current_path = boost::filesystem::current_path();
  std::string path = boost::filesystem::canonical(current_path).string();
  return path;
}

int
CallersAction::Visitor::getStartLine(const clang::SourceRange& rangeLocation) const {
   ASSERT(psSources);
   int startLine = CALLERS_NO_LOCATION_BEGIN;
   if(rangeLocation.isValid())
   {
     auto start = psSources->getPresumedLoc(rangeLocation.getBegin());
     startLine = start.getLine();
   }
   return startLine;
}

int
CallersAction::Visitor::getEndLine(const clang::SourceRange& rangeLocation) const {
   ASSERT(psSources);
   ASSERT(rangeLocation.isValid());
   auto end = psSources->getPresumedLoc(rangeLocation.getEnd());
   return end.getLine();
}

int
CallersAction::Visitor::getNbLines(const clang::SourceRange& rangeLocation) const {
   int nb_lines = this->getEndLine(rangeLocation) - this->getStartLine(rangeLocation);
   return nb_lines;
}

std::string
CallersAction::Visitor::printNumber(int number) const {
   std::ostringstream out;
   out << number;
   return out.str();
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
   ASSERT(llvm::dyn_cast<clang::CXXRecordDecl>(&decl));
   const clang::CXXRecordDecl *RD = llvm::dyn_cast<clang::CXXRecordDecl>(&decl);
   const clang::ClassTemplateSpecializationDecl* TSD = NULL;
   if (decl.getKind() == clang::Decl::ClassTemplateSpecialization) // do not handle partial specialization !
      TSD = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(&decl);
   if (RD && TSD /* RD->getTemplateSpecializationKind() >= clang::TSK_ImplicitInstantiation */) {
      ASSERT(llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(RD));
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
         ASSERT(llvm::dyn_cast<const clang::BuiltinType>(type));
         return printBuiltinType(static_cast<clang::BuiltinType const*>(type));
      case clang::Type::Enum:
         ASSERT(llvm::dyn_cast<const clang::EnumType>(type));
         return std::string("enum ") + static_cast<clang::EnumType const*>
            (type)->getDecl()->getName().str();
      case clang::Type::Auto:
         {  ASSERT(llvm::dyn_cast<const clang::AutoType>(type));
            const auto* autotype = static_cast<clang::AutoType const*>(type);
            return autotype->isDeduced() ? printArithmeticType(
                  autotype->getDeducedType().getTypePtr()) : std::string("");
         }
      case clang::Type::SubstTemplateTypeParm:
         {  ASSERT(llvm::dyn_cast<const clang::SubstTemplateTypeParmType>(type));
            const auto* replacementType = static_cast<clang::SubstTemplateTypeParmType const*>(type);
            return printArithmeticType(replacementType->getReplacementType().getTypePtr());
         };
      case clang::Type::TemplateSpecialization:
         {  ASSERT(llvm::dyn_cast<const clang::TemplateSpecializationType>(type));
            const auto* specializationType = static_cast<clang::TemplateSpecializationType const*>(type);
            return specializationType->isSugared()
               ? printArithmeticType(specializationType->desugar().getTypePtr())
               : std::string("");
         }
      case clang::Type::Typedef:
         {  ASSERT(llvm::dyn_cast<const clang::TypedefType>(type));
            const auto* replacementType = static_cast<clang::TypedefType const*>(type);
            return replacementType->isSugared()
               ? printArithmeticType(replacementType->desugar().getTypePtr())
               : std::string("");
         };
      case clang::Type::Elaborated:
         {  ASSERT(llvm::dyn_cast<const clang::ElaboratedType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::BuiltinType>(type));
            return printBuiltinType(static_cast<clang::BuiltinType const*>(type));
         };
      case clang::Type::Pointer:
         {  ASSERT(llvm::dyn_cast<const clang::PointerType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::LValueReferenceType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::MemberPointerType>(type));
            const auto* memberPointerType = static_cast<clang::MemberPointerType const*>(type);
            std::string className = printQualifiedName(*memberPointerType->getMostRecentCXXRecordDecl());
            if (memberPointerType->isMemberFunctionPointer()) {
               const auto* prototype = (memberPointerType->getPointeeType()
                     .getTypePtr()->getAs<clang::FunctionProtoType>());
               ASSERT(prototype);
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
         {  ASSERT(llvm::dyn_cast<const clang::ConstantArrayType>(type));
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
            {
               std::ostringstream out;
               out << arrayType->getSize().getLimitedValue(UINT64_MAX);
               subType += out.str();
               subType += ']';
            };
            return subType;
         };
      case clang::Type::IncompleteArray:
         {  ASSERT(llvm::dyn_cast<const clang::IncompleteArrayType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::FunctionProtoType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::RecordType>(type));
            const auto* recordType = static_cast<clang::RecordType const*>(type);
            return printQualifiedName(*recordType->getDecl());
         };
      case clang::Type::Enum:
         {  ASSERT(llvm::dyn_cast<const clang::EnumType>(type));
            const auto* enumType = static_cast<clang::EnumType const*>(type);
            return printQualifiedName(*enumType->getDecl());
         };
      case clang::Type::Auto:
         {  ASSERT(llvm::dyn_cast<const clang::AutoType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::SubstTemplateTypeParmType>(type));
            const auto* replacementType = static_cast<clang::SubstTemplateTypeParmType const*>(type);
            return printPlainType(replacementType->getReplacementType());
         };
      case clang::Type::TemplateSpecialization:
         {  ASSERT(llvm::dyn_cast<const clang::TemplateSpecializationType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::TypedefType>(type));
            const auto* replacementType = static_cast<clang::TypedefType const*>(type);
            return printPlainType(replacementType->getDecl()->getUnderlyingType());
         };
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
      case clang::Type::Decayed:
      case clang::Type::Adjusted:
         {  ASSERT(llvm::dyn_cast<const clang::AdjustedType>(type));
            const auto* adjustedType = static_cast<clang::AdjustedType const*>(type);
            return printPlainType(adjustedType->getOriginalType());
         };
#else
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_4
      // Adjusted was apparently introduced in 3.5, but Decayed already exists
      // in 3.4
      case clang::Type::Decayed:
         { ASSERT(llvm::dyn_cast<const clang::DecayedType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::ParenType>(type));
            const auto* parentype = static_cast<clang::ParenType const*>(type);
            return printPlainType(parentype->getInnerType());
         };
      case clang::Type::Elaborated:
         {  ASSERT(llvm::dyn_cast<const clang::ElaboratedType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::DecltypeType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::InjectedClassNameType>(type));
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
         {  ASSERT(llvm::dyn_cast<const clang::RecordType>(type));
            const auto* recordType = static_cast<clang::RecordType const*>(type);
            return printQualifiedName(*recordType->getDecl()) + printTemplateKind(*recordType->getDecl());
         }
      case clang::Type::LValueReference:
      case clang::Type::RValueReference:
         {  ASSERT(llvm::dyn_cast<const clang::ReferenceType>(type));
            const auto* referenceType = static_cast<clang::ReferenceType const*>(type);
            return printCompoundType(referenceType->getPointeeType().getTypePtr());
         }
      case clang::Type::Auto:
         {  ASSERT(llvm::dyn_cast<const clang::AutoType>(type));
            const auto* autotype = static_cast<clang::AutoType const*>(type);
            return autotype->isDeduced()
                  ? printCompoundType(autotype->getDeducedType().getTypePtr())
                  : std::string("");
         }
      case clang::Type::SubstTemplateTypeParm:
         {  ASSERT(llvm::dyn_cast<const clang::SubstTemplateTypeParmType>(type));
            const auto* replacementType = static_cast<clang::SubstTemplateTypeParmType const*>(type);
            return printCompoundType(replacementType->getReplacementType().getTypePtr());
         };
      case clang::Type::TemplateSpecialization:
         {  ASSERT(llvm::dyn_cast<const clang::TemplateSpecializationType>(type));
            const auto* specializationType = static_cast<clang::TemplateSpecializationType const*>(type);
            return specializationType->isSugared()
               ? printCompoundType(specializationType->desugar().getTypePtr())
               : std::string("");
         }
      case clang::Type::Typedef:
         {  ASSERT(llvm::dyn_cast<const clang::TypedefType>(type));
            const auto* replacementType = static_cast<clang::TypedefType const*>(type);
            return replacementType->isSugared()
               ? printCompoundType(replacementType->desugar().getTypePtr())
               : std::string("");
         };
      case clang::Type::Elaborated:
         {  ASSERT(llvm::dyn_cast<const clang::ElaboratedType>(type));
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
      //std::set<CallersData::Namespace>::iterator namespc = (*file)->get_or_create_namespace(qualifiers, *nspc);
      // CallersData::Namespace c_namespace(qualifiers, *nspc);
      // std::cerr << "nspc: " << c_namespace.name << ", qualifiers: " << qualifiers << std::endl;
      // (*file)->add_namespace(c_namespace);
      if (result.length() > 0) {
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
  return CALLERS_DEFAULT_NO_QUALIFICATION_NAME;
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
  return CALLERS_DEFAULT_NO_QUALIFICATION_NAME;
}

std::string
CallersAction::Visitor::printNamespaces(const clang::NamedDecl& namedDecl) const {
  const clang::DeclContext *context = namedDecl.getDeclContext();
  if (context) {
    std::string result = printQualification(context->getParent());
    const clang::Decl::Kind kind = context->getDeclKind();
    if (kind == clang::Decl::Namespace)
      {
	// if (result.length() > 0)
	//   result += "::";
	// else
        result += "::";

	result += static_cast<const clang::NamespaceDecl*>(context)->getName().str();
      }
    return result;
  };
  return CALLERS_DEFAULT_NO_NAMESPACE_NAME;
}

std::string
CallersAction::Visitor::printRootQualification(const clang::DeclContext* context) const {
  if (context) {
    std::string result = printRootQualification(context->getParent());
    if (result.length() > 0) {
      return result;
    }
    const clang::Decl::Kind kind = context->getDeclKind();
    if (kind == clang::Decl::Namespace)
      {
	result = static_cast<const clang::NamespaceDecl*>(context)->getName().str();
      }
    else if (kind >= clang::Decl::firstTag && kind <= clang::Decl::lastTag) {
      result = static_cast<const clang::TagDecl*>(context)->getName().str();
      if (kind >= clang::Decl::firstCXXRecord && kind <= clang::Decl::lastCXXRecord)
	result += printTemplateKind(*static_cast<const clang::CXXRecordDecl*>(context));
    };
    return result;
  };
  return CALLERS_DEFAULT_NO_QUALIFICATION_NAME;
}

std::string
CallersAction::Visitor::printRootNamespace(const clang::NamedDecl& namedDecl,
                                           std::string defaultRootNamespace /*= CALLERS_DEFAULT_NO_NAMESPACE_NAME*/,
                                           std::string recordName) const {

  std::string nspc = CALLERS_DEFAULT_NO_NAMESPACE_NAME;
  const clang::DeclContext *context = namedDecl.getDeclContext();
  if (context) {
    std::string nsp = printRootQualification(context->getParent());
    if (nsp == CALLERS_DEFAULT_NO_QUALIFICATION_NAME) {
      const clang::Decl::Kind kind = context->getDeclKind();
      if (kind == clang::Decl::Namespace)
        {
          nsp = static_cast<const clang::NamespaceDecl*>(context)->getName().str();
        }
      // BEGIN BAD IDEA
      // else if (kind == clang::Decl::CXXRecord)
      //   {
      //     nsp = static_cast<const clang::CXXRecordDecl*>(context)->getName().str();
      //   }
      // END BAD IDEA
    }
    if(nsp != CALLERS_UNSUPPORTED_EMPTY_NAMESPACE)
    {
      return nsp;
    }
  }
  if(defaultRootNamespace != CALLERS_UNSUPPORTED_EMPTY_NAMESPACE)
  {
    std::string root_namespace, namespaces;
    /* bool has_namespace = */ CallersData::get_namespaces(defaultRootNamespace, nspc, namespaces, recordName);
  }

  ASSERT(nspc != CALLERS_UNSUPPORTED_EMPTY_NAMESPACE);
  // currentJsonFile->get_or_create_namespace(nspc);
  return nspc;
}

std::string
CallersAction::Visitor::printRecordName(const clang::CXXRecordDecl* record) const {

  std::string recordName = CALLERS_DEFAULT_RECORD_NAME;

  // returns only the identifier without the template arguments
  // recordName = record->getQualifiedNameAsString();

  // recordFullNameWithTemplateArgs
  //  llvm::raw_string_ostream os(recordName);
  //  record->getNameForDiagnostic(os, ciCompilerInstance.getASTContext().getPrintingPolicy(), // Qualified
  // true);
  //  recordName = os.str();

  recordName = this->printQualifiedName(*record);
  if(recordName == CALLERS_DEFAULT_RECORD_NAME)
  {
    // c'est le cas des record anonymes
    recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
  }
  // ASSERT(recordName != CALLERS_DEFAULT_RECORD_NAME);
  return recordName;
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
  std::string pureName = namedDecl.getNameAsString();
  if((result.length() > 0) && (pureName.length() > 0))
     result += "::";
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
  ASSERT(nd != NULL);
  ASSERT(ctx != NULL);

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
   ASSERT(llvm::dyn_cast<clang::FunctionDecl>(constr));
   const auto& function = *static_cast<const clang ::FunctionDecl*>(constr);
   if(this->isDeclarationOfInterest(function))
   {
     std::string name = printQualifiedName(function);
     MangledName callee_decl_mangledName;
     this->getMangledName(mangle_context_, &function, &callee_decl_mangledName);
     if (name.length() == 0)
        name = "constructor-special";
     std::string result = name;
     result += printTemplateKind(function);
     result += printArgumentSignature(function);
     osOut << inputFile << ": " << printParentFunction() << " -1-> " << result << '\n';

     ASSERT(pfdParent != NULL);
     auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);

     std::string caller_def_sign = printParentFunction();
     std::string caller_def_file = printParentFunctionFilePath();
     int caller_def_begin = getStartLine(constr->getSourceRange());
     int caller_def_end = getEndLine(constr->getSourceRange());

     std::string caller_decl_file = printLocation(pfdParent->getCanonicalDecl()->getSourceRange(), caller_def_file);
     CallersData::Virtuality caller_def_virtuality = (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual;
     if(constr != NULL)
     {
       // get the function declaration and check it's position
       const clang::FunctionDecl* caller_decl = constr->getCanonicalDecl();
       caller_decl_file = printLocation(caller_decl->getSourceRange(), caller_def_file);
     }
     MangledName caller_def_mangledName;
     this->getMangledName(mangle_context_, pfdParent, &caller_def_mangledName);

     std::string caller_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
     std::string caller_recordFilePath = caller_def_file;
     if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
     {
       caller_recordName = printRecordName(parentMethod->getParent());
       caller_recordFilePath = printFilePath(parentMethod->getParent()->getSourceRange(), caller_def_file);
     }
     std::string caller_def_nspc = printRootNamespace(*pfdParent, printQualifiedName(*pfdParent), caller_recordName);

     std::string callee_decl_sign = writeFunction(function);
     std::string callee_decl_file = printFilePath(function.getSourceRange(), caller_def_file);
     int callee_decl_begin = getStartLine(function.getSourceRange());
     int callee_decl_end = getEndLine(function.getSourceRange());

     std::string callee_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
     std::string callee_recordFilePath = callee_decl_file;
     if((constr != NULL) && (constr->getParent() != NULL))
     {
       callee_recordName = printRecordName(constr->getParent());
       callee_recordFilePath = printFilePath(constr->getParent()->getSourceRange(), callee_decl_file);
     }
     std::string callee_decl_nspc = printRootNamespace(function, name, callee_recordName);

     CallersData::Virtuality callee_decl_virtuality = caller_def_virtuality;

     CallersData::FctDef caller(caller_def_mangledName, caller_def_sign, caller_def_virtuality, caller_def_nspc, caller_def_file,
                                caller_def_begin, caller_def_end, caller_decl_file, caller_recordName, caller_recordFilePath);

     CallersData::FctDecl callee(callee_decl_mangledName, callee_decl_sign, callee_decl_virtuality, callee_decl_nspc, callee_decl_file,
                                 callee_decl_begin, callee_decl_end,
                                 callee_recordName, callee_recordFilePath);

     VisitFunctionParameters(*constr, callee);

     CallersData::FctCall fc(caller, callee);

     currentJsonFile->add_function_call(&fc, &otherJsonFiles);
  }
  return true;
}
bool
CallersAction::Visitor::VisitCXXDeleteExpr(const clang::CXXDeleteExpr* deleteExpr) {
   if (deleteExpr->getOperatorDelete()
         && !deleteExpr->getOperatorDelete()->isImplicit()) {
      const clang::FunctionDecl& function = *deleteExpr->getOperatorDelete();
     if(this->isDeclarationOfInterest(function))
     {
        std::string callee_sign = printResultSignature(function);
        MangledName fct_mangledName;
        this->getMangledName(mangle_context_, &function, &fct_mangledName);
        callee_sign += ' ';
        callee_sign += printQualifiedName(function);
        callee_sign += printArgumentSignature(function);
        osOut << inputFile << ": " << printParentFunction() << " -2-> " << callee_sign << '\n';
        std::string callee_filepath = printFilePath(function.getSourceRange());
        int callee_begin = getStartLine(function.getSourceRange());
        int callee_end = getEndLine(function.getSourceRange());
        // if(callee_filepath == CALLERS_NO_FILE_PATH)
        //   {
        //     osOut << inputFile << ":WARNING: unknownFilePath for callee: " << callee_sign << std::endl;
        //     callee_begin = CALLERS_NO_NB_LINES;
        //     callee_end = CALLERS_NO_NB_LINES;
        //   }

        auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);

        std::string caller_def_file = printParentFunctionFilePath();
        std::string caller_decl_file = printLocation(pfdParent->getCanonicalDecl()->getSourceRange(), caller_def_file);
        int caller_def_begin = getStartLine(pfdParent->getSourceRange());
        int caller_def_end = getEndLine(pfdParent->getSourceRange());

        if(parentMethod != NULL)
        {
          caller_def_begin = getStartLine(parentMethod->getSourceRange());
          caller_def_end = getEndLine(parentMethod->getSourceRange());

          // get the function declaration and check it's position
          const clang::FunctionDecl* caller_decl = parentMethod->getCanonicalDecl();
          caller_decl_file = printLocation(caller_decl->getSourceRange(), caller_def_file);
        }

        std::string caller_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
        std::string caller_recordFilePath = caller_decl_file;
        if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
        {
          caller_recordName = printRecordName(parentMethod->getParent());
          caller_recordFilePath = printFilePath(parentMethod->getParent()->getSourceRange(), caller_decl_file);
        }
        std::string caller_nspc = printRootNamespace(*pfdParent, printQualifiedName(*pfdParent), caller_recordName);

        auto calleeMethod = llvm::dyn_cast<clang::CXXMethodDecl>(&function);
        std::string callee_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
        std::string callee_recordFilePath = callee_filepath;
        if((calleeMethod != NULL) && (calleeMethod->getParent() != NULL))
        {
          callee_recordName = printRecordName(calleeMethod->getParent());
          callee_recordFilePath = printFilePath(calleeMethod->getParent()->getSourceRange(), callee_filepath);
        }
        std::string callee_nspc = printRootNamespace(function, printQualifiedName(function), callee_recordName);

        CallersData::FctDef caller(fct_mangledName, printParentFunction(),
                                   (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual,
                                   caller_nspc, caller_def_file,
                                   caller_def_begin, caller_def_end, caller_decl_file,
                                   caller_recordName, caller_recordFilePath);

        CallersData::FctDecl callee(fct_mangledName, callee_sign,
                                    (!calleeMethod || !calleeMethod->isVirtual()) ? CallersData::VNoVirtual
                                    : (calleeMethod->isPure() ? CallersData::VVirtualPure
                                       : (calleeMethod->isThisDeclarationADefinition() ? CallersData::VVirtualDefined : CallersData::VVirtualDeclared)),
                                    callee_nspc, callee_filepath, callee_begin, callee_end, callee_recordName, callee_recordFilePath);

        VisitFunctionParameters(function, callee);

        CallersData::FctCall fc(caller, callee);

        currentJsonFile->add_function_call(&fc, &otherJsonFiles);
        return true;
     }
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
      if (destructor && this->isDeclarationOfInterest(*destructor))
      {
         std::string callee_sign = printQualifiedName(*destructor);
         callee_sign += "()";
         osOut << inputFile << ": " << printParentFunction() << " -3-> " << callee_sign << '\n';
	 std::string callee_filepath = printFilePath(destructor->getSourceRange());
	 int callee_decl_begin = getStartLine(destructor->getSourceRange());
	 int callee_decl_end = getEndLine(destructor->getSourceRange());
	 // if(callee_filepath == CALLERS_NO_FILE_PATH)
	 //   {
	 //     osOut << inputFile << ":WARNING: unknownFilePath for callee: " << callee_sign << std::endl;
	 //     callee_decl_begin = callee_decl_end = CALLERS_NO_NB_LINES;
	 //   }

         std::string caller_def_file = printParentFunctionFilePath();

         int caller_def_begin = getStartLine(pfdParent->getSourceRange());
         int caller_def_end = getEndLine(pfdParent->getSourceRange());

         std::string caller_decl_file = printLocation(pfdParent->getCanonicalDecl()->getSourceRange(), caller_def_file);

         if(destructor != NULL)
         {
           // get the function declaration and check it's position
           const clang::FunctionDecl* caller_decl = destructor->getCanonicalDecl();
           caller_decl_file = printLocation(caller_decl->getSourceRange(), caller_def_file);
         }

         std::string caller_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
         std::string caller_recordFilePath = caller_decl_file;
         if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
         {
           caller_recordName = printRecordName(parentMethod->getParent());
           caller_recordFilePath = printFilePath(parentMethod->getParent()->getSourceRange(), caller_decl_file);
         }
         std::string caller_nspc = printRootNamespace(*pfdParent, printQualifiedName(*pfdParent), caller_recordName);

         std::string callee_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
         std::string callee_recordFilePath = callee_filepath;
         if((destructor != NULL) && (destructor->getParent() != NULL))
         {
           callee_recordName = printRecordName(destructor->getParent());
           callee_recordFilePath = printFilePath(destructor->getParent()->getSourceRange(), callee_filepath);
         }
         std::string callee_nspc = printRootNamespace(*destructor, callee_sign, callee_recordName);

	 CallersData::FctDef caller(fct_mangledName, printParentFunction(),
                                    (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual, caller_nspc,
                                    caller_def_file, caller_def_begin, caller_def_end, caller_decl_file,
                                    caller_recordName, caller_recordFilePath);

         CallersData::FctDecl callee(fct_mangledName, callee_sign,
                                     (!calleeMethod || !calleeMethod->isVirtual()) ? CallersData::VNoVirtual
                                     : (calleeMethod->isPure() ? CallersData::VVirtualPure
                                     : (calleeMethod->isThisDeclarationADefinition() ? CallersData::VVirtualDefined : CallersData::VVirtualDeclared)), callee_nspc,
                                     callee_filepath, callee_decl_begin, callee_decl_end,
                                     callee_recordName, callee_recordFilePath);

         VisitFunctionParameters(*destructor, callee);

	 CallersData::FctCall fc(caller, callee);

	 currentJsonFile->add_function_call(&fc, &otherJsonFiles);
      }
   };
   return true;
}

bool
CallersAction::Visitor::VisitCXXNewExpr(const clang::CXXNewExpr* newExpr) {
  auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);
  if (newExpr->getOperatorNew() && !newExpr->getOperatorNew()->isImplicit())
    {
      const clang::FunctionDecl& operatorNew = *newExpr->getOperatorNew();
      if(this->isDeclarationOfInterest(operatorNew))
      {
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
        int callee_decl_begin = getStartLine(operatorNew.getSourceRange());
        int callee_decl_end = getStartLine(operatorNew.getSourceRange());
        if(callee_filepath == CALLERS_NO_FILE_PATH)
          {
            osOut << inputFile << ":WARNING: unknownFilePath for callee: " << callee_sign << std::endl;
            callee_decl_begin = callee_decl_end = CALLERS_NO_NB_LINES;
          }
        std::string callee_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
        std::string callee_recordFilePath = callee_filepath;
        auto callee_rec = llvm::dyn_cast<clang::CXXMethodDecl>(&operatorNew);
        if((callee_rec != NULL) && (callee_rec->getParent() != NULL))
        {
          callee_recordName = printRecordName(callee_rec->getParent());
          callee_recordFilePath = printFilePath(callee_rec->getSourceRange(), callee_filepath);
        }
        std::string callee_nspc = printRootNamespace(operatorNew, printQualifiedName(operatorNew), callee_recordName);

        std::string caller_def_file = printParentFunctionFilePath();

        std::string caller_decl_file = printLocation(pfdParent->getCanonicalDecl()->getSourceRange(), caller_def_file);

        int caller_def_begin = getStartLine(pfdParent->getSourceRange());
        int caller_def_end = getEndLine(pfdParent->getSourceRange());

        if(parentMethod != NULL)
        {
          // get the function declaration and check it's position
          const clang::FunctionDecl* caller_decl = parentMethod->getCanonicalDecl();
          caller_decl_file = printLocation(caller_decl->getSourceRange(), caller_def_file);
        }

        std::string caller_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
        std::string caller_recordFilePath = caller_decl_file;
        if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
        {
          caller_recordName = printRecordName(parentMethod->getParent());
          caller_recordFilePath = printFilePath(parentMethod->getParent()->getSourceRange(), caller_decl_file);
        }
        std::string caller_nspc = printRootNamespace(*pfdParent, printQualifiedName(*pfdParent), caller_recordName);

        CallersData::FctDef caller(fct_mangledName, printParentFunction(),
                                   (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual,
                                   caller_nspc, printParentFunctionFilePath(),
                                   caller_def_begin, caller_def_end, caller_decl_file,
                                   caller_recordName, caller_recordFilePath);

        CallersData::FctDecl callee(fct_mangledName, callee_sign, CallersData::VNoVirtual, callee_nspc, callee_filepath,
                                    callee_decl_begin, callee_decl_end, callee_recordName, callee_recordFilePath);

        VisitFunctionParameters(operatorNew, callee);

        CallersData::FctCall fc(caller, callee);

        currentJsonFile->add_function_call(&fc, &otherJsonFiles);
      }
    }
  else
    {
      std::string malloc_sign = "malloc";
      //std::string malloc_sign = "void *malloc(size_t)";
      std::cout << "WARNING VisitCXXNewExpr: implicit operator new, replace it by an explicit call to \"" << malloc_sign << "\"" << std::endl;
      osOut << inputFile << ": " << printParentFunction() << " -5-> " << malloc_sign << '\n';
      MangledName malloc_mangled = "builtin_malloc";
      std::string malloc_filepath = "/usr/include/malloc.h";
      int malloc_begin = CALLERS_NO_NB_LINES;
      int malloc_end = CALLERS_NO_NB_LINES;
      if(parentMethod != NULL)
      {
        malloc_begin = getStartLine(parentMethod->getSourceRange()); // incorrect but welformed value
        malloc_end = getEndLine(parentMethod->getSourceRange()); // incorrect but welformed value
      }
      std::string malloc_recordName = CALLERS_DEFAULT_BUILTIN_RECORD_NAME;
      std::string malloc_recordFilePath = malloc_filepath;
      //int malloc_filepos = CALLERS_NO_NB_LINES; //unknown_position

      // this bloc is inspired from function CallersAction::Visitor::VisitBuiltinFunction
      // It checks whether a json file is already present for the malloc builtin function
      // if true, parse it and add the defined function only when necessary
      // if false, create this json file and add the defined function
      {
	boost::filesystem::path p(malloc_filepath);
	std::string basename = p.filename().string();
	std::string dirpath = ::getCanonicalAbsolutePath(p.parent_path().string());
	std::set<CallersData::File>::iterator file = otherJsonFiles.create_or_get_file(basename, dirpath);
        std::string malloc_nspc = CALLERS_DEFAULT_BUILTIN_NAMESPACE;
	CallersData::FctDecl fct(malloc_mangled, malloc_sign, CallersData::VNoVirtual, malloc_nspc, malloc_filepath,
                                 malloc_begin, malloc_end, malloc_recordName, malloc_recordFilePath, true);
	file->get_or_create_declared_function(&fct, malloc_filepath, &otherJsonFiles);
      }

      std::string caller_def_file = printParentFunctionFilePath();
      int caller_def_begin = getStartLine(pfdParent->getSourceRange());
      int caller_def_end = getEndLine(pfdParent->getSourceRange());

      std::string caller_decl_file = printLocation(pfdParent->getCanonicalDecl()->getSourceRange(), caller_def_file);
      if(parentMethod != NULL)
      {
        // get the function declaration and check it's position
        const clang::FunctionDecl* caller_decl = parentMethod->getCanonicalDecl();
        caller_decl_file = printLocation(caller_decl->getSourceRange(), caller_def_file);
      }

      std::string caller_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
      std::string caller_recordFilePath = caller_decl_file;
      if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
      {
        caller_recordName = printRecordName(parentMethod->getParent());
        caller_recordFilePath = printFilePath(parentMethod->getParent()->getSourceRange(), caller_decl_file);
      }
      std::string caller_nspc = printRootNamespace(*pfdParent, printQualifiedName(*pfdParent), caller_recordName);
      CallersData::FctDef caller(malloc_mangled, printParentFunction(),
                                 (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual, caller_nspc,
                                 caller_def_file, caller_def_begin, caller_def_end, caller_decl_file,
                                 caller_recordName, caller_recordFilePath);
      bool is_builtin = true;
      std::string malloc_nspc = CALLERS_DEFAULT_BUILTIN_NAMESPACE;
      CallersData::FctDecl callee(malloc_mangled, malloc_sign, CallersData::VNoVirtual, malloc_nspc, malloc_filepath,
                                  malloc_begin, malloc_end, malloc_recordName, malloc_recordFilePath, is_builtin);

      CallersData::FctCall fc(caller, callee);
      currentJsonFile->add_function_call(&fc, &otherJsonFiles);
    }
   return true;
}

bool
CallersAction::Visitor::VisitBuiltinFunction(const clang::FunctionDecl* fd) {

  if(this->isDeclarationOfInterest(*fd))
  {
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
    int builtin_begin = getStartLine(fd->getSourceRange());
    int builtin_end = getEndLine(fd->getSourceRange());
    std::string builtinRecordName = CALLERS_DEFAULT_BUILTIN_RECORD_NAME;
    std::string builtinRecordFilePath = builtinFile;
    MangledName builtin_mangled;
    this->getMangledName(mangle_context_, fd, &builtin_mangled);
    int builtinPos = getStartLine(fd->getSourceRange());
    std::cout << "DEBUG: builtin name: \"" << builtinName << "\"" << std::endl;
    std::cout << "DEBUG: builtin decl location: " << builtinFile << ":" << builtinPos << std::endl;
    std::cout << "DEBUG: builtin def location (headerName): " << headerName << std::endl;
    osOut << inputFile << ":builtin: " << printParentFunction() << " -6-> " << builtinName << ", defined in: " << headerName << ":" << builtinPos << std::endl;
    if(headerName == "notFoundBuiltinImpl")
      {
        std::cout << "WARNING: visitor.cpp : not found implementation of builtin: \"" << builtinName << "\", headerName: \"" << headerName << "\"" << std::endl;
        headerName = builtinFile;
      }
    else
      {
        ASSERT(builtinName != "notFoundBuiltinName");
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
	bool *IsMapped = NULL;
        bool skipCache = false;

	const clang::FileEntry * result = ciCompilerInstance.getPreprocessor().LookupFile(FilenameLoc, Filename, isAngled, FromDir, FromFile, CurDir, SearchPath, RelativePath, SuggestedModule, IsMapped, skipCache);
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

      auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);
      std::string caller_decl_file = printLocation(pfdParent->getCanonicalDecl()->getSourceRange(), headerName);
      int caller_def_begin = getStartLine(pfdParent->getSourceRange());
      int caller_def_end = getEndLine(pfdParent->getSourceRange());
      if(parentMethod != NULL)
      {
        // get the function declaration and check it's position
        const clang::FunctionDecl* caller_decl = parentMethod->getCanonicalDecl();
        caller_decl_file = printLocation(caller_decl->getSourceRange(), headerName);
      }

      std::string caller_recordName(CALLERS_DEFAULT_NO_RECORD_NAME);
      std::string caller_recordFilePath(caller_decl_file);
      if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
      {
        caller_recordName = printRecordName(parentMethod->getParent());
        caller_recordFilePath = printFilePath(parentMethod->getParent()->getSourceRange(), caller_decl_file);
      }
      std::string caller_nspc = printRootNamespace(*pfdParent, printQualifiedName(*pfdParent), caller_recordName);
      CallersData::FctDef caller(builtin_mangled, printParentFunction(),
                                 (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual, caller_nspc,
                                 printParentFunctionFilePath(),
                                 caller_def_begin, caller_def_end, caller_decl_file,
                                 caller_recordName, caller_recordFilePath);
      bool is_builtin = true;

      std::string builtin_nspc = CALLERS_DEFAULT_BUILTIN_NAMESPACE;
      CallersData::FctDecl callee(builtin_mangled, builtinName, CallersData::VNoVirtual, builtin_nspc, headerName,
                                  builtin_begin, builtin_end, builtinRecordName, builtinRecordFilePath, is_builtin);
      CallersData::FctCall fc(caller, callee);
      currentJsonFile->add_function_call(&fc, &otherJsonFiles);
    }
  }
  return true;
}

bool
CallersAction::Visitor::VisitCallExpr(const clang::CallExpr* callExpr) {
   const clang::FunctionDecl* callee = callExpr->getDirectCallee();
   std::string caller_sign = printParentFunction();
   if (callee && this->isDeclarationOfInterest(*callee)) {
      MangledName caller_mangled, callee_mangled;
      this->getMangledName(mangle_context_, pfdParent, &caller_mangled);

      auto parentMethod = llvm::dyn_cast<clang::CXXMethodDecl>(pfdParent);

      std::string caller_def_file = printParentFunctionFilePath();
      CallersData::Virtuality caller_virtuality = (parentMethod && parentMethod->isVirtual()) ? CallersData::VVirtualDefined : CallersData::VNoVirtual;
      std::string caller_decl_filepath = printFilePath(pfdParent->getCanonicalDecl()->getSourceRange(), caller_def_file);
      std::string caller_decl_filepos = printLocation(pfdParent->getCanonicalDecl()->getSourceRange(), caller_def_file);
      int caller_def_begin = getStartLine(pfdParent->getSourceRange());
      int caller_def_end = getEndLine(pfdParent->getSourceRange());
      if(parentMethod != NULL)
      {
        // get the function declaration and check it's position
        const clang::FunctionDecl* caller_decl = parentMethod->getCanonicalDecl();
        caller_decl_filepos = printLocation(caller_decl->getSourceRange(), caller_def_file);
      }

      std::string caller_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
      std::string caller_recordFilePath = caller_decl_filepath;
      if((parentMethod != NULL) && (parentMethod->getParent() != NULL))
      {
        caller_recordName = printRecordName(parentMethod->getParent());
        caller_recordFilePath = printFilePath(parentMethod->getParent()->getSourceRange(), caller_decl_filepath);
      }
      std::string caller_nspc = printRootNamespace(*pfdParent, printQualifiedName(*pfdParent), caller_recordName);

      if (callee->getBuiltinID() > 0)
       {
	  this->VisitBuiltinFunction(callee);
	  return true;
       }
      if (callee->isThisDeclarationADefinition())
       {
         callee = callee->getCanonicalDecl();
       }
      std::string callee_sign = writeFunction(*callee);
      std::string callee_name = callee->getNameAsString();
      osOut << inputFile << ": " << caller_sign << ":" << caller_mangled << " -7-> " << callee_name << ":" << callee_sign << '\n';
      this->getMangledName(mangle_context_, callee, &callee_mangled);
      ASSERT(callee->getSourceRange().isValid());
      std::string callee_decl_file = printFilePath(callee->getSourceRange(), caller_def_file);
      int callee_decl_begin = getStartLine(callee->getSourceRange());
      int callee_decl_end = getEndLine(callee->getSourceRange());
      if(callee_decl_file == CALLERS_NO_FILE_PATH)
	{
	  callee_decl_begin = callee_decl_end = CALLERS_NO_NB_LINES;
	}
      std::string callee_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
      std::string callee_recordFilePath = callee_decl_file;

      auto calleeMethod = llvm::dyn_cast<clang::CXXMethodDecl>(callee);
      if((calleeMethod != NULL) && (calleeMethod->getParent() != NULL))
      {
        callee_recordName = printRecordName(calleeMethod->getParent());
        callee_recordFilePath = printFilePath(calleeMethod->getParent()->getSourceRange(), callee_decl_file);
      }
      std::string callee_nspc = printRootNamespace(*callee, printQualifiedName(*callee), callee_recordName);

      CallersData::Virtuality callee_virtuality = (!calleeMethod || !calleeMethod->isVirtual()) ? CallersData::VNoVirtual
           : (calleeMethod->isPure() ? CallersData::VVirtualPure
           : (calleeMethod->isThisDeclarationADefinition() ? CallersData::VVirtualDefined : CallersData::VVirtualDeclared));

      CallersData::FctDef caller_def(caller_mangled, caller_sign, caller_virtuality, caller_nspc, caller_def_file,
                                     caller_def_begin, caller_def_end, caller_decl_filepos,
                                     caller_recordName, caller_recordFilePath);

      CallersData::FctDecl callee_decl(callee_mangled, callee_sign, callee_virtuality, callee_nspc, callee_decl_file,
                                       callee_decl_begin, callee_decl_end, callee_recordName, callee_recordFilePath);
      VisitFunctionParameters(*callee, callee_decl);

      CallersData::FctCall fc(caller_def, callee_decl);
      currentJsonFile->add_function_call(&fc, &otherJsonFiles);

      // Specific add-ons for particular builtin callees

      // case of thread creation
      if(callee_name == "pthread_create")
      {
        std::string thr_inst_name = "unknownThreadInstanceVarName";
        std::string thr_routine_name = "unknownThreadRoutineName";
        std::string thr_routine_sign = "unknownThreadRoutineSign";
        std::string thr_routine_mangled = "unknownThreadRoutineMangled";
        // Virtuality thr_routine_virtuality = "unknownThreadRoutineDeclVirtuality";
        CallersData::Virtuality thr_routine_virtuality = CallersData::VNoVirtual;
        std::string thr_routine_file = "unknownThreadRoutineDeclFile";
        int thr_routine_begin = CALLERS_NO_NB_LINES;
        int thr_routine_end = CALLERS_NO_NB_LINES;
        std::string thr_routine_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
        std::string thr_routine_recordFilePath = thr_routine_file;
        std::string thr_create_location = "unknownThreadCreateLocation";

        auto thr_routine_method = llvm::dyn_cast<clang::CXXMethodDecl>(callee);
        thr_routine_virtuality =
          (!thr_routine_method || !thr_routine_method->isVirtual()) ? CallersData::VNoVirtual
          : (thr_routine_method->isPure() ? CallersData::VVirtualPure
          : (thr_routine_method->isThisDeclarationADefinition() ? CallersData::VVirtualDefined : CallersData::VVirtualDeclared));
        auto argIter = callExpr->arg_begin(),
        argIterEnd = callExpr->arg_end();
        osOut << "THREAD:";
        for (; argIter != argIterEnd; ++argIter) {
          osOut << " debug";
          if(auto ICE = llvm::dyn_cast<clang::ImplicitCastExpr>(*argIter)) {
            osOut << " ice";
            if(auto DRE = llvm::dyn_cast<clang::DeclRefExpr>(ICE->getSubExpr())) {
              // it's a reference to a declaration
              if(auto FD = llvm::dyn_cast<clang::FunctionDecl>(DRE->getDecl())) {
                // it's a reference to a function
                thr_routine_name = FD->getNameAsString();
                thr_routine_sign = writeFunction(*FD);
                thr_routine_file = printFilePath(FD->getSourceRange());
                thr_routine_begin = getStartLine(FD->getSourceRange());
                thr_routine_end = getEndLine(FD->getSourceRange());
                if(thr_routine_method != NULL)
                {
                  auto rec = thr_routine_method->getParent();
                  if(rec != NULL)
                  {
                    thr_routine_recordName = printRecordName(rec);
                    thr_routine_recordFilePath = printFilePath(rec->getSourceRange(), thr_routine_file);
                  }
                }
                this->getMangledName(mangle_context_, FD, &thr_routine_mangled);
                osOut << " thread routine name: " << thr_routine_name << ", sign: " << writeFunction(*FD)
                      << ", mangled:" << thr_routine_mangled << std::endl;
                // get the location of the caller function
                thr_create_location = callExpr->getRParenLoc().printToString(ciCompilerInstance.getSourceManager());
                osOut << "thread instantiation location: " << thr_create_location << std::endl;
              }
              // else
              // {
              //   osOut << " other_decl";
              // }
            }
          }
          else if(auto UO = llvm::dyn_cast<clang::UnaryOperator>(*argIter)) {
            osOut << " uo";
            if(auto DRE = llvm::dyn_cast<clang::DeclRefExpr>(UO->getSubExpr())) {
              // it's a reference to a declaration
              if(auto VD = llvm::dyn_cast<clang::VarDecl>(DRE->getDecl())) {
                // it's a reference to a variable
                thr_inst_name = VD->getNameAsString();
                osOut << " thread instance: " << thr_inst_name;
              }
              // else
              // {
              //   osOut << " other_decl";
              // }
            }
          }
        }

        std::string thr_routine_nspc = printRootNamespace(*thr_routine_method, printQualifiedName(*thr_routine_method), thr_routine_recordName);

        // Create the thread instance
        CallersData::Thread thread(thr_inst_name, thr_routine_name, thr_routine_sign, thr_routine_mangled,
                                   thr_routine_virtuality, thr_routine_nspc, thr_routine_file, thr_routine_begin, thr_routine_end,
                                   thr_routine_recordName, thr_routine_recordFilePath, thr_create_location, caller_mangled, caller_sign,
                                   caller_virtuality, caller_nspc, caller_def_file,
                                   caller_def_begin, caller_def_end, caller_decl_filepos,
                                   caller_recordName, caller_recordFilePath);
        currentJsonFile->add_thread(&thread, &otherJsonFiles);
      }
      return true;
   }

   if (callExpr->getCallee()->getStmtClass() == clang::Stmt::CXXPseudoDestructorExprClass)
      return true;
   osOut << "  " << caller_sign << " -> dynamic call\n";
   return true;
}

bool
CallersAction::Visitor::VisitMemberCallExpr(const clang::CXXMemberCallExpr* callExpr) {
   const clang::Expr* callee = callExpr->getCallee();
   bool isVirtualCall = callExpr->getMethodDecl()->isVirtual();
   if (isVirtualCall && callee) {
      ASSERT(llvm::dyn_cast<const clang::MemberExpr>(callee));
      isVirtualCall = !static_cast<const clang::MemberExpr*>(callee)->hasQualifier();
   }
   const clang::FunctionDecl* directCall = callExpr->getMethodDecl();
   if (isVirtualCall) {
     clang::CXXRecordDecl* baseClass = callExpr->getMethodDecl()->getParent();
      // _callersAction->registerDerivedCalls(sParent, baseClass, callExpr->getMethodDecl());
     std::string calleeSign = writeFunction(*directCall);
     osOut << printParentFunction() << " -> derived of class "
           << printQualifiedName(*baseClass) << printTemplateKind(*baseClass)
           << " method " << calleeSign << '\n';
   }
   else {
      std::string calleeSign = writeFunction(*directCall);
      osOut << "  " << printParentFunction() << " -8-> " << calleeSign << '\n';
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

      std::string fctDef_sign = writeFunction(*function);
      std::string fctDef_filepath = printFilePath(function->getSourceRange());
      int fctDef_begin = getStartLine(function->getSourceRange());
      int fctDef_end = getEndLine(function->getSourceRange());
      std::string fctDef_pos = printLocation(function->getSourceRange());

        // check the function virtuality
        auto methodDef = llvm::dyn_cast<clang::CXXMethodDecl>(function);
        auto virtualityDef = (methodDef && methodDef->isVirtual()) ?
          (methodDef->isPure() ? CallersData::VVirtualPure : CallersData::VVirtualDefined)
          : CallersData::VNoVirtual;

        MangledName fctDef_mangledName;
        this->getMangledName(mangle_context_, function, &fctDef_mangledName);

        std::string fctDef_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
        std::string fctDef_recordFilePath = fctDef_filepath;
        if(methodDef != NULL)
        {
          auto rec_def = methodDef->getParent();
          if(rec_def != NULL)
          {
            fctDef_recordName = printRecordName(rec_def);
            fctDef_recordFilePath = printFilePath(rec_def->getSourceRange(), fctDef_filepath);
          }
        }
        std::string fctDef_nspc = printRootNamespace(*function, printQualifiedName(*function), fctDef_recordName);

        // get the function declaration and check it's position
        clang::FunctionDecl* functionDecl = function->getCanonicalDecl();
        std::string fctDecl_filepath = printFilePath(functionDecl->getSourceRange());
        std::string fctDecl_filepos = printLocation(functionDecl->getSourceRange());

        int fctDecl_begin = getStartLine(functionDecl->getSourceRange());
        int fctDecl_end = getEndLine(functionDecl->getSourceRange());

        CallersData::FctDef v_fctDef(fctDef_mangledName, fctDef_sign, virtualityDef, fctDef_nspc,
                                   fctDef_filepath, fctDef_begin, fctDef_end, fctDecl_filepos, fctDef_recordName, fctDef_recordFilePath);
        std::set<CallersData::FctDef>::iterator
        fctDef = currentJsonFile->get_or_create_defined_function(&v_fctDef, fctDef_filepath, &otherJsonFiles);

        auto methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(functionDecl);
        auto virtualityDecl = (methodDecl && methodDecl->isVirtual()) ?
          (methodDecl->isPure() ? CallersData::VVirtualPure : CallersData::VVirtualDeclared)
          : CallersData::VNoVirtual;

        std::string fctDecl_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
        std::string fctDecl_recordFilePath = fctDecl_filepath;
        if(methodDecl != NULL)
        {
          auto rec_decl = methodDecl->getParent();
          if(rec_decl != NULL)
          {
            fctDecl_recordName = printRecordName(rec_decl);
            fctDecl_recordFilePath = printFilePath(rec_decl->getSourceRange(), fctDecl_filepath);
          }
        }

        osOut << "visiting function " << sParent
              << " defined at file " << fctDef_filepath << ':' << fctDef_begin
              << " and declared at file " << fctDecl_filepath << ':' << fctDecl_begin << std::endl;

        std::string fctDecl_nspc = printRootNamespace(*functionDecl, printQualifiedName(*functionDecl), fctDecl_recordName);

        // Complete the function definition with a new "declaration" entry
        CallersData::FctDecl v_fctDecl(fctDef_mangledName, fctDef_sign, virtualityDecl, fctDecl_nspc,
                                     fctDecl_filepath, fctDecl_begin, fctDecl_end, fctDecl_recordName, fctDecl_recordFilePath);

        std::set<CallersData::FctDecl>::iterator fctDecl = currentJsonFile->get_or_create_declared_function(&v_fctDecl, fctDecl_filepath, &otherJsonFiles);

        VisitFunctionParameters(*functionDecl, *fctDecl);
        if(fctDef_filepath == fctDecl_filepos)
        {
          std::ostringstream s_begin;
          s_begin << fctDef_begin;
          std::ostringstream s_end;
          s_end << fctDef_end;
          fctDef_pos = std::string(CALLERS_LOCAL_FCT_DECL) + ":" + s_begin.str() + ":" + s_end.str();
        }
        fctDecl->add_definition(fctDef_sign, fctDef_pos);

   return true;
}

// Visit function parameters
bool CallersAction::Visitor::VisitFunctionParameters(const clang::FunctionDecl& function, const CallersData::FctDecl& fct_decl)
{
#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
   auto paramIter = function.param_begin(),
        paramIterEnd = function.param_end();
#else
   auto paramIter = function.arg_begin(),
        paramIterEnd = function.arg_end();
#endif

   for (; paramIter != paramIterEnd; ++paramIter)
   {
      std::string param_name = (*paramIter)->getName();
      std::string param_type = printType((*paramIter)->getType());
      CallersData::Parameter param(param_name, param_type);
      fct_decl.add_parameter(param);
   };
   return true;
}

bool
CallersAction::Visitor::VisitFunctionDeclaration(clang::FunctionDecl* function) {

      ASSERT(function != NULL);
      std::string fct_sign = writeFunction(*function);
      std::string fct_filepath = printFilePath(function->getSourceRange());
      int fct_begin = getStartLine(function->getSourceRange());
      int fct_end = getEndLine(function->getSourceRange());

        auto virtualityDecl = CallersData::VNoVirtual;

        MangledName fct_mangledName;
        this->getMangledName(mangle_context_, function, &fct_mangledName);
        // osOut << "Debug mangledName ok: " << fct_mangledName << '\n';

        auto methodDef = llvm::dyn_cast<clang::CXXMethodDecl>(function);
        std::string fct_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
        std::string fct_recordFilePath = fct_filepath;
        if(methodDef != NULL)
        {
          auto rec_decl = methodDef->getParent();
          if(rec_decl != NULL)
          {
            fct_recordName = printRecordName(rec_decl);
            fct_recordFilePath = printFilePath(rec_decl->getSourceRange(), fct_filepath);
          }
        }

        std::string fct_nspc = printRootNamespace(*function, printQualifiedName(*function), fct_recordName);

        CallersData::FctDecl fctDecl(fct_mangledName, fct_sign, virtualityDecl, fct_nspc, fct_filepath,
                                     fct_begin, fct_end, fct_recordName, fct_recordFilePath);

        VisitFunctionParameters(*function, fctDecl);

        currentJsonFile->get_or_create_declared_function(&fctDecl, fct_filepath, &otherJsonFiles);

    return true;
}

bool
CallersAction::Visitor::VisitMethodDeclaration(clang::CXXMethodDecl* methodDecl) {

      ASSERT(methodDecl != NULL);
      std::string fct_sign = writeFunction(*methodDecl);
      std::string fct_filepath = printFilePath(methodDecl->getSourceRange());
      int fct_begin = getStartLine(methodDecl->getSourceRange());
      int fct_end = getStartLine(methodDecl->getSourceRange());

        auto virtualityDecl = (methodDecl && methodDecl->isVirtual()) ?
          (methodDecl->isPure() ? CallersData::VVirtualPure : CallersData::VVirtualDeclared)
          : CallersData::VNoVirtual;

		if (methodDecl->isVirtual()) {
		osOut << "visiting virtual declared method " << fct_sign
			  << " at " << fct_filepath << ':' << fct_begin << '\n';
		  }
		  else if (methodDecl->isPure()) {
		osOut << "visiting virtual pure declared method " << fct_sign
			  << " at " << fct_filepath << ':' << fct_begin << '\n';
		  }
		  else {
		osOut << "visiting non-virtual declared method " << fct_sign
			  << " at " << fct_filepath << ':' << fct_begin << '\n';
		  }

        MangledName fct_mangledName;
        this->getMangledName(mangle_context_, methodDecl, &fct_mangledName);
        osOut << "Debug mangledName ok: " << fct_mangledName << '\n';

        std::string fct_recordName = CALLERS_DEFAULT_NO_RECORD_NAME;
        std::string fct_recordFilePath = fct_filepath;
        if((methodDecl != NULL) && (methodDecl->getParent() != NULL))
        {
          fct_recordName = printRecordName(methodDecl->getParent());
          fct_recordFilePath = printFilePath(methodDecl->getParent()->getSourceRange(), fct_filepath);
        }

        std::string fct_nspc = printRootNamespace(*methodDecl, printQualifiedName(*methodDecl), fct_recordName);

        CallersData::FctDecl fctDecl(fct_mangledName, fct_sign, virtualityDecl, fct_nspc, fct_filepath, fct_begin, fct_end, fct_recordName, fct_recordFilePath);
        VisitFunctionParameters(*methodDecl, fctDecl);
        currentJsonFile->get_or_create_declared_function(&fctDecl, fct_filepath, &otherJsonFiles);

    return true;
}

bool
CallersAction::Visitor::isDeclarationOfInterest(const clang::NamedDecl& namedDecl) {

  bool ofInterest = false;
  auto ifMethod = llvm::dyn_cast<clang::CXXMethodDecl>(&namedDecl);
  std::string recordName = CALLERS_DEFAULT_RECORD_NAME;
  if((ifMethod != NULL) && (ifMethod->getParent() != NULL))
  {
    recordName = printRecordName(ifMethod->getParent());
  }
  std::string fct_nspc = printRootNamespace(namedDecl, printQualifiedName(namedDecl), recordName);
  static const std::string filtered_nspc = "std:boost:__gnu_cxx:mpl_:builtin:__cxxabiv1:";
  ofInterest = ! boost::algorithm::contains(filtered_nspc, fct_nspc);
  return ofInterest;
}

// global variable:out: pfdParent
bool
CallersAction::Visitor::VisitFunctionDecl(clang::FunctionDecl* Decl) {

   pfdParent = Decl;
   std::string fct_filepath = printFilePath(Decl->getSourceRange());
   sParent = writeFunction(*Decl);
   std::string fct_begin = printNumber(getStartLine(Decl->getSourceRange()));
   
   bool isDefinition = Decl->isThisDeclarationADefinition();
   auto isMethodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(Decl);

   if(this->isDeclarationOfInterest(*Decl))
   {
     if (isDefinition) {

            osOut << "visiting function \"" << sParent
                  << "\" at " << fct_filepath << ':' << fct_begin << std::endl;

            this->VisitFunctionDefinition(Decl);
          }

     else {

      if (isMethodDecl) {
          osOut << "visiting method \"" << sParent
                 << "\" declared at " << fct_filepath << ':' << fct_begin << std::endl;
         this->VisitMethodDeclaration(isMethodDecl);
      }
      else {
          // osOut << "visiting function \"" << sParent << "\" declared at " << fct_filepath << ':' << fct_begin << std::endl;
          osOut << "visiting function \"" << sParent;
          osOut << "\" declared at " << fct_filepath;
          osOut << ':' << fct_begin << std::endl;
          if(fct_filepath != CALLERS_NO_FILE_PATH)
          {
            this->VisitFunctionDeclaration(Decl);
          }
          else
          {
            osOut << "ignore function \"" << sParent
                   << "\" declared at " << fct_filepath << ':' << fct_begin << std::endl;
          }
      }
     }
   }
   else
   {
     if (isDefinition) {
       osOut << "ignore function \"" << sParent
              << "\" at " << fct_filepath << ':' << fct_begin << std::endl;
     }
     else {
       osOut << "ignore function \"" << sParent
              << "\" at " << fct_filepath << ':' << fct_begin << std::endl;
     }
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
					     std::set<CallersData::Record>::iterator& record)
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
      std::string baseName = printRecordName(base);
      clang::TagTypeKind baseTagKind = base->getTagKind();
      std::string baseNspc = printRootNamespace(*base, printQualifiedName(*base), baseName);
      clang::SourceRange baseSourceRange = base->getSourceRange();
      std::string baseFile = printFilePath(baseSourceRange);
      int baseBegin = getStartLine(baseSourceRange.getBegin());
      int baseEnd = getStartLine(baseSourceRange.getEnd());

      CallersData::Inheritance parent(baseName, baseFile, baseBegin, baseEnd);
      record->add_base_class(parent);

      osOut << " the base record \"" << baseName << "\" of record \"" << record->name << "\" declares the following methods:" << std::endl;

      for(clang::CXXRecordDecl::method_iterator mi = base->method_begin();
          mi != base->method_end();
          mi++)
      {
        clang::CXXMethodDecl* m = (clang::CXXMethodDecl*)(*mi);
        std::string method_sign = writeFunction(*m);
        std::string method_loc = printLocation(m->getSourceRange());
        osOut << " - " << method_sign << std::endl;
        // check if this method is virtual or not
        if(m->isVirtual())
        {
          CallersData::ExtFctDecl redecl_method("tbc_mangled", method_sign, method_loc);
          record->add_redeclared_method(baseName, redecl_method);
        }
      }

      osOut << " the base record \"" << baseName << "\" is inherited by child record \"" << record->name << "\"" << std::endl;
      CallersData::Inheritance child(record->name, record->file, record->begin, record->end);
      CallersData::Record search_parent(baseName, baseTagKind, baseNspc, baseFile, baseBegin, baseEnd);
      std::set<CallersData::Record>::iterator parent_record = currentJsonFile->get_or_create_record(&search_parent, &otherJsonFiles);
      parent_record->add_child_class(child);

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

     if(this->isDeclarationOfInterest(*Decl)) {

       clang::TagTypeKind tagKind = Decl->getTagKind();
        if (tagKind == clang::TTK_Struct || tagKind == clang::TTK_Class) { // avoid unions
           bool isAnonymousRecord = false;
           //std::string recordName = printQualifiedName(*Decl, &isAnonymousRecord);
           std::string recordName = printRecordName(RD);
           std::string recordNspc = printRootNamespace(*RD, printQualifiedName(*RD), CALLERS_DEFAULT_NO_RECORD_NAME);
	   clang::SourceRange recordLoc = Decl->getSourceRange();
           std::string recordFile = printFilePath(recordLoc);
           int recordBegin = getStartLine(recordLoc.getBegin());
           int recordEnd = getStartLine(recordLoc.getEnd());

           recordName += printTemplateKind(*Decl);
           if (!isAnonymousRecord) {

              osOut << "visiting record " << recordName
                    << " at " << printLocation(Decl->getSourceRange()) << '\n';

              CallersData::Record search_record(recordName, tagKind, recordNspc, recordFile, recordBegin, recordEnd);

              std::set<CallersData::Record>::iterator record = currentJsonFile->get_or_create_record(&search_record, &otherJsonFiles);

              osOut << " the record \"" << recordName << "\" inherits from ";
              VisitInheritanceList(RD, record);
              osOut << std::endl;

              osOut << " the record \"" << recordName << "\" declares the following methods:" << std::endl;

              for(clang::CXXRecordDecl::method_iterator m = RD->method_begin();
                  m != RD->method_end();
                  m++)
              {
                clang::FunctionDecl* f = (clang::FunctionDecl*)(*m);
                std::string method_sign = writeFunction(*f);
                osOut << " - " << method_sign << std::endl;
                record->add_method(method_sign);
              }

              osOut << '\n';
           }
        }
     }
   }
   return true;
}


// Local Variables:
// compile-command: "/usr/bin/c++  -DGTEST_HAS_RTTI=0 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -I/home/hugues/work/third_parties/src/build/tools/clang/tools/extra/callers/c -I/home/hugues/work/third_parties/src/llvm/tools/clang/tools/extra/callers/c -I/home/hugues/work/third_parties/src/llvm/tools/clang/include -I/home/hugues/work/third_parties/src/build/tools/clang/include -I/usr/include/libxml2 -I/home/hugues/work/third_parties/src/build/include -I/home/hugues/work/third_parties/src/llvm/include -I/home/hugues/work/third_parties/src/llvm/tools/clang/tools/extra/callers/c/..   -fPIC -fvisibility-inlines-hidden -Werror=date-time -std=c++11 -Wall -Wextra -Wno-unused-parameter -Wwrite-strings -Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long -Wimplicit-fallthrough -Wno-maybe-uninitialized -Wno-noexcept-type -Wdelete-non-virtual-dtor -Wno-comment -ffunction-sections -fdata-sections -fno-common -Woverloaded-virtual -fno-strict-aliasing -g    -fno-exceptions -fno-rtti -fexceptions -o visitor.cpp.o -c visitor.cpp"
// End:
