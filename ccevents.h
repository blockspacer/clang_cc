
#ifndef CCEVENTS_H_
#define CCEVENTS_H_

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
        m_ProjectFile(event.m_ProjectFile)
    {
        SetFilename(event.m_Filename.c_str());
    }

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
//extern const wxEventType ccEVT_PARSE_START;
//extern const wxEventType ccEVT_PARSE_END;
//extern const wxEventType ccEVT_REPARSE_START;
//extern const wxEventType ccEVT_REPARSE_END;

//typedef void (wxEvtHandler::*ccEventFunction)(ccEvent&);


//#define ccEventHandler(func)  \
//	(wxObjectEventFunction)(wxEventFunction) \
//	wxStaticCastEvent(ccEventFunction, &func)
//
//#define EVT_PARSE(event,func)  wx__DECLARE_EVT0(event, ccEventHandler(func))
//#define EVT_PARSE_END(func)  wx__DECLARE_EVT0(ccEVT_PARSE_END, ccEventHandler(func))
//#define EVT_REPARSE_START(func)  wx__DECLARE_EVT0(ccEVT_REPARSE_START, ccEventHandler(func))
//#define EVT_REPARSE_END(func)  wx__DECLARE_EVT0(ccEVT_REPARSE_END, ccEventHandler(func))
//
#endif // CCEVENTS_H_
