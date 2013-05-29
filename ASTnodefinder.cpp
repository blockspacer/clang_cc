
#include "ASTnodefinder.h"

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
	m_Loc = Lexer::GetBeginningOfToken(loc, m_Sm, m_Tu->getASTContext().getLangOpts());
    SourceLocation endloc= Lexer::getLocForEndOfToken(loc, 0, m_Sm, m_Tu->getASTContext().getLangOpts());
	auto fileAndOffset = m_Sm.getDecomposedLoc(m_Sm.getFileLoc(m_Loc));
	llvm::SmallVector<Decl*, 4> decls;
    //Find and visit translation unit level declarations
	//around the location we are interested.
	m_Tu->findFileRegionDecls(fileAndOffset.first, fileAndOffset.second, 0, decls);
    for (auto decl : decls)
	{
		LocationCompare inside = CompareLocationToRange(m_Sm, m_Loc, decl->getSourceRange());
		if(inside == Inside)
		{
			TraverseDecl(decl);
		}
	}
	return m_Node;
}
bool ASTNodeFinder::VisitNode(const NodeType& node, SourceRange range)
{
    if (!range.isValid())
        return true;
    switch (CompareLocationToRange(m_Sm, m_Loc, range))
    {
        case Inside:
            m_Node = node;
            return true;
        default:
            return true;
   }
}
bool ASTNodeFinder::TraverseNestedNameSpecifierLoc(NestedNameSpecifierLoc tloc)
    {
        SmallVector<NestedNameSpecifierLoc, 4> qualifiers;
          for (; tloc; tloc = tloc.getPrefix())
            qualifiers.push_back(tloc);

          while (!qualifiers.empty())
          {
            NestedNameSpecifierLoc Q = qualifiers.pop_back_val();
            NestedNameSpecifier *nns = Q.getNestedNameSpecifier();
            switch (nns->getKind())
            {
              case NestedNameSpecifier::Namespace:
                if(!VisitNode(RefNode(Namespace_Ref, nns->getAsNamespace(), Q.getLocalSourceRange()), Q.getLocalSourceRange()))
                   return false;
               break;
              case NestedNameSpecifier::NamespaceAlias:
                if(!VisitNode(RefNode(NamespaceAlias_Ref, nns->getAsNamespaceAlias(), Q.getLocalSourceRange()), Q.getLocalSourceRange()))
                    return false;
                break;
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

bool ASTNodeFinder::VisitDecl( Decl* decl )
{
    return VisitNode(decl, decl->getSourceRange());
}
bool ASTNodeFinder::VisitNamespaceAliasDecl(NamespaceAliasDecl* decl)
{
    return VisitNode(RefNode(NamespaceAlias_Ref, decl->getAliasedNamespace(), decl->getTargetNameLoc()),
                     decl->getTargetNameLoc());
}

bool ASTNodeFinder::VisitTypeLoc(TypeLoc tloc)
{
   //We omit those are below.Their components
   //will be visited by the recursive AST visitor
   //and we don't want the basic type to hide
   //parts of a declaration.
   switch(tloc.getTypeLocClass())
   {
    case TypeLoc::FunctionProto :
    case TypeLoc::FunctionNoProto:
    case TypeLoc::ConstantArray:
    case TypeLoc::IncompleteArray:
    case TypeLoc::VariableArray:
    case TypeLoc::DependentSizedArray:
        return true;
    default:
        return VisitNode(tloc, tloc.getSourceRange());
   }
}

bool ASTNodeFinder::VisitStmt(Stmt* stmt)
{
    switch(stmt->getStmtClass())
    {
        case Stmt::CXXThisExprClass:
        case Stmt::ImplicitCastExprClass:
            return true;
        default:
            return VisitNode(stmt, stmt->getSourceRange());
    }
}
bool ASTNodeFinder::VisitCXXThisExpr(CXXThisExpr* expr)
{
     if(!expr->isImplicit())
        return VisitNode(expr, expr->getSourceRange());
    return true;
}



