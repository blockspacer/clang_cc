#ifndef ASTNODEUTIL_H_
#define ASTNODEUTIL_H_
#include <clang/AST/Decl.h>
#include "ASTnodefinder.h"
namespace ASTNode
{




const clang::Decl* GetDeclaration(const NodeType& node);

const clang::Decl* GetDeclarationFromStatement(const clang::Stmt* stmt);
const clang::Decl* GetDeclarationFromTypeLoc(const clang::TypeLoc& tloc);
const clang::Decl* GetDeclarationFromRefNode(const RefNode& refNode);
const clang::Decl* GetDefinition(const clang::Decl* decl);
/// Returns the source range this declaration covers
/// the range end points at the one past the location of
/// the last token or semicolon if it's present.
clang::SourceRange GetDeclarationRange(const clang::Decl* decl);

///Utility function for accessing the primary template
///a function or class declaration represents.
clang::TemplateDecl* GetDescribedTemplate(const clang::Decl* decl);
//Editor Commands

///Places the caret at the start of the declaration.
void GotoDeclarationInEditor(const clang::Decl* decl);
/// Selects the declaration in the editor.
/// Selection range is calculated by GetDeclarationRange()
void SelectDeclarationInEditor(const clang::Decl* decl);

}
#endif // ASTNODEUTIL_H_
