#pragma once
#include <wx/popupwin.h>

class TooltipPopupWindow : public wxPopupWindow
{
public:
    void SetText(wxString text)
private:
    wxStaticText* m_TooltipText;
    wxPanel* m_Panel;
}

