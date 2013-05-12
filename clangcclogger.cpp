#include "clangcclogger.h"

//FIXME why the hell Logmanager deletes this?
ClangCCLogger* ClangCCLogger::s_Inst =0;

int idLogMessage = wxNewId();

ClangCCLogger* ClangCCLogger::Get()
{
    if (!s_Inst)
        s_Inst = new ClangCCLogger;
    return s_Inst;
}

ClangCCLogger::ClangCCLogger()
{
}
ClangCCLogger::~ClangCCLogger()
{

}
void ClangCCLogger::Init(wxEvtHandler* parent)
{
    m_LogHandler = parent;
}
void ClangCCLogger::Log(const wxString& msg, Logger::level lv)
{
    wxCommandEvent logEvent(wxEVT_COMMAND_ENTER, idLogMessage);
    logEvent.SetString(msg);
    logEvent.SetInt(lv);
    m_LogHandler->AddPendingEvent(logEvent);
}


