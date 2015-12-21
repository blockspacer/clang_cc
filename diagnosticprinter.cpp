#include "diagnosticprinter.h"

#include "clangcommon.h"
#include <editormanager.h>
#include <cbeditor.h>
#include <cbstyledtextctrl.h>
#include <logmanager.h>
#include "stringio.h"
#include "clangcclogger.h"

DiagnosticPrinter::DiagnosticPrinter(ASTUnit* tu):
    m_TU(tu)
{
    CleanIndicators();
}

void DiagnosticPrinter::CleanIndicators()
{
    for (int i = 0; i < Manager::Get()->GetEditorManager()->GetEditorsCount(); ++i)
    {

        cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinEditor(i);
        if(editor)
        {
            cbStyledTextCtrl* control = editor->GetControl();
            control->SetIndicatorCurrent(GetErrorIndicator());
            control->IndicatorClearRange(0, control->GetTextLength());
            control->SetIndicatorCurrent(GetWarningIndicator());
            control->IndicatorClearRange(0, control->GetTextLength());
        }
    }
}
void DiagnosticPrinter::MarkOnEditors()
{
    auto it = m_TU->stored_diag_begin();
    for (; it != m_TU->stored_diag_end(); ++it)
    {
        StoredDiagnostic diag = *it;
        FullSourceLoc loc = diag.getLocation();

        DiagnosticsEngine::Level level= diag.getLevel();
        Logger::level diagLevel;
        switch (level)
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
        LoggerAccess::Get()->Log(std2wx(diag.getMessage()), diagLevel);
        if (loc.isValid())
        {
            const SourceManager& srcMgr = loc.getManager();
            LoggerAccess::Get()->Log(std2wx(loc.printToString(srcMgr)), diagLevel);
            std::string fileName = srcMgr.getFilename(loc);

            cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinEditor(std2wx(fileName));
            if(!editor)
                return;
            cbStyledTextCtrl* control = editor->GetControl();
            unsigned offset = srcMgr.getFileOffset(loc);
            unsigned selRange = Lexer::MeasureTokenLength(loc, srcMgr, m_TU->getASTContext().getLangOpts());

            switch(level)
            {
                case DiagnosticsEngine::Error :
                    control->IndicatorSetStyle(GetErrorIndicator(), wxSCI_INDIC_SQUIGGLE);
                    control->IndicatorSetForeground(GetErrorIndicator(), wxColour(255,0,0));
                    control->SetIndicatorCurrent(GetErrorIndicator());
                    control->IndicatorFillRange(offset, selRange);
                    break;
                case DiagnosticsEngine::Warning :
                    control->IndicatorSetStyle(GetWarningIndicator(), wxSCI_INDIC_SQUIGGLE);
                    control->IndicatorSetForeground(GetWarningIndicator(), wxColour(0,0,255));
                    control->SetIndicatorCurrent(GetWarningIndicator());
                    control->IndicatorFillRange(offset, selRange);
                    break;
            }
        }
    }
}

