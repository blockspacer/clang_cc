#pragma once
#include <wx/wx.h>
#include <wx/popupwin.h>

class ToolTipPopupWindow : public wxPopupWindow
{
public:
    ToolTipPopupWindow();
    void SetText(wxString text);
private:
    wxStaticText* m_TooltipText;
    wxPanel* m_Panel;
};

