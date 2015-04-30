#ifndef CLANGCCLOGGER_H_
#define CLANGCCLOGGER_H_

#include <logmanager.h>
#include <wx/textctrl.h>
#include <loggers.h>
#include <thread>
#include <mutex>

class ClangCCLogger : public TextCtrlLogger
{
public:

    //This one can be called from other threads
    ClangCCLogger(wxEvtHandler* parent);
    virtual void Log(const wxString& msg, Logger::level lv = info);
    ClangCCLogger(const ClangCCLogger&) = delete;
    ClangCCLogger& operator = (const ClangCCLogger&) = delete;

private:

    wxEvtHandler* m_LogHandler;
    std::mutex m_LoggerMutex;

};
class LoggerAccess
{
public:
   static void Init(ClangCCLogger* logger)
   {
      ptr = logger;
   }
   static ClangCCLogger* Get()
   {
       return ptr;
   }
private:
    static ClangCCLogger * ptr;
};

#endif // CLANGCCLOGGER_H_
