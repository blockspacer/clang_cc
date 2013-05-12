#include "diagnosticlogprinter.h"
#include "llvm/ADT/SmallString.h"
#include "clang/Basic/SourceManager.h"
#include <editormanager.h>
#include <cbeditor.h>
#include <cbstyledtextctrl.h>

constexpr int GetErrorIndicator()
{
    return 17;
}
constexpr int GetWarningIndicator()
{
    return 18;
}
LogDiagnosticConsumer* LogDiagnosticConsumer::clone(DiagnosticsEngine& diags) const
{
    return new LogDiagnosticConsumer();
}
void LogDiagnosticConsumer::BeginSourceFile(const LangOptions &LO, const Preprocessor *PP)
{
    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if (editor)
    {
        m_Filename = editor->GetFilename();
        cbStyledTextCtrl* control = editor->GetControl();
        control->SetIndicatorCurrent(GetErrorIndicator());
        control->IndicatorClearRange(0, control->GetTextLength());
        control->SetIndicatorCurrent(GetWarningIndicator());
        control->IndicatorClearRange(0, control->GetTextLength());
    }
}
void LogDiagnosticConsumer::EndSourceFile()
{

}
void LogDiagnosticConsumer::HandleDiagnostic(DiagnosticsEngine::Level Level,
                                             const Diagnostic &Info)
{
    DiagnosticConsumer::HandleDiagnostic(Level, Info);
    Logger::level diagLevel;

    switch (Level)
    {
        case DiagnosticsEngine::Note:
            diagLevel = Logger::info;
            break;
        case DiagnosticsEngine::Warning:
            diagLevel = Logger::warning;
            break;
        case DiagnosticsEngine::Error:
            diagLevel = Logger::error;
            break;
        case DiagnosticsEngine::Fatal:
            diagLevel = Logger::critical;
            break;
        default:
            diagLevel = Logger::info;

    }
    llvm::SmallString<100> outStr;
    std::string location = Info.getLocation().printToString(Info.getSourceManager());
    SourceLocation loc = Info.getLocation();
    SourceManager& srcMgr = Info.getSourceManager();
    if (loc.isValid())
    {
        StringRef fileName;
        unsigned line, column;
        if (loc.isFileID())
        {
            fileName = srcMgr.getFilename(loc);
            line = srcMgr.getSpellingLineNumber(loc);
            column = srcMgr.getSpellingColumnNumber(loc);
        }
        else if (loc.isMacroID())
        {
           fileName = srcMgr.getFilename(loc);
           line= srcMgr.getExpansionLineNumber(loc);
           column = srcMgr.getExpansionColumnNumber(loc);
        }
        // It's okay we can mark it now..
        if (m_Filename == std2wx(fileName))
        {
            MarkOnEditor(Level, Info);
        }
    }

    Info.FormatDiagnostic(outStr);
    ClangCCLogger::Get()->Log(std2wx(location),diagLevel);
    ClangCCLogger::Get()->Log(std2wx(outStr.str()),diagLevel);

}

void LogDiagnosticConsumer::MarkOnEditor(DiagnosticsEngine::Level Level,
                                         const Diagnostic& Info)
{
    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    SourceLocation loc = Info.getLocation();
    if (!editor || !loc.isValid())
        return;
    cbStyledTextCtrl* control = editor->GetControl();
    SourceManager& srcMgr = Info.getSourceManager();
    unsigned pos = srcMgr.getFileOffset(loc);
    int start = control->WordStartPosition(pos, true);
    int end = control->WordEndPosition(pos, true);
    int selRange = end - start;
    if (selRange == 0)
        selRange++;
    switch(Level)
    {
        case DiagnosticsEngine::Error :
            control->IndicatorSetStyle(GetErrorIndicator(), wxSCI_INDIC_SQUIGGLE);
            control->IndicatorSetForeground(GetErrorIndicator(), wxColour(255,0,0));
            control->SetIndicatorCurrent(GetErrorIndicator());
            control->IndicatorFillRange(start,selRange);
            break;
        case DiagnosticsEngine::Warning :
            control->IndicatorSetStyle(GetWarningIndicator(), wxSCI_INDIC_SQUIGGLE);
            control->IndicatorSetForeground(GetWarningIndicator(), wxColour(0,0,255));
            control->SetIndicatorCurrent(GetWarningIndicator());
            control->IndicatorFillRange(start,selRange);
            break;
    }

}
