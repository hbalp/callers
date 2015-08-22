/****
     Copyright (C) 2015 Commissariat à l'Energie Atomique, Thales Communication
       - All Rights Reserved
     coded by Franck Vedrine, Hugues Balp
****/

//
// Description:
//   clang -> file containing called functions
//

//#define NOT_USE_BOOST_FILESYSTEM
#define NOT_USE_BOOST_REGEX

#include <climits>
#ifndef NOT_USE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
#endif
#ifndef NOT_USE_BOOST_REGEX
#include <boost/regex.hpp>
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

#if (CLANG_VERSION_MAJOR > 3)                                 \
    || (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR >= 5)
#define CLANG_VERSION_GREATER_OR_EQUAL_3_3_5
#endif

#ifndef NOT_USE_BOOST_REGEX
#ifndef NOT_USE_BOOST
void boost::throw_exception(std::exception const&)
{
  std::cerr << "HBDBG EXCEPTION TBC" << std::endl;
}
#endif
#endif

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
  std::string dirname = p.parent_path().string();

#ifdef CLANG_VERSION_GREATER_OR_EQUAL_3_7
  return llvm::make_unique<Visitor>(symbols, inputFile.str(), basename, dirname, fOut, compilerInstance); 
#else
  return new Visitor(symbols, inputFile.str(), basename, dirname, fOut, compilerInstance);
#endif
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
   TraverseDecl(context.getTranslationUnitDecl());
}

