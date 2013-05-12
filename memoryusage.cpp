#include "memoryusage.h"
#include <numeric>
#include <wx/xrc/xmlres.h>
#include <wx/stattext.h>
#include <boost/foreach.hpp>

MemoryUsageDlg::MemoryUsageDlg(wxWindow* parent,std::vector<ASTMemoryUsage> usage)
{
    wxXmlResource::Get()->LoadObject(this, parent, _T("resource_usage_dlg"),_T("wxDialog"));
    unsigned long totalMem = 0;
    BOOST_FOREACH(ASTMemoryUsage& fold,usage)
    {
        switch (fold.m_Kind)
        {
            case AST_Nodes:
                XRCCTRL(*this, "txt_ast_nodes", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
            case AST_Identifiers:
                XRCCTRL(*this, "txt_ast_identifiers", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
            case AST_Selectors:
                XRCCTRL(*this, "txt_ast_selectors", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
            case AST_SideTables:
                XRCCTRL(*this, "txt_ast_tables", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
            case SM_ContentCache:
                XRCCTRL(*this, "txt_sm_content_cache", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
            case SM_Malloc:
                XRCCTRL(*this, "txt_sm_malloc", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
            case SM_Mmap:
                XRCCTRL(*this, "txt_sm_mmap", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
            case SM_DataStructures:
                XRCCTRL(*this, "txt_sm_data_structures", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
            case PP_Total:
                XRCCTRL(*this, "txt_pp_total", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
            case PP_PreprocessingRecord:
                XRCCTRL(*this, "txt_pp_preprocessing_record", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
            case PP_HeaderSearch:
                XRCCTRL(*this, "txt_pp_header_search", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),fold.m_Amount));
                break;
        }
        totalMem += fold.m_Amount;

    }
    XRCCTRL(*this, "txt_total_memory", wxStaticText)->SetLabel(wxString::Format(_("%ld bytes"),totalMem));


}

