#pragma once
#include <logmanager.h>
#include <wx/textctrl.h>
#include <loggers.h>


class DiagnosticListView : public ListCtrlLogger
{
    DiagnosticListView(const wxArrayString& titles, const wxArrayInt& widths, bool fixedPitchFont = false);

};

