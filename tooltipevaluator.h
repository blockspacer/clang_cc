#pragma once
#include <clang/Lex/Lexer.h>
#include "astnodeutil.h"
#include "stringio.h"

class ToolTipEvaluator : public boost::static_visitor<wxString>
{
public:
    ToolTipEvaluator(ASTUnit* tu):
        m_Tu(tu)
    {}
    wxString operator()(const Decl* decl)
    {
        RawwxStringOstream out;
        if (const NamedDecl* named = dyn_cast_or_null<NamedDecl>(decl))
        {

           PrintingPolicy policy(m_Tu->getASTContext().getLangOpts());
           CodeLayoutDeclarationPrinter printer(out, policy);
           printer.Visit(named);

            for (auto it = decl->redecls_begin(), end = decl->redecls_end(); it != end; ++it)
            {
              //  const RawComment* raw = m_Tu->getASTContext().getRawCommentForAnyRedecl(decl);
                const RawComment* raw = m_Tu->getASTContext().getRawCommentForDeclNoCache(*it);
                if (raw)
                {
                    out << "\n" << raw->getRawText(m_Tu->getSourceManager());
                }
            }
         }
        return out.str();
    }
    wxString operator()(const Stmt* stmt)
    {
        const Decl* decl = ASTNode::GetDeclarationFromStatement(stmt);
        return operator()(decl);
    }
    wxString operator()(const boost::blank&)
    {
        return "";
    }
    wxString operator()(const TypeLoc& tloc)
    {
        const Decl* decl = ASTNode::GetDeclarationFromTypeLoc(tloc);
        return operator()(decl);
    }
    wxString operator()(const RefNode& refNode)
    {
        const Decl* decl = refNode.GetReferencedDeclaration();
        return operator()(decl);
    }

private:
   ASTUnit* m_Tu ;
};
