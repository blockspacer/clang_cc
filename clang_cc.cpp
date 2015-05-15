#include "clang_cc.h"
#include <sdk.h> // Code::Blocks SDK
#include <thread>
#include <configurationpanel.h>
#include <logmanager.h>

#include "codelayoutview.h"
#include "codelayoutASTvisitor.h"
#include "memoryusage.h"
#include <cbstyledtextctrl.h>
#include <editor_hooks.h>
#include <cbauibook.h>
#include <compilercommandgenerator.h>
#include <compilerfactory.h>




#include "codecompletion.h"
#include "codecompletepopup.h"
#include "optionsdlg.h"
#include "options.h"
#include "diagnosticprinter.h"
#include "ASTnodefinder.h"
#include "contextmenubuilder.h"
#include "tooltipevaluator.h"
#include "ccevents.h"



#define SCI_GETTEXT 2182


// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<ClangCC> reg(_T("clang_cc"));
}

int idEditorGotoDeclaration = wxNewId();
int idEditorGotoDefinition = wxNewId();
namespace
{
    int idReparseFile               = wxNewId();
    int idMemoryUsage               = wxNewId();
    int idEditorActivatedTimer      = wxNewId();
    int idReparseTimer              = wxNewId();
    int idSaveAST                   = wxNewId();
}

#define EDITOR_ACTIVATED_DELAY    300

BEGIN_EVENT_TABLE(ClangCC, cbCodeCompletionPlugin)
    EVT_TIMER(idEditorActivatedTimer, ClangCC::OnEditorActivatedTimer)
    EVT_TIMER(idReparseTimer, ClangCC::OnReparseTimer)
    EVT_MENU(idReparseFile, ClangCC::OnReparseFile)
    EVT_MENU(idMemoryUsage, ClangCC::OnMemoryUsage)
    EVT_MENU(idSaveAST, ClangCC::OnSaveAST)
    EVT_MENU (idEditorGotoDeclaration,  ClangCC::OnGotoItemDeclaration)
    EVT_MENU (idEditorGotoDefinition,  ClangCC::OnGotoItemDefinition)
END_EVENT_TABLE()



using namespace clang;
// constructor
ClangCC::ClangCC():
    m_Mgr(Manager::Get()),
    m_EditorActivatedTimer(this, idEditorActivatedTimer),
    m_ReparseTimer(this, idReparseTimer),
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
{}

void ClangCC::OnAttach()
{
    m_View = new CodeLayoutView(m_Mgr->GetAppWindow(), m_TUManager);
    m_CCPopup = new CodeCompletePopupWindow(m_Mgr->GetAppWindow());
    m_Mgr->GetProjectManager()->GetUI().GetNotebook()->AddPage(m_View, _("Clang_cc"));

    //Setup logs.

    LoggerAccess::Init(new ClangCCLogger(this));

    LogManager* logMgr = m_Mgr->GetLogManager();
    m_LoggerIndex = logMgr->SetLog(LoggerAccess::Get());
    logMgr->Slot(m_LoggerIndex).title = _("Clang_CC Log");

    CodeBlocksLogEvent evt(cbEVT_ADD_LOG_WINDOW, m_LoggerIndex, logMgr->Slot(m_LoggerIndex).title, logMgr->Slot(m_LoggerIndex).icon);
    Manager::Get()->ProcessEvent(evt);

    //Read options
    Options::Get().Populate();
    //Setup event handlers
    EditorHooks::HookFunctorBase* editorHook = new EditorHooks::HookFunctor<ClangCC>(this, &ClangCC::OnEditorEvent);
    m_EditorHookId = EditorHooks::RegisterHook(editorHook);

    m_Mgr->RegisterEventSink(cbEVT_PROJECT_CLOSE,        new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectClosed));
    m_Mgr->RegisterEventSink(cbEVT_PROJECT_SAVE,         new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectSaved));
    m_Mgr->RegisterEventSink(cbEVT_PROJECT_OPEN,         new cbEventFunctor<TranslationUnitManager, CodeBlocksEvent>(&m_TUManager, &TranslationUnitManager::OnProjectOpened));
    m_Mgr->RegisterEventSink(cbEVT_PROJECT_FILE_ADDED,   new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectFileAdded));
    m_Mgr->RegisterEventSink(cbEVT_PROJECT_FILE_REMOVED, new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectFileRemoved));
    m_Mgr->RegisterEventSink(cbEVT_PROJECT_FILE_CHANGED, new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnProjectFileChanged));

    m_Mgr->RegisterEventSink(cbEVT_EDITOR_MODIFIED,      new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnEditorSaveOrModified));
    m_Mgr->RegisterEventSink(cbEVT_EDITOR_ACTIVATED,     new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnEditorActivated));
    m_Mgr->RegisterEventSink(cbEVT_EDITOR_TOOLTIP,       new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnEditorTooltip));

    m_Mgr->RegisterEventSink(cbEVT_COMPLETE_CODE,        new cbEventFunctor<ClangCC, CodeBlocksEvent>(this, &ClangCC::OnCodeComplete));

    m_TUManager.Bind(ccEVT_PARSE_START, &ClangCC::OnParseStart,this);
    m_TUManager.Bind(ccEVT_REPARSE_START, &ClangCC::OnParseStart,this);
    m_TUManager.Bind(ccEVT_PARSE_END, &ClangCC::OnParseEnd,this);
    m_TUManager.Bind(ccEVT_REPARSE_END, &ClangCC::OnParseEnd,this);
}

