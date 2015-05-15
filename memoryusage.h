#pragma once

#include "wx/dialog.h"
#include "translationunitmanager.h"

class MemoryUsageDlg : public wxDialog
{
public:
    MemoryUsageDlg(wxWindow* parent,std::vector<ASTMemoryUsage>);
    virtual ~MemoryUsageDlg(){};
};

