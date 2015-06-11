#include "tooltippopup.h"

ToolTipPopupWindow::ToolTipPopupWindow()
{
    m_Panel = new wxPanel (this,wxID_ANY);
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(m_Tooltip, wxALL);
    m_Panel->SetSizer(topSizer);


}
void ToolTipPopupWindow::SetText(wxString tooltip)
{
    m_Tooltip->SetLabel(tooltip);


}

