#ifndef CLANGCCLOGGER_H_
#define CLANGCCLOGGER_H_

#include <logmanager.h>
#include <wx/textctrl.h>
#include <loggers.h>
#include <thread>
#include <mutex>

extern int idLogMessage;

class ClangCCLogger : public TextCtrlLogger
{
public:
    static ClangCCLogger* Get();
    //This one can be called from other threads
    void Init(wxEvtHandler* parent);
    virtual void Log(const wxString& msg, Logger::level lv = info);
private:
    ClangCCLogger();
    virtual ~ClangCCLogger();
    ClangCCLogger(const ClangCCLogger&);
    ClangCCLogger& operator = (const ClangCCLogger&);

private:
    static ClangCCLogger* s_Inst;
    wxEvtHandler* m_LogHandler;
    std::mutex m_LoggerMutex;

};

#endif // CLANGCCLOGGER_H_
