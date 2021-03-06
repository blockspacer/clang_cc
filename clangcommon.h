#ifndef CLANGCOMMON_H_
#define CLANGCOMMON_H_

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/MemoryBuffer.h>

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/LangOptions.h>

#include <clang/AST/Decl.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Comment.h>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/PCHContainerOperations.h>
#include <clang/Sema/CodeCompleteConsumer.h>

#include <clang/Lex/Lexer.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#endif // CLANGCOMMON_H_INCLUDED
