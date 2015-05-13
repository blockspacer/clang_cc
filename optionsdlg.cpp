#include <wx/xrc/xmlres.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <configmanager.h>
#include "configurationpanel.h"
#include "optionsdlg.h"
#include "options.h"

OptionsPanel::OptionsPanel(wxWindow* parent)
{
    wxXmlResource::Get()->LoadPanel(this, parent, _("options_panel"));
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_("clang_cc"));
    //Code Completion Options
    XRCCTRL(*this, "chk_inc_macros", wxCheckBox)->SetValue(cfg->ReadBool(_T("/cc_inc_macros"), true));
    XRCCTRL(*this, "chk_inc_patterns", wxCheckBox)->SetValue(cfg->ReadBool(_T("/cc_inc_patterns"), true));
    XRCCTRL(*this, "chk_inc_keywords", wxCheckBox)->SetValue(cfg->ReadBool(_T("/cc_inc_keywords"), true));
    XRCCTRL(*this, "chk_inc_comments", wxCheckBox)->SetValue(cfg->ReadBool(_T("/cc_inc_comments"), true));
    //Code Complete Popup Options
    XRCCTRL(*this, "rb_string_filter_type", wxRadioBox)->SetSelection(cfg->ReadInt(_T("/pop_string_filter_type"), 0));
    XRCCTRL(*this, "chk_case_insensitive_filter", wxCheckBox)->SetValue(cfg->ReadBool(_T("/pop_case_insensitive_filter"), true));
    //Translation Unit Options
    XRCCTRL(*this, "chk_skip_function_bodies", wxCheckBox)->SetValue(cfg->ReadBool(_T("/tu_skip_function_bodies"), true));
    XRCCTRL(*this, "chk_spell_check", wxCheckBox)->SetValue(cfg->ReadBool(_T("/tu_spell_check"), true));
    m_ClangOptionsTextCtrl = XRCCTRL(*this, "txt_clang_options", wxTextCtrl);
    wxArrayString clangOptions;
    cfg->Read(_T("/tu_clang_options"), &clangOptions);
    for (const auto& line : clangOptions)
    {
        *m_ClangOptionsTextCtrl << line << "\n";
    }




}
OptionsPanel::~OptionsPanel()
{}
void OptionsPanel::OnApply()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_("clang_cc"));
    //Code Completion Options
    cfg->Write(_T("/cc_inc_macros"), XRCCTRL(*this, "chk_inc_macros", wxCheckBox)->GetValue());
    cfg->Write(_T("/cc_inc_patterns"), XRCCTRL(*this, "chk_inc_patterns", wxCheckBox)->GetValue());
    cfg->Write(_T("/cc_inc_keywords"), XRCCTRL(*this, "chk_inc_keywords", wxCheckBox)->GetValue());
    cfg->Write(_T("/cc_inc_comments"), XRCCTRL(*this, "chk_inc_comments", wxCheckBox)->GetValue());
    //Code Complete Popup Options
    cfg->Write(_T("/pop_string_filter_type"), XRCCTRL(*this, "rb_string_filter_type", wxRadioBox)->GetSelection());
    cfg->Write(_T("/pop_case_insensitive_filter"),  XRCCTRL(*this, "chk_case_insensitive_filter", wxCheckBox)->GetValue());
    //Translation Unit Options
    cfg->Write(_T("/tu_skip_function_bodies"),  XRCCTRL(*this, "chk_skip_function_bodies", wxCheckBox)->GetValue());
    cfg->Write(_T("/tu_spell_check"),  XRCCTRL(*this, "chk_spell_check", wxCheckBox)->GetValue());

    auto lineCount = m_ClangOptionsTextCtrl->GetNumberOfLines();
    wxArrayString clangOptions;
    for (auto i = 0; i < lineCount; ++ i)
    {
        clangOptions.push_back(m_ClangOptionsTextCtrl->GetLineText(i));
    }
    cfg->Write(_T("/tu_clang_options"), clangOptions);
    Options::Get().Populate();
}
void OptionsPanel::OnCancel()
{

}