void ClangCC::OnRelease(bool appShutDown)
{
        EditorHooks::UnregisterHook(m_EditorHookId);
        m_Mgr->RemoveAllEventSinksFor(this);

        int index = m_Mgr->GetProjectManager()->GetUI().GetNotebook()->GetPageIndex(m_View);
        if (index != -1)
            m_Mgr->GetProjectManager()->GetUI().GetNotebook()->RemovePage(index);
        m_View->Destroy();
        m_View = nullptr;
        m_CCPopup->Destroy();
        m_CCPopup = nullptr;

        CodeBlocksLogEvent evt(cbEVT_REMOVE_LOG_WINDOW, m_LoggerIndex);
        Manager::Get()->ProcessEvent(evt);


        m_TUManager.Unbind(ccEVT_PARSE_START, &ClangCC::OnParseStart,this);
        m_TUManager.Unbind(ccEVT_REPARSE_START, &ClangCC::OnParseStart,this);
        m_TUManager.Unbind(ccEVT_PARSE_END, &ClangCC::OnParseEnd,this);
        m_TUManager.Unbind(ccEVT_REPARSE_END, &ClangCC::OnParseEnd,this);
        m_TUManager.Clear();
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
    clangCCMenu->Append(idSaveAST, _("Save AST File"));
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
            boost::apply_visitor(builder, astNode);

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
        MemoryUsageDlg dlg(Manager::Get()->GetAppWindow(), usages);
        dlg.ShowModal();
    }
}
void ClangCC::OnSaveAST(wxCommandEvent& event)
{
    cbEditor* editor = m_Mgr->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
        return;
    ProjectFile* projFile = editor->GetProjectFile();
    if(!projFile)
        return;
    ASTUnit* tu = m_TUManager.GetASTUnitForProjectFile(projFile);
    if(tu)
    {
        cbProject* project = projFile->GetParentProject();

        wxString projectPath = project->GetBasePath();
        wxFileName path(projectPath);
        path.AppendDir(_T(".clang_ast"));
        path.Mkdir();
        wxString fileName = projFile->file.GetFullName() + _T(".ast");
        std::string refPath = wx2std(path.GetPath(wxPATH_GET_SEPARATOR) + fileName);
        tu->Save(refPath);
    }



}

ClangCC::CCProviderStatus ClangCC::GetProviderStatusFor(cbEditor* ed)
{
    if (IsCFamily(ed->GetShortName()))
        return CCProviderStatus::ccpsActive;
    else
        return CCProviderStatus::ccpsInactive;
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
    LoggerAccess::Get()->Log(logstring);
    int length = control->GetTextLength();
    llvm::MemoryBuffer* membuf = llvm::MemoryBuffer::getNewUninitMemBuffer(length+1,fileName).release();
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
                     ArrayRef<ASTUnit::RemappedFile>(remap), /*Remapped files*/
                     ccOpts.IncludeMacros, /*Include Macros*/
                     ccOpts.IncludeCodePatterns, /*include patterns*/
                     true, /*include brief comments*/
                     *m_CCConsumer,
                     *helper.m_DiagEngine,helper.m_LangOpts,*helper.m_SourceMgr,
                     *helper.m_FileMgr,helper.m_StoredDiags,helper.m_OwnedBuffers
                     );
