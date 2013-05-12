#include "Clang_cc.h"
#include <sdk.h> // Code::Blocks SDK
#include <boost/thread.hpp>
#include <configurationpanel.h>
#include <logmanager.h>
#include <boost/foreach.hpp>

#include "symbolbrowser.h"
#include "symbolbrowserASTvisitor.h"
#include "memoryusage.h"
#include <cbstyledtextctrl.h>
#include <editor_hooks.h>
#include <llvm/Support/MemoryBuffer.h>

#include <clang/Lex/Lexer.h>
#include "codecompletion.h"
#include "Codecompletepopup.h"
#include "optionsdlg.h"
#include "options.h"
#include "diagnosticprinter.h"
#include "ASTnodefinder.h"
#include "contextmenubuilder.h"
#include "tooltipevaluator.h"

#define SCI_GETTEXT 2182


// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<ClangCC> reg(_T("Clang_cc"));
}

int idEditorGotoDeclaration = wxNewId();
int idEditorGotoDefinition = wxNewId();
namespace
{
    int idReparseFile               = wxNewId();
    int idMemoryUsage               = wxNewId();
    int idEditorActivatedTimer      = wxNewId();
    int idReparseTimer              = wxNewId();
}

#define EDITOR_ACTIVATED_DELAY    300

BEGIN_EVENT_TABLE(ClangCC, cbCodeCompletionPlugin)
    EVT_TIMER(idEditorActivatedTimer, ClangCC::OnEditorActivatedTimer)
    EVT_TIMER(idReparseTimer, ClangCC::OnReparseTimer)
    EVT_MENU(idReparseFile, ClangCC::OnReparseFile)
    EVT_MENU(idMemoryUsage, ClangCC::OnMemoryUsage)
    EVT_COMMAND(idParseStart, wxEVT_COMMAND_ENTER, ClangCC::OnParseStart)
    EVT_COMMAND(idParseEnd, wxEVT_COMMAND_ENTER, ClangCC::OnParseEnd)
    EVT_COMMAND(idLogMessage, wxEVT_COMMAND_ENTER, ClangCC::OnLogMessage)

    EVT_MENU (idEditorGotoDeclaration,  ClangCC::OnGotoItemDeclaration)
    EVT_MENU (idEditorGotoDefinition,  ClangCC::OnGotoItemDefinition)
END_EVENT_TABLE()



using namespace clang;
// constructor
ClangCC::ClangCC():
    m_Mgr(Manager::Get()),
    m_EditorActivatedTimer(this, idEditorActivatedTimer),
    m_TUManager(*this)
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if (!Manager::LoadResource(_T("clang_cc.zip")))
    {
        NotifyMissingFile(_T("clang_cc.zip"));
    }

}

// destructor
ClangCC::~ClangCC()
{
}

void ClangCC::OnAttach()
{
    m_Browser = new SymbolBrowser(m_Mgr->GetAppWindow());
    m_CCPopup = new CodeCompletePopupWindow(m_Mgr->GetAppWindow());
    m_Mgr->GetProjectManager()->GetNotebook()->AddPage(m_Browser, _("Clang_cc"));
    ClangCCLogger::Get()->Init(this);

    LogManager* logMgr = m_Mgr->GetLogManager();
    m_LoggerIndex = logMgr->SetLog(ClangCCLogger::Get());
    logMgr->Slot(m_LoggerIndex).title = _("Clang_CC Log");

    CodeBlocksLogEvent evt(cbEVT_ADD_LOG_WINDOW, m_LoggerIndex, logMgr->Slot(m_LoggerIndex).title, logMgr->Slot(m_LoggerIndex).icon);
    Manager::Get()->ProcessEvent(evt);

    EditorHooks::HookFunctorBase* editorHook = new EditorHooks::HookFunctor<ClangCC>(this, &ClangCC::OnEditorEvent);
    m_EditorHookId = EditorHooks::RegisterHook(editorHook);

    m_Mgr->RegisterEventSink(cbEVT_PROJECT_ACTIVATE,     new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectActivated));
    m_Mgr->RegisterEventSink(cbEVT_PROJECT_CLOSE,        new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectClosed));
    m_Mgr->RegisterEventSink(cbEVT_PROJECT_SAVE,         new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectSaved));
    m_Mgr->RegisterEventSink(cbEVT_PROJECT_FILE_ADDED,   new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectFileAdded));
    m_Mgr->RegisterEventSink(cbEVT_PROJECT_FILE_REMOVED, new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectFileRemoved));
    m_Mgr->RegisterEventSink(cbEVT_PROJECT_FILE_CHANGED, new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectFileChanged));

    m_Mgr->RegisterEventSink(cbEVT_EDITOR_MODIFIED,      new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnEditorSaveOrModified));
    m_Mgr->RegisterEventSink(cbEVT_EDITOR_OPEN,          new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnEditorOpen));
    m_Mgr->RegisterEventSink(cbEVT_EDITOR_ACTIVATED,     new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnEditorActivated));
    m_Mgr->RegisterEventSink(cbEVT_EDITOR_TOOLTIP,       new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnEditorTooltip));

    m_Mgr->RegisterEventSink(cbEVT_COMPLETE_CODE,        new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnCodeComplete));
}

