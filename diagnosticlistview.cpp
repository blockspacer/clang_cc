#include "diagnosticlistview.h"
#include <wx/listctrl.h>


DiagnosticListView::DiagnosticListView(const wxArrayString& titles,
                                       const wxArrayInt& widths,
                                       bool fixedPitchFont) :
    ListCtrlLogger(titles, widths, fixedPitchFont)
{

}
DiagnosticListView::~DiagnosticListView()
{
    this->control->Destroy();
}
