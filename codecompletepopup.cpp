// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include "codecompletepopup.h"
#include "autocomplist.h"
#include "cbstyledtextctrl.h"
#include "options.h"
#include "clangcclogger.h"
#include "stringio.h"
#include "util.h"
#include <algorithm>
#include <wx/imaglist.h>
#include <wx/xrc/xmlres.h>

#define BOOST_RANGE_ENABLE_CONCEPT_ASSERT 0
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/bind.hpp>





using namespace clang;
static int  idAutoCompList = wxNewId();

constexpr int CodeCompletePopupWindow::m_PriorityArray[3];
BEGIN_EVENT_TABLE(CodeCompletePopupWindow, wxPopupWindow)
	EVT_LIST_ITEM_ACTIVATED(idAutoCompList, CodeCompletePopupWindow::OnItemActivated)
	EVT_LIST_ITEM_SELECTED(idAutoCompList, CodeCompletePopupWindow::OnItemSelected)
END_EVENT_TABLE()

CodeCompletePopupWindow::CodeCompletePopupWindow(wxWindow* parent):
    wxPopupWindow(parent)
{
	m_ToolbarImages = new wxImageList(16,16,true,0);
	m_ToolbarImages->Add(wxXmlResource::Get()->LoadBitmap(_("browser_images")));

	//CreateToolBar();

	m_CompleteListCtrl = new AutoCompList(this,idAutoCompList);

	m_CompleteListCtrl->Connect(wxEVT_ENTER_WINDOW,wxMouseEventHandler(CodeCompletePopupWindow::OnCompletionListMouseOver),
		nullptr,this);
    m_CompleteListCtrl->Connect(wxEVT_SIZE,wxSizeEventHandler(CodeCompletePopupWindow::OnCompletionListSize),
        nullptr,this);

	wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );
	topSizer->Add( m_CompleteListCtrl, 1, wxEXPAND );
//	topSizer->Add( m_FilterToolBar,0,wxEXPAND|wxDOWN);

    this->SetSizerAndFit(topSizer);
    SetClientSize(m_CompleteListCtrl->GetSize());
}