void ClangCC::OnRelease(bool appShutDown)
{
    if (!appShutDown)
    {
        EditorHooks::UnregisterHook(m_EditorHookId);
        m_Mgr->RemoveAllEventSinksFor(this);

        int index = m_Mgr->GetProjectManager()->GetNotebook()->GetPageIndex(m_Browser);
        if (index != -1)
            m_Mgr->GetProjectManager()->GetNotebook()->RemovePage(index);
        m_Browser->Destroy();
        m_Browser = NULL;
        m_CCPopup->Destroy();
        m_CCPopup = NULL;

        CodeBlocksLogEvent evt(cbEVT_REMOVE_LOG_WINDOW, m_LoggerIndex);
        Manager::Get()->ProcessEvent(evt);
        m_TUManager.Clear();
    }
}

int ClangCC::Configure()
{
    //create and display the configuration dialog for your plugin
    cbConfigurationDialog dlg(Manager::Get()->GetAppWindow(), wxID_ANY, _("Your dialog title"));
    cbConfigurationPanel* panel = GetConfigurationPanel(&dlg);
    if (panel)
    {
        dlg.AttachConfigurationPanel(panel);
        PlaceWindow(&dlg);
        return dlg.ShowModal() == wxID_OK ? 0 : -1;
    }
    return -1;
}

void ClangCC::BuildMenu(wxMenuBar* menuBar)
{
    int index = menuBar->FindMenu(_("P&lugins"));
    wxMenu* clangCCMenu = new wxMenu();
    clangCCMenu->Append(idReparseFile,_("Reparse File"));
    clangCCMenu->Append(idMemoryUsage,_("Show Memory Usage"));
    menuBar->Insert(index + 1, clangCCMenu,_("&Clang_CC"));
}
void ClangCC::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
    if (type == mtEditorManager)
    {
        cbEditor* editor = m_Mgr->GetEditorManager()->GetBuiltinActiveEditor();
        if (!editor || !IsProviderFor(editor))
            return;
        ProjectFile* projFile = editor->GetProjectFile();
        if (!projFile)
            return;
        ASTUnit* tu = m_TUManager.GetASTUnitForProjectFile(projFile);
        if(tu)
        {
            ASTNodeFinder finder(tu);
            wxString fileName = editor->GetFilename();
            NodeType astNode = finder.GetASTNode(wx2std(fileName),
                                                 editor->GetControl()->GetCurrentPos());
            ContextMenuBuilder builder(menu);
            boost::apply_visitor(builder,astNode);

        }
    }

}
cbConfigurationPanel* ClangCC::GetConfigurationPanel(wxWindow* parent)
{
    return new OptionsPanel(parent);
}
void ClangCC::OnMemoryUsage(wxCommandEvent& event)
{
    cbEditor* editor = m_Mgr->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
        return;
    ProjectFile* projFile = editor->GetProjectFile();
    if (projFile)
    {
        std::vector<ASTMemoryUsage> usages = m_TUManager.GetMemoryUsageForProjectFile(projFile);
        MemoryUsageDlg dlg(Manager::Get()->GetAppWindow(),usages);
        dlg.ShowModal();
    }
}

