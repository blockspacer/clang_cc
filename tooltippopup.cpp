#include "tooltippopup.h"

ToolTipPopupWindow::ToolTipPopupWindow(wxWindow* parent):
    wxPopupTransientWindow(parent)
{
    m_Panel = new wxPanel (this,wxID_ANY);
    m_Tooltip = new wxStaticText( m_Panel, wxID_ANY,wxT("empty"));
    topSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(m_Tooltip, wxALL);
    m_Panel->SetSizer(topSizer);


}
void ToolTipPopupWindow::SetText(wxString tooltip)
{
    m_Tooltip->SetLabel(tooltip);
    topSizer->Fit(m_Panel);
    SetClientSize(m_Panel->GetSize());

}

