#include "codelayoutview.h"
#include <wx/treectrl.h>
#include <wx/imaglist.h>
#include <wx/xrc/xh_all.h>
#include <manager.h>
#include <logmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <cbstyledtextctrl.h>
#include "codelayoutASTvisitor.h"
#include "contextmenubuilder.h"
#include "ccevents.h"
#include "translationunitmanager.h"
#include <llvm/ADT/StringRef.h>


#include <wx/intl.h>
#include <wx/string.h>

#include "astnodeutil.h"
#include "util.h"



const long CodeLayoutView::ID_TREECTRL1 = wxNewId();

const int idCodeLayoutViewGoto = wxNewId();
const int idCodeLayoutViewGotoDeclaration = wxNewId();
const int idCodeLayoutViewGotoDefinition = wxNewId();
const int idCodeLayoutViewSelectInEditor = wxNewId();


BEGIN_EVENT_TABLE(CodeLayoutView,wxPanel)
    EVT_TREE_ITEM_ACTIVATED(ID_TREECTRL1, CodeLayoutView::OnTreeItemDoubleClicked)
    EVT_TREE_ITEM_RIGHT_CLICK(ID_TREECTRL1, CodeLayoutView::OnTreeItemRightClicked)
    EVT_MENU(idCodeLayoutViewGoto,  CodeLayoutView::OnGotoItem)
    EVT_MENU(idCodeLayoutViewGotoDeclaration, CodeLayoutView::OnGotoItemDeclaration)
    EVT_MENU(idCodeLayoutViewGotoDefinition, CodeLayoutView::OnGotoItemDefinition)
    EVT_MENU(idCodeLayoutViewSelectInEditor, CodeLayoutView::OnSelectInEditor)
END_EVENT_TABLE()

using namespace clang;
CodeLayoutView::CodeLayoutView(wxWindow* parent, TranslationUnitManager& tm):
    wxPanel(parent), m_TUManager(tm)
{
	//Initialize(LayoutView)
	wxBoxSizer* BoxSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxSize(284,296), wxTAB_TRAVERSAL, "wxID_ANY");
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	m_TreeCtrl = new wxTreeCtrl(this, ID_TREECTRL1, wxDefaultPosition, wxSize(273,283), wxTR_HIDE_ROOT|wxTR_DEFAULT_STYLE, wxDefaultValidator, "ID_TREECTRL1");
	BoxSizer1->Add(m_TreeCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	Layout();
	//
	wxImageList* imglist = new wxImageList(16,16,true,0);
	imglist->Add(wxXmlResource::Get()->LoadBitmap("browser_images"));
	m_TreeCtrl->SetImageList(imglist);
	m_TreeCtrl->AddRoot("root");
	//Setup events

	GetParent()->Bind(ccEVT_PARSE_END,&CodeLayoutView::OnParseEnd,this);
	GetParent()->Bind(ccEVT_REPARSE_END,&CodeLayoutView::OnParseEnd,this);

}

CodeLayoutView::~CodeLayoutView()
{
    GetParent()->Unbind(ccEVT_PARSE_END,&CodeLayoutView::OnParseEnd,this);
    GetParent()->Unbind(ccEVT_REPARSE_END,&CodeLayoutView::OnParseEnd,this);
}
wxTreeItemId CodeLayoutView::AddNode(wxString name,clang::Decl* node, clang::Decl* parent)
{
    int image = GetImageIndexForDeclaration(node);

    if (parent == nullptr)
    {
        m_TreeCtrl->SetItemData(m_TreeCtrl->GetRootItem(),new CodeLayoutViewItemData(node));
        return m_TreeCtrl->GetRootItem();
    }
    if (parent->getKind() == Decl::TranslationUnit)
    {

       return m_TreeCtrl->AppendItem(m_TreeCtrl->GetRootItem(),name,image,image,new CodeLayoutViewItemData(node));

    }
    else
    {
       ItemDataComparer comparer(m_TreeCtrl,parent);
       wxTreeItemId parentNode = FindNode(comparer);
       if (!parentNode.IsOk())
       {
           RawwxStringOstream out;
           PrintingPolicy policy(m_TU->getASTContext().getLangOpts());
           CodeLayoutDeclarationPrinter printer(out, policy);
           printer.Visit(parent);
           wxString parentName = out.str();
           //wxString parentName= wxString::FromAscii(parent->getDeclKindName());
           //parentName += "-Bad Parent";
           parentNode = m_TreeCtrl->AppendItem(m_TreeCtrl->GetRootItem(),parentName,GetImageIndexForDeclaration(parent),
                                               GetImageIndexForDeclaration(parent),new CodeLayoutViewItemData(parent));
       }
       wxTreeItemId item = m_TreeCtrl->AppendItem(parentNode,name,image,image,new CodeLayoutViewItemData(node));
       m_TreeCtrl->Expand(parentNode);
       return item;
    }
}
wxTreeItemId CodeLayoutView::AddNode(wxString name,clang::Decl* node, wxTreeItemId parent)
{
    int image = GetImageIndexForDeclaration(node);

    wxTreeItemId item = m_TreeCtrl->AppendItem(parent,name,image,image,new CodeLayoutViewItemData(node));
    if (parent != m_TreeCtrl->GetRootItem())
        m_TreeCtrl->Expand(parent);
    return item;

}
template<typename T>
wxTreeItemId CodeLayoutView::FindNode(const T& comparer)
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
void CodeLayoutView::Clear()
{
    m_TreeCtrl->DeleteChildren(m_TreeCtrl->GetRootItem());
}
void CodeLayoutView::SetActiveFile(wxString fileName, ASTUnit* tu,bool fileReparsed)
{
      cbAssert(!fileName.IsEmpty() && tu);
      if (fileName != m_ActiveFile || m_TU != tu || fileReparsed )
      {
          m_ActiveFile = fileName;
          m_TU = tu;
          m_TreeCtrl->Freeze();
          Clear();
          const FileEntry* entry = tu->getFileManager().getFile(wx2std(fileName));
          CodeLayoutASTVisitor visitor(*this, tu->getASTContext(), entry);
          visitor.TraverseDecl(tu->getASTContext().getTranslationUnitDecl());
          m_TreeCtrl->Thaw();
      }
}

