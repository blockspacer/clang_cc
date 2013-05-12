#ifndef OPTIONSDLG_H_
#define OPTIONSDLG_H_
#include "configurationpanel.h"

class OptionsPanel: public cbConfigurationPanel
{
public:
    OptionsPanel(wxWindow* parent);
    ~OptionsPanel();
    virtual wxString GetTitle() const          { return _("Clang CC"); }
    virtual wxString GetBitmapBaseName() const { return _("clang_cc"); }
    virtual void OnApply();
    virtual void OnCancel();
};

#endif //OPTIONSDLG_H_