void CodeCompletePopupWindow::OnItemActivated(wxListEvent& event)
{
    InsertCompletionString();
    DelayedDeactivate();
    event.Skip();
}
void CodeCompletePopupWindow::OnItemSelected(wxListEvent& event)
{
	m_CompleteListCtrl->EnsureVisible(event.m_itemIndex);
	event.Skip();
}
void CodeCompletePopupWindow::OnCharAdded(wxScintillaEvent& event)
{
	SetFilter(GetWordAtCursor());
	event.Skip();
}
void CodeCompletePopupWindow::OnMouse(wxMouseEvent &event)
{
	event.Skip();
}
void CodeCompletePopupWindow::OnCompletionListMouseOver(wxMouseEvent &event)
{
	event.Skip();
}
void CodeCompletePopupWindow::OnKillFocus(wxFocusEvent &event)
{
	//this->Hide();
}
void CodeCompletePopupWindow::OnKeyDown(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
    {
        case WXK_UP:
            m_CompleteListCtrl->Select(m_CompleteListCtrl->GetFirstSelected()-1);
            return;
        case WXK_DOWN:
            m_CompleteListCtrl->Select(m_CompleteListCtrl->GetFirstSelected()+1);
            return;
        case WXK_PAGEUP:
            {int item = std::max<int>(0, m_CompleteListCtrl->GetFirstSelected() - ROW_COUNT);
            m_CompleteListCtrl->Select(item);}
            return;
        case WXK_PAGEDOWN:
            {int item = std::min<int>(m_CompleteListCtrl->GetItemCount() - 1,
                                m_CompleteListCtrl->GetFirstSelected() + ROW_COUNT);
            m_CompleteListCtrl->Select(item);}
            return;
        case WXK_TAB:
        case WXK_RETURN:
            InsertCompletionString();
            DelayedDeactivate();
            return;
        case WXK_BACK:
        case WXK_DELETE:
            SetFilter(GetWordAtCursor().RemoveLast());
            break;
        case WXK_ESCAPE:
            DelayedDeactivate();
            break;
        case WXK_SPACE:
            //FIXME this should be Ctrl+Space but
            //Space key is not received when Ctrl is pressed.Only the recurrent Ctrl events.
            //maybe existing hooks by editor manager or another plugin?
            if(event.GetModifiers() == wxMOD_SHIFT)
            {
               m_CurrentPriorityIndex = sizeof(m_PriorityArray) - 1;
               SetFilter(GetWordAtCursor());
               return;
            }
            else
                DelayedDeactivate();
                break;
        default :
            break;

    }
   //We don't let the pro
   event.Skip();

}
void CodeCompletePopupWindow::OnCompletionListSize(wxSizeEvent& event)
{
    SetClientSize(event.GetSize());

    //Reposition the window
    if (m_Scintilla)
        Position(m_ScreenPoint, wxSize(0, m_Scintilla->TextHeight(m_Scintilla->GetCurrentLine())));
	event.Skip();
}
void CodeCompletePopupWindow::SetEditor(cbEditor* editor)
{
    if (IsActive())
    {
        DeActivate();
    }
    cbAssert(editor && editor->IsOK() && "Invalid Editor");
    m_Editor = editor;
    m_Scintilla = editor->GetControl();
}
bool CodeCompletePopupWindow::Activate()
{
    cbAssert(m_Editor && m_Editor->IsOK() && "Invalid Editor");
    SetFilter(GetWordAtCursor());

    if (m_FilteredItems.empty()) //Nothing to show
        return false;

    m_Scintilla->Connect(wxEVT_KEY_DOWN,wxKeyEventHandler(CodeCompletePopupWindow::OnKeyDown),
		nullptr, this);
	m_Scintilla->Connect(wxEVT_KILL_FOCUS,wxFocusEventHandler(CodeCompletePopupWindow::OnKillFocus),
		nullptr, this);
	m_Scintilla->Connect(wxEVT_SCI_CHARADDED,wxScintillaEventHandler(CodeCompletePopupWindow::OnCharAdded),
		nullptr,this);
	m_Scintilla->Connect(wxEVT_LEFT_DOWN,wxMouseEventHandler(CodeCompletePopupWindow::OnMouse),
		nullptr, this);
	m_Scintilla->Connect(wxEVT_RIGHT_DOWN,wxMouseEventHandler(CodeCompletePopupWindow::OnMouse),
		nullptr, this);

	wxPoint pnt = m_Scintilla->PointFromPosition(m_Scintilla->GetCurrentPos());
	m_ScreenPoint = m_Scintilla->ClientToScreen(pnt);
	Position(m_ScreenPoint, wxSize(0, m_Scintilla->TextHeight(m_Scintilla->GetCurrentLine())));
	Show();
	m_Active = true;
	return true;
//TODO
//	wxPoint screenpnt = this->GetScreenPosition();
//	int  height  = this->GetSize().GetY();
//	screenpnt.y += height;
//	m_FilterPopup->Position(screenpnt,wxSize());
//	m_FilterPopup->Show();

}
void CodeCompletePopupWindow::DeActivate()
{
    m_Scintilla->Disconnect(wxEVT_KEY_DOWN,wxKeyEventHandler(CodeCompletePopupWindow::OnKeyDown),
		nullptr, this);
	m_Scintilla->Disconnect(wxEVT_KILL_FOCUS,wxFocusEventHandler(CodeCompletePopupWindow::OnKillFocus),
		nullptr, this);
	m_Scintilla->Disconnect(wxEVT_SCI_CHARADDED,wxScintillaEventHandler(CodeCompletePopupWindow::OnCharAdded),
		nullptr,this);
	m_Scintilla->Disconnect(wxEVT_LEFT_DOWN,wxMouseEventHandler(CodeCompletePopupWindow::OnMouse),
		nullptr, this);
	m_Scintilla->Disconnect(wxEVT_RIGHT_DOWN,wxMouseEventHandler(CodeCompletePopupWindow::OnMouse),
		nullptr, this);
    if (IsShownOnScreen())
    {
        Hide();
    }
    m_Active = false;
}
void CodeCompletePopupWindow::DelayedDeactivate()
{
#if !wxCHECK_VERSION(2, 9, 5)
    Connect(wxEVT_IDLE, wxIdleEventHandler(CodeCompletePopupWindow::OnIdle));
#else
    CallAfter(&CodeCompletePopupWindow::DeActivate);
#endif
}
void CodeCompletePopupWindow::OnIdle(wxIdleEvent& event)
{
    DeActivate();
    Disconnect(wxEVT_IDLE, wxIdleEventHandler(CodeCompletePopupWindow::OnIdle));
}
bool CodeCompletePopupWindow::IsActive()
{
   return m_Active && IsShownOnScreen();
}
//Just in case
bool CodeCompletePopupWindow::AcceptsFocusRecursively()
{
	return false;
}
//Just in case
bool CodeCompletePopupWindow::AcceptsFocus()
{
	return false;
}
wxString CodeCompletePopupWindow::GetWordAtCursor()
{
    int pos = m_Scintilla->GetCurrentPos();
	int start = m_Scintilla->WordStartPosition(pos, true);
	return m_Scintilla->GetTextRange(start, pos);
}wxString CodeCompletePopupWindow::GetWholeWordAtCursor()
{

	int pos = m_Scintilla->GetCurrentPos();
	int start = m_Scintilla->WordStartPosition(pos, true);
	int end   = m_Scintilla->WordEndPosition(pos, true);
	return m_Scintilla->GetTextRange(start, end);
}

