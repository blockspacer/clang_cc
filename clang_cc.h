/***************************************************************
 * Name:      clang_cc
 * Purpose:   Code::Blocks plugin
 * Author:    hurcan solter ()
 * Created:   2012-12-05
 * Copyright: hurcan solter
 * License:   GPL
 **************************************************************/

#ifndef CLANG_CC_H_
#define CLANG_CC_H_

// For compilers that support precompilation, includes <wx/wx.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <cbplugin.h> // for "class cbToolPlugin"
#include "symbolbrowser.h"
#include "translationunitmanager.h"
#include <map>

class CodeCompletePopupWindow;
class EditorCodeCompleteConsumer;

extern int idEditorGotoDeclaration;
extern int idEditorGotoDefinition;

class ClangCC : public cbCodeCompletionPlugin
{
public:
    /** Constructor. */
    ClangCC();
    /** Destructor. */
    virtual ~ClangCC();

    /** Invoke configuration dialog. */
    virtual int Configure();
    virtual int GetConfigurationPriority() const { return 50; }
    virtual int GetConfigurationGroup() const { return cgUnknown; }
    virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent);
    virtual cbConfigurationPanel* GetProjectConfigurationPanel(wxWindow* parent, cbProject* project){ return 0; }
    // Code Completion Methods..
    virtual wxArrayString GetCallTips() { return wxArrayString(); }
    virtual int CodeComplete();
    virtual void ShowCallTip();
    virtual bool IsProviderFor(cbEditor* ed);
    /** build menus in the main frame */
    virtual void BuildMenu(wxMenuBar* menuBar);
    /** build context menu */
    virtual void BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data = 0);
    // Editor event handlers
    /** SDK project related events */
    void OnProjectActivated(CodeBlocksEvent& event);
    void OnProjectClosed(CodeBlocksEvent& event);
    void OnProjectSaved(CodeBlocksEvent& event);
    void OnProjectFileAdded(CodeBlocksEvent& event);
    void OnProjectFileRemoved(CodeBlocksEvent& event);
    void OnProjectFileChanged(CodeBlocksEvent& event);
    /** SDK editor related events */
    void OnEditorEvent(cbEditor* editor, wxScintillaEvent& sciEvent);
    void OnEditorSaveOrModified(CodeBlocksEvent& event);
    void OnEditorOpen(CodeBlocksEvent& event);
    void OnEditorActivated(CodeBlocksEvent& event);
    void OnEditorClosed(CodeBlocksEvent& event);
    void OnEditorTooltip(CodeBlocksEvent& event);
    /** Timer related events */
    void OnEditorActivatedTimer(wxTimerEvent& event);
    void OnReparseTimer(wxTimerEvent& event);
    /** Code completion events*/
    void OnReparseFile(wxCommandEvent& event);
    void OnCodeComplete(CodeBlocksEvent& event);
    void OnMemoryUsage(wxCommandEvent& event);
    // Called when a parsing is about to begin.
    void OnParseStart(wxCommandEvent& event);
    // Called after parsing ends.
    void OnParseEnd(wxCommandEvent& event);
    // Handles log messages
    void OnLogMessage(wxCommandEvent& event);
    // ContextMenu Handlers
    void OnGotoItemDeclaration(wxCommandEvent& event);
    void OnGotoItemDefinition(wxCommandEvent& event);
    SymbolBrowser* GetSymbolBrowser() {return m_Browser;}
protected:
    virtual void OnAttach();
    virtual void OnRelease(bool appShutDown);
private:
     SymbolBrowser* m_Browser;
     CodeCompletePopupWindow* m_CCPopup;
     std::shared_ptr<EditorCodeCompleteConsumer> m_CCConsumer;
     //Convenience accessor
     Manager* const m_Mgr ;
     TranslationUnitManager m_TUManager;
     int m_LoggerIndex; // Index of Logger in LogManager
     int m_EditorHookId;
     /** delay after receive editor activated event*/
     wxTimer                 m_EditorActivatedTimer;
     DECLARE_EVENT_TABLE()
};

#endif // CLANG_CC_H_
