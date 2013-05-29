#include "diagnosticlistview.h"


DiagnosticListView::DiagnosticListView(const wxArrayString& titles,
                                       const wxArrayInt& widths,
                                       bool fixedPitchFont) :
    ListCtrlLogger(titles, widths, fixedPitchFont)
{

}
