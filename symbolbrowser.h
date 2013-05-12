#ifndef SYMBOLBROWSER_H_
#define SYMBOLBROWSER_H_

//(*Headers(SymbolBrowser)
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
//*)
#include <clang/AST/Decl.h>
#include <clang/Frontend/CompilerInstance.h>
#pragma push_macro("interface")
#undef interface
#include <clang/Frontend/ASTUnit.h>
#pragma pop_macro("interface")

//Context menu event ids.
extern int idBrowserGoto;
extern int idBrowserGotoDeclaration;
extern int idBrowserGotoDefinition;

using clang::CompilerInstance;
using clang::ASTUnit;
using clang::Decl;
struct SymbolBrowserItemData : public wxTreeItemData
{
    SymbolBrowserItemData(clang::Decl* data):
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
       SymbolBrowserItemData* lpData = static_cast<SymbolBrowserItemData*>(m_Tree->GetItemData(item));
       return (lpData->m_Data == m_Decl);
    }
    wxTreeCtrl* m_Tree;
    clang::Decl* m_Decl;
};
class SymbolBrowser: public wxPanel
{
	public:
		SymbolBrowser(wxWindow* parent);
		virtual ~SymbolBrowser();
        wxTreeItemId AddNode(wxString name,clang::Decl* node, clang::Decl* parent);
        wxTreeItemId AddNode(wxString name,clang::Decl* node, wxTreeItemId parent);
        template<typename T>
        wxTreeItemId FindNode(const T& comparer);
        void SetActiveFile(wxString fileName, ASTUnit* tu,bool fileReparsed = false);
        void Clear();
        const wxTreeCtrl* GetTreeCtrl() { return m_TreeCtrl; }
        void GotoDeclaration(const Decl* decl);
		//(*Declarations(SymbolBrowser)
		wxTreeCtrl* m_TreeCtrl;
		//*)

	protected:

		//(*Identifiers(SymbolBrowser)
		static const long ID_TREECTRL1;
		//*)

	private:
        ASTUnit* m_TU;
        wxString          m_ActiveFile;
        void ShowContextMenu(wxTreeCtrl* tree, wxTreeItemId item);
		//Handlers(SymbolBrowser)
		void OnTreeItemDoubleClicked(wxTreeEvent& event);
		void OnTreeItemRightClicked(wxTreeEvent& event);
		//
		void OnGotoItem(wxCommandEvent& event);
		void OnGotoItemDeclaration(wxCommandEvent& event);
		void OnGotoItemDefinition(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif // SYMBOLBROWSER_H_
