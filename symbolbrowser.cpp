#include "symbolbrowser.h"
#include <wx/treectrl.h>
#include <wx/imaglist.h>
#include <wx/xrc/xh_all.h>
#include <manager.h>
#include <logmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <cbstyledtextctrl.h>
#include "symbolbrowserASTvisitor.h"
#include "contextmenubuilder.h"
#include <llvm/ADT/StringRef.h>


#include <wx/intl.h>
#include <wx/string.h>



const long SymbolBrowser::ID_TREECTRL1 = wxNewId();

int idBrowserGoto = wxNewId();
int idBrowserGotoDeclaration = wxNewId();
int idBrowserGotoDefinition = wxNewId();


BEGIN_EVENT_TABLE(SymbolBrowser,wxPanel)
    EVT_TREE_ITEM_ACTIVATED  (ID_TREECTRL1,      SymbolBrowser::OnTreeItemDoubleClicked)
    EVT_TREE_ITEM_RIGHT_CLICK (ID_TREECTRL1,     SymbolBrowser::OnTreeItemRightClicked)
    EVT_MENU (idBrowserGoto,  SymbolBrowser::OnGotoItem)
    EVT_MENU (idBrowserGotoDeclaration,  SymbolBrowser::OnGotoItemDeclaration)
    EVT_MENU (idBrowserGotoDefinition,  SymbolBrowser::OnGotoItemDefinition)
END_EVENT_TABLE()

