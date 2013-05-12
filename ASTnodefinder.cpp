
#include "ASTnodefinder.h"
#include "ASTTypeTraits.h"

NodeType ASTNodeFinder::GetASTNode( std::string fileName, unsigned line, unsigned column )
{
	const FileEntry* fe = m_Tu->getFileManager().getFile(fileName);
	SourceLocation loc = m_Tu->getLocation(fe, line, column);
	return FindNode(loc);
}

NodeType ASTNodeFinder::GetASTNode( std::string fileName, unsigned pos )
{
	const FileEntry* fe = m_Tu->getFileManager().getFile(fileName);
	SourceLocation loc = m_Tu->getLocation(fe, pos);
	return FindNode(loc);
}

NodeType ASTNodeFinder::FindNode(SourceLocation loc)
{
	// Since source ranges for statements usually returns same
	// start and end locations, we get the beginning of the token
	// to match the location.
	m_Loc = Lexer::GetBeginningOfToken(loc,m_Sm,m_Tu->getASTContext().getLangOpts());
    SourceLocation endloc= Lexer::getLocForEndOfToken(loc,0,m_Sm,m_Tu->getASTContext().getLangOpts());
	auto fileAndOffset = m_Sm.getDecomposedLoc(m_Sm.getFileLoc(m_Loc));
	llvm::SmallVector<Decl*,4> decls;
    //Find and visit translation unit level declarations
	//around the location we are interested.
	m_Tu->findFileRegionDecls(fileAndOffset.first,fileAndOffset.second,0,decls);
	for(auto it = decls.begin();it != decls.end(); ++it)
	{
		Decl* decl = *it;
		LocationCompare inside = CompareLocationToRange(m_Sm, m_Loc, decl->getSourceRange());
		if(inside == Inside)
		{
			TraverseDecl(decl);
		}
	}
	return m_Node;
}
bool ASTNodeFinder::VisitNode(const NodeType& node,SourceRange range)
{
    if (!range.isValid())
        return true;
    switch (CompareLocationToRange(m_Sm, m_Loc, range))
    {
        case Before:
            return false;
        case After:
            return true;
        case Inside:
            m_Node = node;
            return true;
   }
}
bool ASTNodeFinder::TraverseNestedNameSpecifierLoc(NestedNameSpecifierLoc tloc)
    {
        SmallVector<NestedNameSpecifierLoc, 4> Qualifiers;
          for (; tloc; tloc = tloc.getPrefix())
            Qualifiers.push_back(tloc);

          while (!Qualifiers.empty())
          {
            NestedNameSpecifierLoc Q = Qualifiers.pop_back_val();
            NestedNameSpecifier *NNS = Q.getNestedNameSpecifier();
            switch (NNS->getKind())
            {
              case NestedNameSpecifier::Namespace:
                if(!VisitNode(RefNode(Namespace_Ref,NNS->getAsNamespace(),Q.getLocalSourceRange()),Q.getLocalSourceRange()))
                   return false;
               break;
              case NestedNameSpecifier::NamespaceAlias:
              case NestedNameSpecifier::Global:
              case NestedNameSpecifier::Identifier:
                return true;

              case NestedNameSpecifier::TypeSpec:
              case NestedNameSpecifier::TypeSpecWithTemplate:
                return TraverseTypeLoc(Q.getTypeLoc());
              break;
            }
    }
     return true;
}

//bool ASTNodeFinder::VisitStmt( Stmt *stmt )
//{
//    return VisitNode(stmt,stmt->getSourceRange());
//}

bool ASTNodeFinder::VisitDecl( Decl* decl )
{
    return VisitNode(decl,decl->getSourceRange());
}
bool ASTNodeFinder::VisitTypeLoc(TypeLoc tloc)
{
    return VisitNode(tloc, tloc.getSourceRange());
}
bool ASTNodeFinder::VisitTagTypeLoc(TagTypeLoc tloc )
{
    return VisitNode(tloc,tloc.getSourceRange());

}

bool ASTNodeFinder::VisitTypedefTypeLoc(TypedefTypeLoc tloc)
{
    return VisitNode(tloc,tloc.getSourceRange());

}

bool ASTNodeFinder::VisitCXXThisExpr(CXXThisExpr* expr)
{
     if(!expr->isImplicit())
        return VisitNode(expr,expr->getSourceRange());
    return true;
}

#define IMPL_VISIT_EXPR(EXPR)      \
    bool ASTNodeFinder::Visit##EXPR(EXPR* expr) \
    {                                                  \
        return VisitNode(expr,expr->getSourceRange()); \
    } \

IMPL_VISIT_EXPR(DeclRefExpr)
IMPL_VISIT_EXPR(MemberExpr)


