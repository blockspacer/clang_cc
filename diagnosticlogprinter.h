#ifndef DIAGNOSTICLOGPRINTER_H
#define DIAGNOSTICLOGPRINTER_H

#include "clang/Basic/Diagnostic.h"

#include <string>

#include "clangcclogger.h"
#include "util.h"


using namespace clang;
class LogDiagnosticConsumer : public DiagnosticConsumer
{
public:
	LogDiagnosticConsumer* clone(DiagnosticsEngine& diags) const;
	void BeginSourceFile(const LangOptions &LO, const Preprocessor *PP);
	void EndSourceFile();
	void HandleDiagnostic(DiagnosticsEngine::Level Level, const Diagnostic &Info);
	void MarkOnEditor(DiagnosticsEngine::Level Level,const Diagnostic& Info);
private:

    wxString m_Filename;
};


#endif // DIAGNOSTICLOGPRINTER_H
