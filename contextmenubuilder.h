#pragma once

#include <clang/AST/DeclVisitor.h>
#include <clang/AST/StmtVisitor.h>
#include <clang/AST/TypeLocVisitor.h>
#include <wx/menu.h>
#include "ASTnodefinder.h"
#include "astnodeutil.h"
#include "clang_cc.h"

using namespace clang;
class ContextMenuBuilder : public ConstDeclVisitor<ContextMenuBuilder>,
                           public StmtVisitor<ContextMenuBuilder>,
                           public TypeLocVisitor<ContextMenuBuilder>,
                           public boost::static_visitor<>
{
public:
    ContextMenuBuilder(wxMenu* menu):
        m_Menu(menu)
    {}
    void operator()(const Decl* decl)
    {
		if (!decl)
			return;
        std::string log = "DeclKindName : ";
        log += decl->getDeclKindName();
        LoggerAccess::Get()->Log(std2wx(log));
        m_Menu->Append(idEditorGotoDeclaration, "Goto Declaration");
        if(const Decl* definition = ASTNode::GetDefinition(decl))
            if (definition != decl)
                 m_Menu->Append(idEditorGotoDeclaration, "Goto Definition");

    }
    void operator()(const Stmt* stmt)
    {
        std::string log = "StatementClassName : ";
        log += stmt->getStmtClassName();
        LoggerAccess::Get()->Log(std2wx(log));
        if (ASTNode::GetDeclarationFromStatement(stmt))
            m_Menu->Append(idEditorGotoDeclaration, "Goto Declaration");

    }
    void operator()(const boost::blank&)
    {
        std::string log = "Empty Ast Node : ";
        LoggerAccess::Get()->Log(std2wx(log));
    }
    void operator()(const TypeLoc& tloc)
    {
        std::string className = "TypeClass name of typeloc : ";
        className = className + tloc.getTypePtr()->getTypeClassName();
        std::string log = "TypeLoc : ";
        SplitQualType T_split = tloc.getType().split();
        log+= QualType::getAsString(T_split);
        LoggerAccess::Get()->Log(std2wx(log));
        LoggerAccess::Get()->Log(std2wx(className));

        operator()(ASTNode::GetDeclarationFromTypeLoc(tloc));
    }
    void operator()(const RefNode& refNode)
    {
       std::string log = "RefNode : ";
       m_Menu->Append(idEditorGotoDeclaration, "Goto Declaration");
       LoggerAccess::Get()->Log(std2wx(log));
    }
    void VisitStmt(Stmt* node) const
    {

    }
    void VisitDecl(Decl* node) const
    {
        m_Menu->Append(idEditorGotoDeclaration, "Goto Declaration");
    }
    void VisitFunctionDecl(FunctionDecl* decl)
    {
        if (decl->isThisDeclarationADefinition() &&
            decl->getPreviousDecl())
        {
            m_Menu->Append(idCodeLayoutViewGotoDeclaration, "Goto Declaration");
        }
        if (decl->hasBody() && !decl->isThisDeclarationADefinition())
        {
            m_Menu->Append(idCodeLayoutViewGotoDefinition, "Goto Definition");
        }
    }
    void VisitTagDecl(TagDecl* decl)
    {
        if (decl->isThisDeclarationADefinition() &&
            decl->getPreviousDecl())
        {
            m_Menu->Append(idCodeLayoutViewGotoDeclaration, "Goto Declaration");
        }
        if (decl->getDefinition() && !decl->isThisDeclarationADefinition())
        {
            m_Menu->Append(idCodeLayoutViewGotoDefinition, "Goto Definition");
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
            m_Menu->Append(idEditorGotoDeclaration, "Goto Declaration");
    }
    void VisitMemberExpr(MemberExpr* expr)
    {
        m_Menu->Append(idEditorGotoDeclaration, "Goto Declaration");
    }
    void VisitTagTypeLoc(TagTypeLoc tloc)
    {
        Decl*  decl= tloc.getDecl();
        m_Menu->Append(idEditorGotoDeclaration, "Goto Declaration");
    }
private:
    wxMenu* m_Menu;
};
