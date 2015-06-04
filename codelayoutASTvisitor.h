
#pragma once



#include "clang_cc.h"
#include <stack>
#include <sstream>
#include "clangcclogger.h"
#include "stringio.h"
#include "codelayoutnameprinter.h"
using namespace clang;

class CodeLayoutASTVisitor	: public clang::RecursiveASTVisitor<CodeLayoutASTVisitor> {

public:
	CodeLayoutASTVisitor(CodeLayoutView& view,ASTContext& ctx,const FileEntry* fileEntry):
	    m_View(view),
        m_Ctx(ctx),
        m_SM(ctx.getSourceManager()),
        m_FileEntry(fileEntry),
        m_Policy(m_Ctx.getLangOpts())
	{
	    m_Policy.Bool = true;
	    m_Policy.SuppressScope = true;
	    m_Policy.TerseOutput = true;
	    m_Policy.AnonymousTagLocations = false;
	}
    wxTreeItemId FindParent(clang::Decl* parentDecl)
    {
        wxTreeItemId parentItem;
        while(!m_ParentStack.empty())
        {
            parentItem = m_ParentStack.top();
            CodeLayoutViewItemData* lpData = static_cast<CodeLayoutViewItemData*>
                                        (m_View.GetTreeCtrl()->GetItemData(parentItem));
            if (lpData->m_Data == parentDecl)
                break;
            parentItem.Unset();
            m_ParentStack.pop();
        }
        return parentItem;

    }
    void AddToView(wxString name, clang::Decl* decl)
    {
        DeclContext* dc = decl->getDeclContext();
        Decl* parent = clang::dyn_cast<clang::Decl>(dc);
        wxTreeItemId parentItem = FindParent(parent);
        if (parentItem.IsOk())
            m_ParentStack.push(m_View.AddNode(name,decl,parentItem));
        else
            m_ParentStack.push(m_View.AddNode(name,decl,parent));

    }
    bool IsOutOfFile(Decl* decl)
    {
        const FileEntry* entry = m_SM.getFileEntryForID(m_SM.getFileID(decl->getLocation()));
        return (entry != m_FileEntry);
    }
    void PrintDecl(llvm::raw_ostream& out, Decl* decl)
    {
        CodeLayoutDeclarationPrinter printer(out, m_Policy);
        printer.Visit(decl);
    }
    wxString PrintDecl(Decl* decl)
    {
        RawwxStringOstream out;
        PrintDecl(out, decl);
        return out.str();
    }

	bool VisitEnumDecl(clang::EnumDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        AddToView(PrintDecl(decl), decl);
        return true;
	}
	bool VisitEnumConstantDecl(clang::EnumConstantDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        AddToView(PrintDecl(decl), decl);
        return true;
	}

	bool VisitRecordDecl(clang::RecordDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        if (clang::isa<CXXRecordDecl>(decl))
            return true;
        AddToView(PrintDecl(decl), decl);
        return true;
	}

	bool VisitCXXRecordDecl(CXXRecordDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        if (clang::isa<ClassTemplateSpecializationDecl>(decl))
            return true;
        AddToView(PrintDecl(decl), decl);
        return true;
	}

//	bool VisitClassTemplateDecl(ClassTemplateDecl* decl)
//	{
//	    if (IsOutOfFile(decl))
//            return true;
//        AddToBrowser(PrintDecl(decl->getTemplatedDecl()),decl);
//        return true;
//
//	}
	bool VisitNamespaceDecl(clang::NamespaceDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        AddToView(PrintDecl(decl), decl);
        return true;
	}
	bool VisitLinkageSpecDecl(clang::LinkageSpecDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        AddToView(PrintDecl(decl), decl);
            return true;
	}
	// We don't want to visit the declarations in a
	// function body (including CXXMethod,CXXConstructor etc.)
	// so we stop traversing.
	bool TraverseFunctionDecl(clang::FunctionDecl* decl)
	{
	    return WalkUpFromFunctionDecl(decl);
	}

    bool TraverseCXXMethodDecl(clang::CXXMethodDecl* decl)
    {
        return WalkUpFromCXXMethodDecl(decl);
    }
    bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl* decl)
    {
        return WalkUpFromCXXConstructorDecl(decl);
    }
    bool TraverseCXXConversionDecl(clang::CXXConversionDecl* decl)
    {
        return WalkUpFromCXXConversionDecl(decl);
    }
    bool TraverseCXXDesctructorDecl(clang::CXXDestructorDecl* decl)
    {
        return WalkUpFromCXXDestructorDecl(decl);
    }
    bool VisitFunctionDecl(clang::FunctionDecl * decl)
	{
	    if (IsOutOfFile(decl))
            return true;

        AddToView(PrintDecl(decl), decl);
		return true;
	}
	bool VisitFieldDecl(clang::FieldDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;

        AddToView(PrintDecl(decl), decl);
		return true;
	}
	bool VisitVarDecl(clang::VarDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        if (clang::isa<ParmVarDecl>(decl))
            return true;
        AddToView(PrintDecl(decl), decl);
		return true;
	}
	bool VisitTypedefDecl(clang::TypedefDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;

        AddToView(PrintDecl(decl), decl);
        return true;

	}
	bool VisitTranslationUnitDecl(clang::TranslationUnitDecl* decl)
	{
	    m_ParentStack.push(m_View.AddNode("",decl, nullptr));
	    return true;
	}
private:
	ASTContext&     m_Ctx;
	SourceManager&  m_SM;
	CodeLayoutView&  m_View;
    PrintingPolicy m_Policy;
    const FileEntry* m_FileEntry;
	std::stack<wxTreeItemId> m_ParentStack;
};



