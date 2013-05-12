#ifndef MEMORYUSAGE_H_
#define MEMORYUSAGE_H_

#include "wx/dialog.h"
#include "translationunitmanager.h"

class MemoryUsageDlg : public wxDialog
{
public:
    MemoryUsageDlg(wxWindow* parent,std::vector<ASTMemoryUsage>);
    virtual ~MemoryUsageDlg(){};
};
#endif // MEMORYUSAGE_H_
