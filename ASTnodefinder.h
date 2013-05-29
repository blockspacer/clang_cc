#ifndef ASTNODEFINDER_H_
#define ASTNODEFINDER_H_
#include <iostream>
#include <boost/variant.hpp>
#include <clang/Lex/Lexer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#pragma push_macro("interface")
#undef interface
#include <clang/Frontend/ASTUnit.h>
#pragma pop_macro("interface")
#include <clang/AST/Attr.h>
#include <clang/AST/Comment.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;
enum LocationCompare
{
	Before,
	Inside,
	After
};
inline LocationCompare CompareLocationToRange(const SourceManager& SM, SourceLocation loc, SourceRange range)
{
	if (loc == range.getBegin() || loc == range.getEnd())
		return Inside;
	if (SM.isBeforeInTranslationUnit(loc, range.getBegin()))
		return Before;
	if (SM.isBeforeInTranslationUnit(range.getEnd(), loc))
		return After;
	return Inside;
}

enum RefType
{
   Namespace_Ref,
   NamespaceAlias_Ref,
   Type_Ref
};
class RefNode
{
public:
   RefNode(RefType type,Decl* decl,SourceRange loc):
       m_Type(type),
       m_Referenced(decl),
       m_Loc(loc)
   {}
   const Decl* GetReferencedDeclaration() const
   {
       return m_Referenced;
   }
private:
    RefType m_Type;
    Decl* m_Referenced;
    SourceRange m_Loc;
};
typedef boost::variant<boost::blank, Decl*, Stmt*, TypeLoc, RefNode> NodeType;
//typedef llvm::PointerUnion4<Decl*,Stmt*,Attr*,PreprocessedEntity,TypeLoc*> NodeType;
class ASTNodeFinder : public RecursiveASTVisitor<ASTNodeFinder>
{
public:
	ASTNodeFinder(ASTUnit* tu):
        m_Tu(tu),
		m_Sm(tu->getASTContext().getSourceManager())
	{}
	NodeType GetASTNode(std::string fileName, unsigned pos);
	NodeType GetASTNode(std::string fileName, unsigned line, unsigned column);
    bool shouldWalkTypesOfTypeLocs() const { return false; }
	bool VisitNode(const NodeType& node, SourceRange range);
    bool TraverseNestedNameSpecifierLoc(NestedNameSpecifierLoc tloc);

    bool VisitStmt(Stmt *S);
    bool VisitCXXThisExpr(CXXThisExpr* expr);
	bool VisitDecl(Decl* decl);
    bool VisitNamespaceAliasDecl(NamespaceAliasDecl* decl);
    bool VisitTypeLoc(TypeLoc tloc);


private:
	NodeType FindNode(SourceLocation loc);
private:
	SourceLocation m_Loc;
	ASTUnit* m_Tu;
	const SourceManager& m_Sm;
	NodeType m_Node ;

};

#endif // ASTNODEFINDER_H_