#ifdef CLANGCC_TIMING
    LoggerAccess::Get()->Log(wxString::Format(_("tu->CodeComplete executed in %ldms"), watch.Time()),Logger::info);
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
        LoggerAccess::Get()->Log(_T("wxEvt_SCI_CHARAdded "));
        wxChar chr = sciEvent.GetKey();

        int currPos = control->GetCurrentPos();

        //Do nothing if string/comment/character
        int style = control->GetStyleAt(currPos);
        if (control->IsString(style)   ||
            control->IsComment(style)  ||
            control->IsCharacter(style))
        {
            sciEvent.Skip();
            return;
        }

        wxChar prevChr = control->GetCharAt(currPos - 2);

        if (chr == '.'                   ||  // after .
            chr == '>' && prevChr == '-' || //after ->
            chr == ':' && prevChr == ':' || //after ::
            !m_CCPopup->IsActive() && iswalnum(chr) && iswalpha(prevChr))  // after two characters and no active completion box
        {
            CodeComplete();
        }

//        if (chr == ' ' || chr == '\t' || chr == '\n')
//            m_HasResult = true;
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
    if (projFile && GetProviderStatusFor(editor) == ccpsActive)
    {
        ASTUnit* tu = m_TUManager.GetASTUnitForProjectFile(projFile);
        if (!tu)
        {
            if (IsHeaderFile(projFile->file.GetFullName()))
            {
                ProjectFile* pairedFile = GetProjectFilePair(projFile);
                if (pairedFile) projFile = pairedFile ;
            }
            std::thread t(&TranslationUnitManager::ParseProjectFile,
                          &m_TUManager,projFile,true);
            t.detach();

        }
        if (tu)
        {
            m_View->SetActiveFile(editor->GetFilename(),tu);
        }
    }
    LoggerAccess::Get()->Log(_("Editor activated : ") + editor->GetFilename());
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
        wxString toolTip;
        if (!m_TUManager.IsFileBeingParsed(projFile))
        {
            ASTNodeFinder finder(tu);
            NodeType node = finder.GetASTNode(wx2std(projFile->file.GetFullPath()), pos);
            ToolTipEvaluator ttEval(tu);
            toolTip = boost::apply_visitor(ttEval, node);
        }
        else
            toolTip = _T("The file is still being parsed.");

        if (!toolTip.empty())
        {
            control->CallTipCancel();
            control->CallTipShow(pos, toolTip);
        }
    }
}
void ClangCC::OnParseStart(ccEvent& event)
{
  //TODO do something sensible
  event.Skip();
}
void ClangCC::OnParseEnd(ccEvent& event)
{
    cbEditor* editor = m_Mgr->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
        return;
    ProjectFile* editorFile = editor->GetProjectFile();
    if (!editorFile)
        return;
    ProjectFile* projFile = event.GetProjectFile();
    ProjectFile* pairedFile = GetProjectFilePair(projFile);
    if (editorFile == projFile || editorFile == pairedFile)
    {
        ASTUnit* tu = event.GetTranslationUnit();
        if (tu)
        {
            //Show diagnostics if any
            DiagnosticPrinter printer(tu);
            printer.MarkOnEditors();
        }

    }
    event.Skip();
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
    if (projFile && GetProviderStatusFor(editor) == CCProviderStatus::ccpsActive)
    {
        std::thread(&TranslationUnitManager::ReparseProjectFile,
                    &m_TUManager,projFile).detach();
    }
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

        const Decl* decl = ASTNode::GetDeclaration(astNode);
        if(decl)
            ASTNode::GotoDeclarationInEditor(decl);
    }

}
void ClangCC::OnGotoItemDefinition(wxCommandEvent& event)
{

}
void ClangCC::OnEditorSaveOrModified(CodeBlocksEvent& event)
{

}

void ClangCC::OnProjectClosed(CodeBlocksEvent& event)
{
    cbProject* project = event.GetProject();
    m_TUManager.RemoveProject(project);
}
void ClangCC::OnProjectSaved(CodeBlocksEvent& event)
{
}

void ClangCC::OnProjectFileAdded(CodeBlocksEvent& event)
{
}
void ClangCC::OnProjectFileRemoved(CodeBlocksEvent& event)
{
    m_TUManager.RemoveFile(event.GetProject(), event.GetString());
    event.Skip();
}
void ClangCC::OnProjectFileChanged(CodeBlocksEvent& event)
{
}


