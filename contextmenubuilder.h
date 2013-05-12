#ifndef CONTEXTMENUBUILDER_H_
#define CONTEXTMENUBUILDER_H_

#include <clang/AST/DeclVisitor.h>
#include <clang/AST/StmtVisitor.h>
#include <clang/AST/TypeLocVisitor.h>
#include "ASTnodefinder.h"

using namespace clang;
class ContextMenuBuilder : public DeclVisitor<ContextMenuBuilder>,
                           public StmtVisitor<ContextMenuBuilder>,
                           public TypeLocVisitor<ContextMenuBuilder>,
                           public boost::static_visitor<>
{
public:
    ContextMenuBuilder(wxMenu* menu):
        m_Menu(menu)
    {}
    void operator()(Decl* decl)
    {
        std::string log = "DeclKindName : ";
        log += decl->getDeclKindName();
        ClangCCLogger::Get()->Log(std2wx(log));
        DeclVisitor::Visit(decl);
    }
    void operator()(Stmt* stmt)
    {
        std::string log = "StatementClassName : ";
        log += stmt->getStmtClassName();
        ClangCCLogger::Get()->Log(std2wx(log));
        StmtVisitor::Visit(stmt);
    }
    void operator()(const boost::blank&)
    {
        std::string log = "Empty Ast Node : ";
        ClangCCLogger::Get()->Log(std2wx(log));
    }
    void operator()(TypeLoc& tloc)
    {
        std::string log = "TypeLoc : ";
        SplitQualType T_split = tloc.getType().split();
        log+= QualType::getAsString(T_split);

        ClangCCLogger::Get()->Log(std2wx(log));
        TypeLocVisitor::Visit(tloc);
    }
    void operator()(const RefNode& refNode)
    {
       std::string log = "RefNode : ";
       m_Menu->Append(idEditorGotoDeclaration, _T("Goto Declaration"));
       ClangCCLogger::Get()->Log(std2wx(log));
    }
    void VisitStmt(Stmt* node) const
    {

    }
    void VisitDecl(Decl* node) const
    {
        m_Menu->Append(idEditorGotoDeclaration, _T("Goto Declaration"));
    }
    void VisitFunctionDecl(FunctionDecl* decl)
    {
        if (decl->isThisDeclarationADefinition() &&
            decl->getPreviousDecl())
        {
            m_Menu->Append(idBrowserGotoDeclaration, _T("Goto Declaration"));
        }
        if (decl->hasBody() && !decl->isThisDeclarationADefinition())
        {
            m_Menu->Append(idBrowserGotoDefinition, _T("Goto Definition"));
        }
    }
    void VisitTagDecl(TagDecl* decl)
    {
        if (decl->isThisDeclarationADefinition() &&
            decl->getPreviousDecl())
        {
            m_Menu->Append(idBrowserGotoDeclaration, _T("Goto Declaration"));
        }
        if (decl->getDefinition() && !decl->isThisDeclarationADefinition())
        {
            m_Menu->Append(idBrowserGotoDefinition, _T("Goto Definition"));
        }
    }
    void VisitDeclRefExpr(DeclRefExpr* expr)
    {
        NamedDecl* decl = expr->getDecl();
        if(!decl)
        {
            decl = expr->getFoundDecl();
        }
        if(decl)
            m_Menu->Append(idEditorGotoDeclaration, _T("Goto Declaration"));
    }
    void VisitMemberExpr(MemberExpr* expr)
    {
        m_Menu->Append(idEditorGotoDeclaration, _T("Goto Declaration"));
    }
    void VisitTagTypeLoc(TagTypeLoc tloc)
    {
        Decl*  decl= tloc.getDecl();
        m_Menu->Append(idEditorGotoDeclaration, _T("Goto Declaration"));
    }
private:
    wxMenu* m_Menu;
};
#endif // CONTEXTMENUBUILDER_H_