void CodeLayoutView::OnTreeItemDoubleClicked(wxTreeEvent& event)
{
    wxTreeCtrl* tree = static_cast<wxTreeCtrl*> (event.GetEventObject());
    wxTreeItemId item = event.GetItem();
    CodeLayoutViewItemData* itemData = static_cast<CodeLayoutViewItemData*>(tree->GetItemData(item));
    ASTNode::GotoDeclarationInEditor(itemData->m_Data);
}

void CodeLayoutView::OnTreeItemRightClicked(wxTreeEvent& event)
{
    wxTreeCtrl* tree = (wxTreeCtrl*)event.GetEventObject();
    if (!tree)
        return;
    ShowContextMenu(tree, event.GetItem());
}

bool CodeLayoutView::IsChildItem(const wxTreeItemId& child, const wxTreeItemId& parent)
{
    wxTreeItemId immediateParent = m_TreeCtrl->GetItemParent(child);
    if (immediateParent == parent)
        return true;
    else if (immediateParent == m_TreeCtrl->GetRootItem())
        return false;
    else
        return IsChildItem(immediateParent, parent);

}
void CodeLayoutView::ShowContextMenu(wxTreeCtrl* tree, wxTreeItemId item)
{
    CodeLayoutViewItemData* itemData = static_cast<CodeLayoutViewItemData*>(tree->GetItemData(item));
    std::unique_ptr<wxMenu> menu(new wxMenu(wxEmptyString));
    Decl* decl = itemData->m_Data;
    if (item.IsOk() && decl)
    {
        menu->Append(idCodeLayoutViewGoto, "Goto");
        menu->Append(idCodeLayoutViewSelectInEditor, "Select in Editor");
        const Decl* definition= ASTNode::GetDefinition(decl);
        if(definition && definition != decl)
            menu->Append(idCodeLayoutViewGotoDefinition, "Goto Definition");
        else if(decl->getPreviousDecl())
            menu->Append(idCodeLayoutViewGotoDeclaration, "Goto Declaration");

    }
    if (menu->GetMenuItemCount())
        PopupMenu(menu.get());
}
void CodeLayoutView::OnGotoItem(wxCommandEvent& event)
{
   wxTreeItemId item = m_TreeCtrl->GetSelection();
   CodeLayoutViewItemData* itemData = static_cast<CodeLayoutViewItemData*>(m_TreeCtrl->GetItemData(item));
   Decl* decl = itemData->m_Data;
   if(decl)
   {
        ASTNode::GotoDeclarationInEditor(decl);
   }
}
void CodeLayoutView::OnGotoItemDeclaration(wxCommandEvent& event)
{
   wxTreeItemId item = m_TreeCtrl->GetSelection();
   CodeLayoutViewItemData* itemData = static_cast<CodeLayoutViewItemData*>(m_TreeCtrl->GetItemData(item));
   Decl* decl = itemData->m_Data;
   if(decl)
   {
       Decl* prev = decl->getPreviousDecl();
       ASTNode::GotoDeclarationInEditor(prev);
   }
}
void CodeLayoutView::OnGotoItemDefinition(wxCommandEvent& event)
{
   wxTreeItemId item = m_TreeCtrl->GetSelection();
   CodeLayoutViewItemData* itemData = static_cast<CodeLayoutViewItemData*>(m_TreeCtrl->GetItemData(item));
   Decl* decl = itemData->m_Data;
   if(decl)
   {
        const Decl* definition = ASTNode::GetDefinition(decl);
        ASTNode::GotoDeclarationInEditor(definition);
   }
}
void CodeLayoutView::OnSelectInEditor(wxCommandEvent& event)
{
   wxTreeItemId item = m_TreeCtrl->GetSelection();
   CodeLayoutViewItemData* itemData = static_cast<CodeLayoutViewItemData*>(m_TreeCtrl->GetItemData(item));
   Decl* decl = itemData->m_Data;
   if(decl)
   {
        ASTNode::SelectDeclarationInEditor(decl);
   }
}
void CodeLayoutView::OnParseEnd(ccEvent& event)
{
    TRACE(L"Parse End received in CodeLayouView::OnParseEend");
    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    ASTUnit* tu = event.GetTranslationUnit();
    if (!editor || !tu)
        return;
    bool isReparse = event.GetEventType() == ccEVT_REPARSE_END;
    ProjectFile* projFile = event.GetProjectFile();
    wxString pairedFilename;
    if(projFile)
    {
        if (ProjectFile* pairedFile = GetProjectFilePair(projFile))
            pairedFilename = pairedFile->file.GetFullPath();
    }

    wxString fileName = event.GetFileName();
    wxString editorFilename = editor->GetFilename();

    if (editorFilename == fileName || editorFilename == pairedFilename)
        SetActiveFile(editorFilename, tu, isReparse);

    event.Skip();
}