bool ClangCC::IsProviderFor(cbEditor* ed)
{
    return IsCFamily(ed->GetShortName());
}
void ClangCC::OnCodeComplete(CodeBlocksEvent& event)
{
    //FIXME Currently we ignore the request if we are already
    //activated. That's because C::B send the Code completion request twice
    // for some reason
    if (!m_CCPopup->IsActive())
        CodeComplete();
}
int ClangCC::CodeComplete()
{
    cbEditor* editor = m_Mgr->GetEditorManager()->GetBuiltinActiveEditor();
    //Check whether stuff is alright.
    if (!editor && !IsProviderFor(editor))
        return -1;
    ProjectFile* projFile = editor->GetProjectFile();
    if (!projFile)
        return -1;
    ASTUnit* tu = m_TUManager.GetASTUnitForProjectFile(projFile);
    if(!tu)
        return -1;

    cbStyledTextCtrl* control = editor->GetControl();
    std::string fileName = wx2std(editor->GetFilename());
    int line = control->GetCurrentLine() + 1;
    int pos = control->GetCurrentPos();
    int wordStartPos = control->WordStartPosition(pos,true);
    int column = control->GetColumn(wordStartPos) + 1;
    wxString logstring;
    logstring << _("Code Complete at : ") <<editor->GetShortName() << _(":")<< line <<_(":")<<column;
    ClangCCLogger::Get()->Log(logstring);
    int length = control->GetTextLength();
    llvm::MemoryBuffer* membuf = llvm::MemoryBuffer::getNewUninitMemBuffer(length+1,fileName);
    control->SendMsg(SCI_GETTEXT, length+1, (wxUIntPtr)membuf->getBufferStart());

    ASTUnit::RemappedFile remap = std::make_pair(fileName,membuf);
    CodeCompleteResultHelper helper(tu->getFileSystemOpts());
    CCListedResultTypes shownTypes = Options::Get().GetListedResultTypes();
    CodeCompleteOptions ccOpts;
    ccOpts.IncludeBriefComments = shownTypes.m_IncludeBriefComments;
    ccOpts.IncludeMacros = shownTypes.m_IncludeMacros;
    ccOpts.IncludeCodePatterns = shownTypes.m_IncludePatterns;


    m_CCConsumer.reset(new EditorCodeCompleteConsumer(ccOpts,m_CCPopup));
#ifdef CLANGCC_TIMING
    wxStopWatch watch;
#endif // CLANGCC_TIMING
    //We clear in case CC generates no results.
    m_CCPopup->ClearItems();
    tu->CodeComplete(fileName,line,column,
                     &remap,1, /*Remapped files*/
                     ccOpts.IncludeMacros, /*Include Macros*/
                     ccOpts.IncludeCodePatterns, /*include patterns*/
                     true, /*include brief comments*/
                     *m_CCConsumer,
                     *helper.m_DiagEngine,helper.m_LangOpts,*helper.m_SourceMgr,
                     *helper.m_FileMgr,helper.m_StoredDiags,helper.m_OwnedBuffers
                     );
#ifdef CLANGCC_TIMING
    ClangCCLogger::Get()->Log(wxString::Format(_("tu->CodeComplete executed in %ldms"), watch.Time()),Logger::info);
#endif // CLANGCC_TIMING
    /// Deactivates the current popup
    m_CCPopup->SetEditor(editor);
    m_CCPopup->Activate();
    return 0;
}
void ClangCC::ShowCallTip(){ }


