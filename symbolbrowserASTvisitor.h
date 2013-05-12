
#ifndef SYMBOLBROWSERASTVISITOR_H_
#define SYMBOLBROWSERASTVISITOR_H_


#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include "clang_cc.h"
#include <stack>
#include <sstream>
#include "clangcclogger.h"
#include "util.h"
using namespace clang;

class SymbolBrowserASTVisitor	: public clang::RecursiveASTVisitor<SymbolBrowserASTVisitor> {

public:
	SymbolBrowserASTVisitor(SymbolBrowser& browser,ASTContext& ctx,const FileEntry* fileEntry):
	    m_Browser(browser),
        m_Ctx(ctx),
        m_SM(ctx.getSourceManager()),
        m_FileEntry(fileEntry),
        m_Policy(m_Ctx.getLangOpts())
	{
	    m_Policy.Bool = true;
	    m_Policy.SuppressScope=true;
	    m_Policy.TerseOutput =true;
	}
    wxTreeItemId FindParent(clang::Decl* parentDecl)
    {
        wxTreeItemId parentItem;
        while(!m_ParentStack.empty())
        {
            parentItem = m_ParentStack.top();
            SymbolBrowserItemData* lpData = static_cast<SymbolBrowserItemData*>
                                        (m_Browser.GetTreeCtrl()->GetItemData(parentItem));
            if (lpData->m_Data == parentDecl)
                break;
            parentItem.Unset();
            m_ParentStack.pop();
        }
        return parentItem;

    }
    void AddToBrowser(wxString name, clang::Decl* decl)
    {
        DeclContext* dc = decl->getDeclContext();
        Decl* parent = clang::dyn_cast<clang::Decl>(dc);
        wxTreeItemId parentItem = FindParent(parent);
        if (parentItem.IsOk())
            m_ParentStack.push(m_Browser.AddNode(name,decl,parentItem));
        else
            m_ParentStack.push(m_Browser.AddNode(name,decl,parent));

    }
    bool IsOutOfFile(Decl* decl)
    {
        const FileEntry* entry = m_SM.getFileEntryForID(m_SM.getFileID(decl->getLocation()));
        return (entry != m_FileEntry);
    }
	bool VisitRecordDecl(clang::RecordDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        if (clang::isa<CXXRecordDecl>(decl))
            return true;
        wxString name = std2wx(decl->getName());

        AddToBrowser(name, decl);
        return true;

	}
	bool VisitEnumDecl(clang::EnumDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        wxString name = std2wx(decl->getName());

        AddToBrowser(name, decl);
        return true;

	}
	bool VisitEnumConstantDecl(clang::EnumConstantDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        wxString name = std2wx(decl->getName());

        AddToBrowser(name, decl);
        return true;

	}
	bool VisitCXXRecordDecl(CXXRecordDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;

        wxString name = std2wx(decl->getName());

        AddToBrowser(name, decl);
        return true;
	}
	bool VisitClassTemplateDecl(ClassTemplateDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;
        wxString name = std2wx(decl->getName());

        AddToBrowser(name, decl);
        return true;

	}
	bool VisitNamespaceDecl(clang::NamespaceDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;

        wxString name = std2wx(decl->getName());

        if (name.IsEmpty())
            name = _("anonymous ") + std2wx(decl->Decl::getDeclKindName());
        AddToBrowser(name, decl);
        return true;
	}
	bool VisitLinkageSpecDecl(clang::LinkageSpecDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;

        LinkageSpecDecl::LanguageIDs lang = decl->getLanguage();
        wxString name;
        if (lang == clang::LinkageSpecDecl::lang_c)
            name = _("extern \"C\"");
        else
            name = _("extern \"C++\"");

        AddToBrowser(name, decl);
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

		std::ostringstream diagname;
		diagname << decl->getResultType().getAsString(m_Policy) << " " << decl->getNameAsString() <<"(";
		FunctionDecl::param_iterator it = decl->param_begin();
		for (;it != decl->param_end();)
		{
			diagname << (*it)->getType().getAsString(m_Policy)<<" " <<(*it)->getNameAsString();
			if (++it != decl->param_end())
				diagname <<", ";

		}
		diagname << ")";
        wxString name = std2wx(diagname.str());

        AddToBrowser(name, decl);
		return true;
	}
	bool VisitFieldDecl(clang::FieldDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;

        std::ostringstream diagname;
        diagname << decl->getType().getAsString(m_Policy) <<" "<< decl->getNameAsString();
	    wxString name = std2wx(diagname.str());

        AddToBrowser(name, decl);
        return true;

	}
	bool VisitVarDecl(clang::VarDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;

        std::ostringstream diagname;
        diagname << decl->getType().getAsString(m_Policy) <<" "<< decl->getNameAsString();
	    wxString name = std2wx(diagname.str());

        AddToBrowser(name, decl);
        return true;

	}
	bool VisitTypedefDecl(clang::TypedefDecl* decl)
	{
	    if (IsOutOfFile(decl))
            return true;

	    std::ostringstream diagname;
	    diagname << decl->getUnderlyingType().getAsString(m_Policy)<<" "<<decl->getNameAsString();
        wxString name = std2wx(diagname.str());

        AddToBrowser(name, decl);
        return true;

	}
	bool VisitTranslationUnitDecl(clang::TranslationUnitDecl* decl)
	{
	    m_ParentStack.push(m_Browser.AddNode(_T(""),decl,0));
	    return true;
	}
private:
	ASTContext&     m_Ctx;
	SourceManager&  m_SM;
	SymbolBrowser&  m_Browser;
    PrintingPolicy m_Policy;
    const FileEntry* m_FileEntry;
	std::stack<wxTreeItemId> m_ParentStack;
};




#endif // SYMBOLBROWSERASTVISITOR_H_