void CodeCompletePopupWindow::InsertCompletionString()
{
    long index = m_CompleteListCtrl->GetFirstSelected();
    if (index != wxNOT_FOUND)
    {
        int pos = m_Scintilla->GetCurrentPos();
        int start = m_Scintilla->WordStartPosition(pos, true);
        m_Scintilla->SetTargetStart(start);
        m_Scintilla->SetTargetEnd(pos);
        auto ccs = m_FilteredItems[index].m_Ccs;
        auto ccr = m_FilteredItems[index].m_Ccr;
        unsigned caretPosFound = 0;
        wxString textToInsert;
        for (auto& chunk : *ccs)
        {
            switch (chunk.Kind)
            {   //TODO Consider other possibilities
                case CodeCompletionString::CK_LeftParen:
                case CodeCompletionString::CK_RightParen:
                case CodeCompletionString::CK_LeftAngle:
                case CodeCompletionString::CK_RightAngle:
                    if (!caretPosFound)
                    {
                        caretPosFound = textToInsert.Length() + 1;
                    }
                    textToInsert << std2wx(chunk.Text);
                    break;
                case CodeCompletionString::CK_TypedText:
                    textToInsert << std2wx(chunk.Text);
                    break;
                default :
                    break;
            }
        }
        m_Scintilla->ReplaceTarget(textToInsert);
        if (caretPosFound)
            m_Scintilla->GotoPos(start + caretPosFound);
        else
            m_Scintilla->GotoPos(start + textToInsert.Length());
    }
}
//TODO Make sure you are gonna complete this.
void CodeCompletePopupWindow::CreateToolBar()
{
	m_FilterPopup = new wxPopupWindow(this);
	m_FilterToolBar = new wxToolBar(m_FilterPopup, wxID_ANY, wxDefaultPosition, wxDefaultSize,wxTB_FLAT);
	m_FilterToolBar->AddTool(wxID_ANY,_("Label"),m_ToolbarImages->GetBitmap(0));
	m_FilterToolBar->AddTool(wxID_ANY,_("Label"),m_ToolbarImages->GetBitmap(3));
	m_FilterToolBar->AddTool(wxID_ANY,_("Label"),m_ToolbarImages->GetBitmap(5));
	m_FilterToolBar->AddTool(wxID_ANY,_("Label"),m_ToolbarImages->GetBitmap(8));
	m_FilterToolBar->AddTool(wxID_ANY,_("Label"),m_ToolbarImages->GetBitmap(11));
	m_FilterToolBar->AddTool(wxID_ANY,_("Label"),m_ToolbarImages->GetBitmap(13));
	m_FilterToolBar->AddTool(wxID_ANY,_("Label"),m_ToolbarImages->GetBitmap(15));
	m_FilterToolBar->AddTool(wxID_ANY,_("Label"),m_ToolbarImages->GetBitmap(18));
	m_FilterToolBar->AddTool(wxID_ANY,_("Label"),m_ToolbarImages->GetBitmap(21));
	m_FilterToolBar->Realize();
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(m_FilterToolBar,0);
	m_FilterPopup->SetSizerAndFit(sizer);

}
CodeCompletePopupWindow::~CodeCompletePopupWindow()
{
	//delete m_ToolbarImages;
}
void CodeCompletePopupWindow::SetFilter(wxString filterString)
{
	using boost::adaptors::filtered;
	auto filter = Options::Get().MakeStringFilter(filterString);
	auto priorityFilter = [this](const CodeCompleteResultHolder& res)
                             { return res.m_Ccr.Priority <= m_PriorityArray[m_CurrentPriorityIndex] ;};

    auto  range = m_Items | filtered(priorityFilter) | filtered(filter);

    if (range.empty())
    {
        ++m_CurrentPriorityIndex %=sizeof(m_PriorityArray);
        // Try with more More options
        if (m_CurrentPriorityIndex != 0)
            SetFilter(filterString);
        //Nothing to show
        else
        {
            DelayedDeactivate();
            m_FilteredItems.clear();
            m_CompleteListCtrl->SetItemCount(0);
            return;
        }
    }
    //We have results to show.
    else
    {
        m_FilteredItems = boost::copy_range<std::vector<CodeCompleteResultHolder>>(range);
        m_CompleteListCtrl->SetItems(CreateListStrings(range));
    }
}
int CodeCompletePopupWindow::GetFilteredImageIndex(long item)
{
   cbAssert(item >= 0 && "index must be positive");
   return GetImageIndexForCompletionResult(m_FilteredItems[item].m_Ccr,
                                           m_FilteredItems[item].m_Access);
}
void CodeCompletePopupWindow::SetItems(std::vector<CodeCompleteResultHolder> items)
{
   m_Items = std::move(items);
   m_CurrentPriorityIndex = 0;
}
void CodeCompletePopupWindow::ClearItems()
{
    if (IsActive())
        DeActivate();
    m_FilteredItems.clear();
    m_Items.clear();
}
template<typename RangeType>
std::vector<wxString> CodeCompletePopupWindow::CreateListStrings(const RangeType& range)
{
#ifdef CLANGCC_TIMING
    wxStopWatch watch;
#endif // CLANGCC_TIMING
    std::vector<wxString> out;
    for (CodeCompleteResultHolder& p : range)
    {
        wxString itemText;
        for (auto chunk : *p.m_Ccs)
        {
            switch (chunk.Kind)
            {
                case CodeCompletionString::CK_Optional:
                case CodeCompletionString::CK_Informative:
                case CodeCompletionString::CK_VerticalSpace :
                case CodeCompletionString::CK_ResultType :
                    break;
                default :
                    itemText << std2wx(chunk.Text);
            }
        }
        itemText << _T(":p") << p.m_Ccr.Priority;
        out.push_back(std::move(itemText));
    }
#ifdef CLANGCC_TIMING
    ClangCCLogger::Get()->Log(wxString::Format(_("CreateListStrings completed in %ldms"), watch.Time()),Logger::info);
#endif // CLANGCC_TIMING
    return out;
}
std::vector<wxString>
CodeCompletePopupWindow::CreateListStrings()
{
    std::vector<wxString> out;
    for (CodeCompleteResultHolder& p : m_FilteredItems)
    {
        wxString itemText;

        for (auto chunk : *p.m_Ccs)
        {
            switch (chunk.Kind)
            {
                case CodeCompletionString::CK_Optional:
                case CodeCompletionString::CK_Informative:
                case CodeCompletionString::CK_VerticalSpace :
                case CodeCompletionString::CK_ResultType :
                    break;
                default :
                    itemText << std2wx(chunk.Text);
            }
        }
        out.push_back(std::move(itemText));
    }
    return out;
}