void ClangCC::OnEditorEvent(cbEditor* editor, wxScintillaEvent& sciEvent)
{
    ProjectFile* projFile = editor->GetProjectFile();
    if (!projFile && IsProviderFor(editor))
    {
        sciEvent.Skip();
        return;
    }
    cbStyledTextCtrl* control = editor->GetControl();
    int evtype = sciEvent.GetEventType();
    if(evtype == wxEVT_SCI_CHARADDED)
    {
        ClangCCLogger::Get()->Log(_T("wxEvt_SCI_CHARAdded "));
        wxChar chr = sciEvent.GetKey();
        wxChar prevChr = control->GetCharAt(control->GetCurrentPos() - 2);

        if (chr == _T('.') ||  /// after .
            chr == _T('>') && prevChr == _T('-') || ///after ->
            chr == _T(':') && prevChr == _T(':') || ///after ::
            !m_CCPopup->IsActive() && iswalnum(chr) && iswalpha(prevChr))  /// after two characters and no active completion box
        {
            CodeComplete();
        }
    }
    sciEvent.Skip();


}
void ClangCC::OnEditorActivated(CodeBlocksEvent& event)
{
    m_EditorActivatedTimer.Start(EDITOR_ACTIVATED_DELAY, wxTIMER_ONE_SHOT);
    event.Skip();
}

