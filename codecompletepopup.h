#pragma once

#include <wx/popupwin.h>
#include <cbeditor.h>
#include <wx/listctrl.h>
#include "codecompletion.h"
#include <clang/Sema/CodeCompleteConsumer.h>
#include <array>


class AutoCompList;
class CodeCompletePopupWindow : public wxPopupWindow
{
public:
	CodeCompletePopupWindow(wxWindow* parent);
	bool AcceptsFocusRecursively();
	bool AcceptsFocus();

	void SetEditor(cbEditor* editor);
	~CodeCompletePopupWindow();
    int GetFilteredImageIndex(long item);
    void SetItems(std::vector<CodeCompleteResultHolder> items);
    void ClearItems();
	void SetFilter(wxString filterString);
    bool IsActive();
    bool Activate();
    void DeActivate();
    ///This is introduced because wxWidgets can't
    ///handle if an event handler is disconnected
    ///during the dispatch and there are other handlers
    ///connected to the event.This utilizes EVT_IDLE or
    ///CallAfter() if the latter is present.
    void DelayedDeactivate();
    void OnIdle(wxIdleEvent& event);
	/* EventHandlers */
	void OnMouse(wxMouseEvent &event);
	void OnItemActivated(wxListEvent& event);
	void OnItemSelected(wxListEvent& event);
	void OnKillFocus(wxFocusEvent &event);
	void OnKeyDown(wxKeyEvent& event);
	void OnCharAdded(wxScintillaEvent& event);
	void OnCompletionListMouseOver(wxMouseEvent &event);
	void OnCompletionListSize(wxSizeEvent& event);
private:
    wxString GetWordAtCursor();
    wxString GetWholeWordAtCursor();
    void CreateToolBar();
    void InsertCompletionString();
private:
    std::vector<wxString> CreateListStrings();
    template<typename RangeType>
    std::vector<wxString> CreateListStrings(const RangeType& range);
    std::vector<CodeCompleteResultHolder> m_Items;
	std::vector<CodeCompleteResultHolder> m_FilteredItems;
	bool m_Active = false;
	int m_CurrentPriorityIndex = 0;
	wxPoint m_ScreenPoint;
    static constexpr int m_PriorityArray[3] = {clang::CCP_MemberDeclaration,
                      clang::CCP_MemberDeclaration + clang::CCD_InBaseClass, //Include base class results
                      clang::CCP_NestedNameSpecifier};
	cbEditor* m_Editor = nullptr;
	cbStyledTextCtrl* m_Scintilla = nullptr;

	wxImageList* m_ToolbarImages;
	AutoCompList *m_CompleteListCtrl;
	wxToolBar* m_FilterToolBar;
	wxPopupWindow* m_FilterPopup;
	DECLARE_EVENT_TABLE()
};