using namespace clang;
SymbolBrowser::SymbolBrowser(wxWindow* parent)
{
	//(*Initialize(SymbolBrowser)
	wxBoxSizer* BoxSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxSize(284,296), wxTAB_TRAVERSAL, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	m_TreeCtrl = new wxTreeCtrl(this, ID_TREECTRL1, wxDefaultPosition, wxSize(273,283), wxTR_HIDE_ROOT|wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
	BoxSizer1->Add(m_TreeCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	Layout();
	//*)
	wxImageList* imglist = new wxImageList(16,16,true,0);
	imglist->Add(wxXmlResource::Get()->LoadBitmap(_("browser_images")));
	m_TreeCtrl->SetImageList(imglist);
	m_TreeCtrl->AddRoot(_T("root"));
}

SymbolBrowser::~SymbolBrowser()
{
	//(*Destroy(SymbolBrowser)
	//*)
}
wxTreeItemId SymbolBrowser::AddNode(wxString name,clang::Decl* node, clang::Decl* parent)
{
    int image = GetImageIndexForDeclaration(node);

    if (parent == nullptr)
    {
        m_TreeCtrl->SetItemData(m_TreeCtrl->GetRootItem(),new SymbolBrowserItemData(node));
        return m_TreeCtrl->GetRootItem();
    }
    if (parent->getKind() == Decl::TranslationUnit)
    {
       return m_TreeCtrl->AppendItem(m_TreeCtrl->GetRootItem(),name,image,image,new SymbolBrowserItemData(node));
    }
    else
    {
       ItemDataComparer comparer(m_TreeCtrl,parent);
       wxTreeItemId parentNode = FindNode(comparer);
       if (!parentNode.IsOk())
       {
           wxString parentName= wxString::FromAscii(parent->getDeclKindName());
           parentName += _("-Bad Parent");
           parentNode = m_TreeCtrl->AppendItem(m_TreeCtrl->GetRootItem(),parentName,GetImageIndexForDeclaration(parent),
                                               GetImageIndexForDeclaration(parent),new SymbolBrowserItemData(parent));
       }
       m_TreeCtrl->AppendItem(parentNode,name,image,image,new SymbolBrowserItemData(node));
       return parentNode;
    }
}
wxTreeItemId SymbolBrowser::AddNode(wxString name,clang::Decl* node, wxTreeItemId parent)
{
    int image = GetImageIndexForDeclaration(node);
    return m_TreeCtrl->AppendItem(parent,name,image,image,new SymbolBrowserItemData(node));
}
template<typename T>
wxTreeItemId SymbolBrowser::FindNode(const T& comparer)
{
  	std::stack<wxTreeItemId> items;
	if (m_TreeCtrl->GetRootItem().IsOk())
		items.push(m_TreeCtrl->GetRootItem());

	while (!items.empty())
	{
		wxTreeItemId next = items.top();
		items.pop();

		if (next != m_TreeCtrl->GetRootItem() && comparer(next))
			return next;

		wxTreeItemIdValue cookie;
		wxTreeItemId nextChild = m_TreeCtrl->GetFirstChild(next, cookie);
		while (nextChild.IsOk())
		{
			items.push(nextChild);
			nextChild = m_TreeCtrl->GetNextSibling(nextChild);
		}
	}

	return wxTreeItemId();
}
void SymbolBrowser::Clear()
{
    m_TreeCtrl->DeleteChildren(m_TreeCtrl->GetRootItem());
}
void SymbolBrowser::SetActiveFile(wxString fileName, ASTUnit* tu,bool fileReparsed)
{
      cbAssert(!fileName.IsEmpty() && tu);
      if (fileName != m_ActiveFile || m_TU != tu || fileReparsed )
      {
          m_ActiveFile = fileName;
          m_TU = tu;
          m_TreeCtrl->Freeze();
          Clear();
          const FileEntry* entry = tu->getFileManager().getFile(wx2std(fileName));
          SymbolBrowserASTVisitor visitor(*this,tu->getASTContext(),entry);
          visitor.TraverseDecl(tu->getASTContext().getTranslationUnitDecl());
          m_TreeCtrl->Thaw();
      }


}
void SymbolBrowser::GotoDeclaration(const Decl* decl)
{
    if (decl)
    {
        FullSourceLoc loc = m_TU->getASTContext().getFullLoc(decl->getLocStart());

		if (loc.isValid())
        {
            StringRef fileName;
            unsigned line,col;
            if (loc.isFileID())
            {
                fileName = loc.getManager().getFilename(loc);
                line= loc.getSpellingLineNumber();
                col = loc.getSpellingColumnNumber();
            }
            else if (loc.isMacroID())
            {
               FullSourceLoc expansionLoc = loc.getExpansionLoc();
               fileName = expansionLoc.getManager().getFilename(expansionLoc);
               line= expansionLoc.getExpansionLineNumber();
               col = expansionLoc.getExpansionColumnNumber();
            }
            wxString wxFileName = std2wx(fileName);
            cbEditor* ed = Manager::Get()->GetEditorManager()->Open(wxFileName);
            if (!ed)
                return;
            ed->GotoLine(line-1);
            int startPos = ed->GetControl()->GetCurrentPos();
            ed->GetControl()->GotoPos(startPos + col -1);
            ed->SetFocus();
        }
    }
}
void SymbolBrowser::OnTreeItemDoubleClicked(wxTreeEvent& event)
{
    wxTreeCtrl* tree = static_cast<wxTreeCtrl*> (event.GetEventObject());
    wxTreeItemId item = event.GetItem();
    SymbolBrowserItemData* itemData = static_cast<SymbolBrowserItemData*>(tree->GetItemData(item));
    GotoDeclaration(itemData->m_Data);
}

void SymbolBrowser::OnTreeItemRightClicked(wxTreeEvent& event)
{
    wxTreeCtrl* tree = (wxTreeCtrl*)event.GetEventObject();
    if (!tree)
        return;
    ShowContextMenu(tree, event.GetItem());
}
void SymbolBrowser::ShowContextMenu(wxTreeCtrl* tree, wxTreeItemId item)
{
    SymbolBrowserItemData* itemData = static_cast<SymbolBrowserItemData*>(tree->GetItemData(item));
    std::unique_ptr<wxMenu> menu(new wxMenu(wxEmptyString));
    Decl* decl = itemData->m_Data;
    if (item.IsOk() && decl)
    {
        menu->Append(idBrowserGoto, _T("Goto"));
        ContextMenuBuilder builder(menu.get());
        builder.DeclVisitor::Visit(decl);
    }
    if (menu->GetMenuItemCount())
        PopupMenu(menu.get());
}
void SymbolBrowser::OnGotoItem(wxCommandEvent& event)
{
   wxTreeItemId item = m_TreeCtrl->GetSelection();
   SymbolBrowserItemData* itemData = static_cast<SymbolBrowserItemData*>(m_TreeCtrl->GetItemData(item));
   Decl* decl = itemData->m_Data;
   if(decl)
   {
       GotoDeclaration(decl);
   }
}
void SymbolBrowser::OnGotoItemDeclaration(wxCommandEvent& event)
{
   wxTreeItemId item = m_TreeCtrl->GetSelection();
   SymbolBrowserItemData* itemData = static_cast<SymbolBrowserItemData*>(m_TreeCtrl->GetItemData(item));
   Decl* decl = itemData->m_Data;
   if(decl)
   {
       Decl* prev = decl->getPreviousDecl();
       GotoDeclaration(prev);
   }
}
void SymbolBrowser::OnGotoItemDefinition(wxCommandEvent& event)
{
   wxTreeItemId item = m_TreeCtrl->GetSelection();
   SymbolBrowserItemData* itemData = static_cast<SymbolBrowserItemData*>(m_TreeCtrl->GetItemData(item));
   Decl* decl = itemData->m_Data;
   if(decl)
   {
       if (FunctionDecl* fdecl = clang::dyn_cast<FunctionDecl>(decl))
       {
           const FunctionDecl* functionDefinition;
           fdecl->hasBody(functionDefinition);
           GotoDeclaration(functionDefinition);
       }
       else if(TagDecl* tdecl = clang::dyn_cast<TagDecl>(decl))
       {
           GotoDeclaration(tdecl->getDefinition());
       }
   }
}
