#include "clangcclogger.h"

//FIXME why the hell Logmanager deletes this?


ClangCCLogger* LoggerAccess::ptr = 0;

ClangCCLogger::ClangCCLogger(wxEvtHandler* parent)
{
    m_LogHandler = parent;
}

void ClangCCLogger::Log(const wxString& msg, Logger::level lv)
{
    m_LogHandler->CallAfter(std::bind(&ClangCCLogger::Append,this,msg,lv));
}


