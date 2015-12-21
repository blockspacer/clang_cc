#pragma once


#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include "clangcommon.h"


//Context menu event ids.
extern const int idCodeLayoutViewGoto;
extern const int idCodeLayoutViewGotoDeclaration;
extern const int idCodeLayoutViewGotoDefinition;

using clang::CompilerInstance;
using clang::ASTUnit;
using clang::Decl;
class TranslationUnitManager;
class ccEvent;
struct CodeLayoutViewItemData : public wxTreeItemData
{
    CodeLayoutViewItemData(clang::Decl* data):
        m_Data(data)
    {}
    clang::Decl* m_Data;
};
struct ItemDataComparer
{
    ItemDataComparer(wxTreeCtrl* tree,clang::Decl* decl):
        m_Tree(tree),
        m_Decl(decl)
    {}
    bool operator() (const wxTreeItemId& item) const
    {
       CodeLayoutViewItemData* lpData = static_cast<CodeLayoutViewItemData*>(m_Tree->GetItemData(item));
       return (lpData->m_Data == m_Decl);
    }
    wxTreeCtrl* m_Tree;
    clang::Decl* m_Decl;
};
class CodeLayoutView: public wxPanel
{
public:
    CodeLayoutView(wxWindow* parent, TranslationUnitManager& tm);
    virtual ~CodeLayoutView();
    wxTreeItemId AddNode(wxString name,clang::Decl* node, clang::Decl* parent);
    wxTreeItemId AddNode(wxString name,clang::Decl* node, wxTreeItemId parent);
    template<typename T>
    wxTreeItemId FindNode(const T& comparer);
    bool IsChildItem(const wxTreeItemId& child, const wxTreeItemId& parent);
    void SetActiveFile(wxString fileName, ASTUnit* tu,bool fileReparsed = false);
    void Clear();
    const wxTreeCtrl* GetTreeCtrl() { return m_TreeCtrl; }

protected:
    static const long ID_TREECTRL1;
    wxTreeCtrl* m_TreeCtrl;

private:
    void ShowContextMenu(wxTreeCtrl* tree, wxTreeItemId item);
    //Handlers LayoutView
    void OnTreeItemDoubleClicked(wxTreeEvent& event);
    void OnTreeItemRightClicked(wxTreeEvent& event);
    //
    void OnGotoItem(wxCommandEvent& event);
    void OnGotoItemDeclaration(wxCommandEvent& event);
    void OnGotoItemDefinition(wxCommandEvent& event);
    void OnSelectInEditor(wxCommandEvent& event);
    //
    void OnParseEnd(ccEvent& event);
    ASTUnit* m_TU;
    wxString m_ActiveFile;
    wxTreeItemId m_ItemDragged;
    TranslationUnitManager& m_TUManager;
    DECLARE_EVENT_TABLE()
};