void ClangCC::OnEditorActivatedTimer(wxTimerEvent& event)
{
    cbEditor* editor = m_Mgr->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
        return;
    ProjectFile* projFile = editor->GetProjectFile();
    if (projFile && IsProviderFor(editor))
    {
        ASTUnit* tu = m_TUManager.GetASTUnitForProjectFile(projFile);
        if (!tu)
        {
            if (IsHeaderFile(projFile->file.GetFullName()))
            {
                ProjectFile* pairedFile = GetProjectFilePair(projFile);
                if (pairedFile) projFile = pairedFile ;
            }
            boost::thread(&TranslationUnitManager::CreateASTUnitForProjectFile,
                          &m_TUManager,projFile,true);
        }
        if (tu)
        {
            m_Browser->SetActiveFile(editor->GetFilename(),tu);
        }
    }
    ClangCCLogger::Get()->Log(_("Editor activated : ") + editor->GetFilename());
}
void ClangCC::OnEditorTooltip(CodeBlocksEvent& event)
{
    cbEditor* editor = m_Mgr->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
        return;
    ProjectFile* projFile = editor->GetProjectFile();
    if (projFile && IsProviderFor(editor))
    {
        cbStyledTextCtrl* control = editor->GetControl();
        int pos = control->PositionFromPointClose(event.GetX(), event.GetY());
        if (pos == wxSCI_INVALID_POSITION)
        {
            event.Skip();
            return;
        }
        ASTUnit* tu = m_TUManager.GetASTUnitForProjectFile(projFile);
        if (!tu)
            return;

        //FIXME Do this better..Maybe our own tooltip
        int isError = control->IndicatorValueAt(GetErrorIndicator(), pos);
        int isWarning = control->IndicatorValueAt(GetWarningIndicator(), pos);
        if (isError || isWarning)
        {
            auto it = tu->stored_diag_begin();
            for ( ; it != tu->stored_diag_end(); ++it)
            {
                StoredDiagnostic diag = *it;
                FullSourceLoc loc = diag.getLocation();
                unsigned offset = loc.getDecomposedLoc().second;
                unsigned range = Lexer::MeasureTokenLength(loc, tu->getSourceManager(), tu->getASTContext().getLangOpts());
                if(offset <= pos && pos <= offset + range)
                {
                    control->CallTipCancel();
                    std::string message = diag.getMessage();
                    control->CallTipShow(pos, std2wx(message));
                    break;
                }
            }
            return;
        }
        ASTNodeFinder finder(tu);
        NodeType node = finder.GetASTNode(wx2std(projFile->file.GetFullPath()), pos);
        ToolTipEvaluator ttEval(tu);
        std::string toolTip = boost::apply_visitor(ttEval, node);
        control->CallTipCancel();
        control->CallTipShow(pos, std2wx(toolTip));



    }
}
void ClangCC::OnParseStart(wxCommandEvent& event)
{
  //TODO do something sensible
  int i=5;
}
void ClangCC::OnParseEnd(wxCommandEvent& event)
{
    cbEditor* editor = m_Mgr->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
        return;
    ProjectFile* editorFile = editor->GetProjectFile();
    if (!editorFile)
        return;
    bool isReparse = event.GetInt();
    ProjectFile* projFile = (ProjectFile*) event.GetClientData();
    ProjectFile* pairedFile = GetProjectFilePair(projFile);
    if (editorFile == projFile || editorFile == pairedFile)
    {
        ASTUnit* tu = m_TUManager.GetASTUnitForProjectFile(projFile);
        if (tu)
             m_Browser->SetActiveFile(editor->GetFilename(), tu, isReparse);
    }
}
void ClangCC::OnReparseTimer(wxTimerEvent& event)
{
    wxCommandEvent forward(wxEVT_COMMAND_MENU_SELECTED, idReparseFile);
    ProcessEvent(forward);
}
void ClangCC::OnReparseFile(wxCommandEvent& event)
{
    cbEditor* editor = m_Mgr->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
        return;
    ProjectFile* projFile = editor->GetProjectFile();
    if (projFile && IsProviderFor(editor))
    {
        boost::thread(&TranslationUnitManager::ReparseProjectFile,
                    &m_TUManager,projFile);
    }
}
void ClangCC::OnLogMessage(wxCommandEvent& event)
{
    wxString msg = event.GetString();
    Logger::level lv = (Logger::level) event.GetInt();
    ClangCCLogger::Get()->Append(msg, lv);
}
void ClangCC::OnGotoItemDeclaration(wxCommandEvent& event)
{
    cbEditor* editor = m_Mgr->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor || !IsProviderFor(editor))
        return;
    ProjectFile* projFile = editor->GetProjectFile();
    if (!projFile)
        return;
    ASTUnit* tu = m_TUManager.GetASTUnitForProjectFile(projFile);
    if(tu)
    {
        ASTNodeFinder finder(tu);
        wxString fileName = editor->GetFilename();
        NodeType astNode = finder.GetASTNode(wx2std(fileName),
                                             editor->GetControl()->GetCurrentPos());

        if(astNode.type() == typeid(Decl*))
           m_Browser->GotoDeclaration(boost::get<Decl*>(astNode));
        if(astNode.type() == typeid(Stmt*))
        {
            if (DeclRefExpr* expr = clang::dyn_cast<DeclRefExpr>(boost::get<Stmt*>(astNode)))
                m_Browser->GotoDeclaration(expr->getDecl());
            if (MemberExpr* expr = clang::dyn_cast<MemberExpr>(boost::get<Stmt*>(astNode)))
                m_Browser->GotoDeclaration(expr->getFoundDecl());
        }
        if(astNode.type() == typeid(RefNode))
        {
            RefNode refNode = boost::get<RefNode>(astNode);
            m_Browser->GotoDeclaration(refNode.GetReferencedDeclaration());
        }
        if(astNode.type() == typeid(TypeLoc))
        {
            TypeLoc tloc = boost::get<TypeLoc>(astNode);
            if(TagTypeLoc tagTloc = tloc.getAs<TagTypeLoc>())
            {
                Decl*  decl= tagTloc.getDecl();
                m_Browser->GotoDeclaration(decl);
            }
            if(ArrayTypeLoc arrayTloc = tloc.getAs<ArrayTypeLoc>())
            {
               // arrayTloc.getElementLoc().getDecl();
            }
        }


    }

}
void ClangCC::OnGotoItemDefinition(wxCommandEvent& event)
{

}
void ClangCC::OnEditorSaveOrModified(CodeBlocksEvent& event)
{

}
void ClangCC::OnEditorOpen(CodeBlocksEvent& event)
{

}
void ClangCC::OnProjectActivated(CodeBlocksEvent& event)
{

}
void ClangCC::OnProjectClosed(CodeBlocksEvent& event)
{
}
void ClangCC::OnProjectSaved(CodeBlocksEvent& event)
{
}
void ClangCC::OnProjectFileAdded(CodeBlocksEvent& event)
{
}
void ClangCC::OnProjectFileRemoved(CodeBlocksEvent& event)
{
}
void ClangCC::OnProjectFileChanged(CodeBlocksEvent& event)
{
}