std::string
CallersAction::Visitor::printLocation(const clang::SourceRange& rangeLocation) const {
   assert(psSources);
   auto start = psSources->getPresumedLoc(rangeLocation.getBegin());
   const char* startFile = start.getFilename();
   int startLine = 0;
   if (!startFile)
      startFile = "<unknown>";
   else
      startLine = start.getLine();
   std::string result = startFile;
   result += ':';
   std::ostringstream out;
   out << startLine;
   result += out.str();
   return result;
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

std::string
CallersAction::Visitor::printQualification(const clang::DeclContext* context) const {
   if (context) {
      std::string result = printQualification(context->getParent());
      const clang::Decl::Kind kind = context->getDeclKind();
      if (kind == clang::Decl::Namespace) {
         if (result.length() > 0)
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
CallersAction::Visitor::printQualifiedName(const clang::NamedDecl& namedDecl, bool* isEmpty) const {
   std::string result = printQualification(namedDecl.getDeclContext());
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
      else { // see build-llvm/tools/clang/include/clang/AST/DeclNodes.inc

      };
   };
   result += name;
   result += printTemplateKind(function);
   result += printArgumentSignature(function);
   return result;
}

bool
CallersAction::Visitor::VisitCXXConstructExpr(const clang::CXXConstructExpr* constructor) {
   assert(llvm::dyn_cast<clang::FunctionDecl>(constructor->getConstructor()));
   const auto& function = *static_cast<const clang ::FunctionDecl*>(constructor->getConstructor());
   std::string name = printQualifiedName(function);
   if (name.length() == 0)
      name = "constructor-special";
   std::string result = name;
   result += printTemplateKind(function);
   result += printArgumentSignature(function);
   osOut << inputFile << ": " << printParentFunction() << " -> " << result << '\n';

   CallersData::FctCall fc(printParentFunction(), printParentFunctionLine(), writeFunction(function), printLine(function.getSourceRange()));
   jsonFile.add_function_call(&fc, symbols);

   return true;
}

bool
CallersAction::Visitor::VisitCXXDeleteExpr(const clang::CXXDeleteExpr* deleteExpr) {
   if (deleteExpr->getOperatorDelete()
         && !deleteExpr->getOperatorDelete()->isImplicit()) {
      const clang::FunctionDecl& function = *deleteExpr->getOperatorDelete();
      std::string result = printResultSignature(function);
      result += ' ';
      result += printQualifiedName(function);
      result += printArgumentSignature(function);
      osOut << inputFile << ": " << printParentFunction() << " -> " << result << '\n';

      CallersData::FctCall fc(printParentFunction(), printParentFunctionLine(), result, printLine(function.getSourceRange()));
      jsonFile.add_function_call(&fc, symbols);

      return true;
   };
   const auto* recordDecl = deleteExpr->getType()->getPointeeCXXRecordDecl();
   if (!recordDecl)
      recordDecl = deleteExpr->getType()->getCanonicalTypeInternal().getTypePtr()
         ->getAsCXXRecordDecl();
   if (recordDecl) {
      clang::CXXDestructorDecl* destructor = recordDecl->getDestructor();
      if (destructor) {
         std::string result = printQualifiedName(*destructor);
         result += "()";
         osOut << inputFile << ": " << printParentFunction() << " -> " << result << '\n';

	 CallersData::FctCall fc(printParentFunction(), printParentFunctionLine(), result, printLine(destructor->getSourceRange()));
	 jsonFile.add_function_call(&fc, symbols);
      };
   };
   return true;
}

bool
CallersAction::Visitor::VisitCXXNewExpr(const clang::CXXNewExpr* newExpr) {
   if (newExpr->getOperatorNew() && !newExpr->getOperatorNew()->isImplicit()) {
      const auto& operatorNew = *newExpr->getOperatorNew();
      std::string result = printResultSignature(operatorNew);
      result += ' ';
      result += printQualifiedName(operatorNew);
      if (newExpr->isArray())
         result += " [] ";
      result += printArgumentSignature(operatorNew);
   };
   return true;
}

bool
CallersAction::Visitor::VisitCallExpr(const clang::CallExpr* callExpr) {
   const clang::FunctionDecl* fd = callExpr->getDirectCallee();
   if (fd) {
      unsigned builtinID = fd->getBuiltinID();
      if (builtinID > 0)
         return true;
      std::string result = writeFunction(*fd);
      osOut << inputFile << ": " << printParentFunction() << " -> " << result << '\n';

      CallersData::FctCall fc(printParentFunction(), printParentFunctionLine(), result, printLine(fd->getSourceRange()));
      jsonFile.add_function_call(&fc, symbols);
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
      std::string result = writeFunction(*directCall);
      osOut << printParentFunction() << " -> derived of class "
         << printQualifiedName(*baseClass) << printTemplateKind(*baseClass)
         << " method " << result << '\n';
   }
   else {
      std::string result = writeFunction(*directCall);
      osOut << "  " << printParentFunction() << " -> " << result << '\n';
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

bool
CallersAction::Visitor::VisitFunctionDecl(clang::FunctionDecl* Decl) {
   if (Decl->isThisDeclarationADefinition()) {
      pfdParent = Decl;
      sParent = writeFunction(*Decl);
      osOut << "visiting function " << sParent
            << " at " << printLocation(Decl->getSourceRange()) << '\n';
      CallersData::Fct fct(writeFunction(*Decl), printLine(Decl->getSourceRange()));
      jsonFile.add_defined_function(&fct);
   };
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
CallersAction::Visitor::VisitInheritanceList(clang::CXXRecordDecl* cxxDecl) {
   clang::CXXRecordDecl::base_class_iterator endBase = cxxDecl->bases_end();
   bool isFirst = true;
   for (clang::CXXRecordDecl::base_class_iterator
         iterBase = cxxDecl->bases_begin(); iterBase != endBase; ++iterBase) {
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
         osOut << ",\n                  ";
      else
         isFirst = false;
      osOut << printQualifiedName(*base);
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
      auto tagKind = Decl->getTagKind();
      if (tagKind == clang::TTK_Struct || tagKind == clang::TTK_Class) { // avoid unions
         bool isAnonymousRecord = false;
         std::string recordName = printQualifiedName(*Decl, &isAnonymousRecord);
         recordName += printTemplateKind(*Decl);
         if (!isAnonymousRecord) {
            osOut << "visiting record " << recordName
                  << " at " << printLocation(Decl->getSourceRange()) << '\n';
            osOut << "    inherits from ";
            VisitInheritanceList(RD);
            osOut << '\n';
         };
      }
   };

   return true;
}
