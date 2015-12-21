
#pragma once

#include <wx/event.h>

namespace clang
{
    class ASTUnit;
}
class ProjectFile;

class ccEvent : public wxEvent
{
public:
    ccEvent(wxEventType eventType = wxEVT_NULL, int id = wxID_ANY):
        wxEvent(id, eventType)
    {}

    ccEvent(wxEventType eventType, wxString fileName, clang::ASTUnit* tu = nullptr, ProjectFile* projFile = nullptr):
        ccEvent(eventType, wxID_ANY, fileName, tu, projFile)
    {}

    ccEvent(wxEventType eventType, int id, wxString fileName, clang::ASTUnit* tu, ProjectFile* projFile):
        wxEvent(id, eventType),
        m_Filename(fileName),
        m_Tu(tu),
        m_ProjectFile(projFile)
    {}

    ccEvent(const ccEvent& event):
        wxEvent(event),
        m_Tu(event.m_Tu),
        m_ProjectFile(event.m_ProjectFile),
        m_Filename(event.m_Filename)
    {}

    wxEvent* Clone() const override
    {
        return new ccEvent(*this);
    }

    void SetTranslationUnit(clang::ASTUnit* tu) { m_Tu = tu; }
    clang::ASTUnit* GetTranslationUnit() const { return m_Tu; }
    void SetProjectFile(ProjectFile* projFile) { m_ProjectFile = projFile; }
    ProjectFile* GetProjectFile() const { return m_ProjectFile; }
    void SetFilename(const wxString& fileName) { m_Filename = fileName; }
    wxString GetFileName() const { return m_Filename; }


private:
    //No Assignment
    ccEvent& operator=(const ccEvent&) = delete;
    clang::ASTUnit* m_Tu = nullptr;
    ProjectFile* m_ProjectFile = nullptr;
    wxString m_Filename;

};

wxDECLARE_EVENT(ccEVT_PARSE_START,ccEvent);
wxDECLARE_EVENT(ccEVT_PARSE_END,ccEvent);
wxDECLARE_EVENT(ccEVT_REPARSE_START,ccEvent);
wxDECLARE_EVENT(ccEVT_REPARSE_END,ccEvent);

