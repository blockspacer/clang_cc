#pragma once
#include <wx/wx.h>
#include <wx/popupwin.h>
class ToolTipPopupWindow : public wxPopupTransientWindow
{
public:
    ToolTipPopupWindow(wxWindow *parent);
    void SetText(wxString tooltip);
private:
    wxPanel* m_Panel;
    wxStaticText* m_Tooltip;
    wxBoxSizer* topSizer;
};
