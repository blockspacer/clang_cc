#ifndef CODECOMPLETEPOPUP_H_
#define CODECOMPLETEPOPUP_H_


#include <wx/popupwin.h>
#include "cbEditor.h"
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
    void Activate();
    void DeActivate();
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
    static constexpr int m_PriorityArray[3] = {clang::CCP_LocalDeclaration,
                      clang::CCP_MemberDeclaration + clang::CCD_InBaseClass, //Include base class results
                      clang::CCP_NestedNameSpecifier};
	cbEditor* m_Editor = nullptr;
	cbStyledTextCtrl* m_Scintilla = nullptr;
	wxPanel *m_panel;
	wxImageList* m_ToolbarImages;
	AutoCompList *m_CompleteListCtrl;
	wxToolBar* m_FilterToolBar;
	wxPopupWindow* m_FilterPopup;
	DECLARE_EVENT_TABLE()
};
#endif // CODECOMPLETEPOPUP_H_
