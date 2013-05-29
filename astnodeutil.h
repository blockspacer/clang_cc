#ifndef ASTNODEUTIL_H_
#define ASTNODEUTIL_H_
#include <clang/AST/Decl.h>
#include "ASTnodefinder.h"
namespace ASTNode
{

using namespace clang;


const Decl* GetDeclaration(const NodeType& node);

const Decl* GetDeclarationFromStatement(const Stmt* stmt);
const Decl* GetDeclarationFromTypeLoc(const TypeLoc& tloc);
const Decl* GetDeclarationFromRefNode(const RefNode& refNode);
const Decl* GetDefinition(const Decl* decl);
/// Returns the source range this declaration covers
/// the range end points at the one past the location of
/// the last token or semicolon if it's present.
SourceRange GetDeclarationRange(const Decl* decl);

///Utility function for accessing the primary template
///a function or class declaration represents.
TemplateDecl* GetDescribedTemplate(const Decl* decl);
//Editor Commands

///Places the caret at the start of the declaration.
void GotoDeclarationInEditor(const Decl* decl);
/// Selects the declaration in the editor.
/// Selection range is calculated by GetDeclarationRange()
void SelectDeclarationInEditor(const Decl* decl);

}
#endif // ASTNODEUTIL_H_
