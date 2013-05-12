
#include "translationunitmanager.h"
#include <manager.h>
#include <logmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <cbproject.h>
#include "macrosmanager.h"
#include <cbstyledtextctrl.h>

#include "clangcclogger.h"
#include "clang_cc.h"
#include "util.h"
#include "diagnosticprinter.h"
#include "options.h"

#include <llvm/Support/Host.h>
#include <llvm/Support/MemoryBuffer.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/FrontEnd/Utils.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Lex/Preprocessor.h>
#include <boost/foreach.hpp>


int idParseStart    = wxNewId();
int idParseEnd      = wxNewId();

using namespace clang;
TranslationUnitManager::TranslationUnitManager(ClangCC& CC):
    m_CC(CC)
{

}
void TranslationUnitManager::ParseFilesInProject(cbProject* proj)
{

    if (m_ProjectTranslationUnits.end() != m_ProjectTranslationUnits.find(proj))
        return;
    FilesList files = proj->GetFilesList();
    ParserMapType parserMap;
    for (FilesList::const_iterator it = files.begin(); it != files.end(); ++it)
    {

        ProjectFile* file = *it;
        if (!IsCFamily(file->relativeFilename))
            continue;
        wxStatusBar* statusBar = Manager::Get()->GetAppFrame()->GetStatusBar();
        if (statusBar)
        {
            Manager::Get()->GetAppFrame()->SetStatusText(_("Parsing File ")+file->relativeFilename);
        }
        ClangCCLogger::Get()->Log(_("Parsing File ")+file->relativeFilename);
        CreateASTUnitForProjectFile(file);
    }
}
bool TranslationUnitManager::AddASTUnitForProjectFile(ProjectFile* file, ASTUnit* tu)
{
    cbAssert (file && file->file.FileExists() && "File does not exist.");
    cbProject* proj = file->GetParentProject();

    if (proj)
    {
        boost::lock_guard<boost::mutex> lock(m_ProjectsMapMutex);
        m_ProjectTranslationUnits[proj].insert(std::make_pair(file,std::shared_ptr<ASTUnit>(tu)));
        return true;
    }
    return false;
}
ASTUnit* TranslationUnitManager::CreateASTUnitForProjectFile(ProjectFile* file,bool allowAdd)
{
    cbAssert (file && file->file.FileExists() && "File not exists");

    wxCommandEvent startEvent(wxEVT_COMMAND_ENTER,idParseStart);
    startEvent.SetInt(0); //Parse Indicator
    startEvent.SetClientData(file);
    m_CC.AddPendingEvent(startEvent);
#ifdef CLANGCC_TIMING
    wxStopWatch watch;
#endif // CLANGCC_TIMING
    DiagnosticOptions* diagOpts = new DiagnosticOptions();
    IntrusiveRefCntPtr<DiagnosticsEngine> diags = CompilerInstance::createDiagnostics(diagOpts,0);

    std::vector<const char*> args;
    args.push_back("-x");
    if (file->compilerVar == _("CC")) // it is a C file
        args.push_back("c");
    else
        args.push_back("c++");
    args.push_back("-fsyntax-only");
    if (!Options::Get().ShouldSpellCheck())
        args.push_back("-fno-spell-checking");

    //FIXME These are hacks...
    args.push_back("-std=gnu++11");
    args.push_back("-Wno-ignored-attributes");

    std::string fileToParse = wx2std(file->file.GetFullPath());
    args.push_back(fileToParse.c_str());
    // Using command line here so clang could find(hopefully) system headers via Driver.
    CompilerInvocation* invocation = clang::createInvocationFromCommandLine(llvm::makeArrayRef(&args[0],&args[0] + args.size()),
                                                                           diags);

   //Add project wide definitions and include paths.
    MacrosManager* macrosMgr = Manager::Get()->GetMacrosManager();
    const wxArrayString& macros = file->GetParentProject()->GetCompilerOptions();
    for (auto &macro : macros)
    {
        wxString definition;
        //FIXME needs macro replacement? Or we get them already expanded.
        if (macro.StartsWith(_T("-D"), &definition))
        {
            ClangCCLogger::Get()->Log(_("\t Preprocessor Definitions  : ") + definition);
            invocation->getPreprocessorOpts().addMacroDef(wx2std(definition));
        }
    }
    for (auto includePath : file->GetParentProject()->GetIncludeDirs())
    {
        macrosMgr->ReplaceMacros(includePath);
        wxFileName path = wxFileName::DirName(includePath);
        path.Normalize(wxPATH_NORM_ALL,file->GetParentProject()->GetBasePath());
        invocation->getHeaderSearchOpts().AddPath(wx2std(path.GetPath()),
                                                  clang::frontend::Angled,
                                                  false,false);
        ClangCCLogger::Get()->Log(_("\t added Project Include Path : ")+path.GetPath());

    }
    //Add target specific definitions and include paths.
    if (!file->buildTargets.IsEmpty())
    {
        ProjectBuildTarget* target = nullptr;
        wxString targetName = file->GetParentProject()->GetActiveBuildTarget();
        int index = file->buildTargets.Index(targetName);
        index != wxNOT_FOUND ? target = file->GetParentProject()->GetBuildTarget(targetName)
                             : target = file->GetParentProject()->GetBuildTarget(file->buildTargets[0]);
        if (target)
        {
            const wxArrayString& targetMacros = target->GetCompilerOptions();
            for (auto &macro : targetMacros)
            {
                wxString definition;
                //FIXME needs macro replacement?
                if (macro.StartsWith(_T("-D"), &definition))
                {
                    ClangCCLogger::Get()->Log(_("\t Target Preprocessor Definitions  : ") + definition);
                    invocation->getPreprocessorOpts().addMacroDef(wx2std(definition));
                }
            }
            for (auto &includePath : target->GetIncludeDirs())
            {
                macrosMgr->ReplaceMacros(includePath, target);
                wxFileName path = wxFileName::DirName(includePath);
                path.Normalize(wxPATH_NORM_ALL, file->GetParentProject()->GetBasePath());
                invocation->getHeaderSearchOpts().AddPath(wx2std(path.GetPath()),
                                                          clang::frontend::Angled,
                                                          false,false);
                ClangCCLogger::Get()->Log(_("\t Added Target Include Path : ")+path.GetPath());
            }
        }
    }

    PreprocessorOptions &ppOpts = invocation->getPreprocessorOpts();
    ppOpts.RemappedFilesKeepOriginalName = true;
    ppOpts.AllowPCHWithCompilerErrors = true;

    invocation->getFrontendOpts().SkipFunctionBodies = Options::Get().ShouldSkipFunctionBodies();


    ASTUnit* ast = ASTUnit::LoadFromCompilerInvocation(invocation,diags,
                                                       false, /* OnlyLocalDecls */
                                                       true, /*CaptureDiagnostics*/
                                                       true, /*PrecompilePreamble*/
                                                       TU_Complete,
                                                       true,/*CacheCodeCompilationResults*/
                                                       true,/* Include Brief Comment*/
                                                       true  /* User Files are volatile*/
                                                       );
#ifdef CLANGCC_TIMING
    ClangCCLogger::Get()->Log(wxString::Format(_("Parsing %s completed in %ldms"), file->file.GetFullName().c_str(), watch.Time()),Logger::info);
#endif // CLANGCC_TIMING
    if (!ast)
      ClangCCLogger::Get()->Log(_("\t Cannot Create ASTUnit for ") + file->file.GetFullName());
    if (ast)
    {
        if (allowAdd)
            AddASTUnitForProjectFile(file,ast);
        //Show stored diagnostics on the log and the editor
        DiagnosticPrinter printer(ast);
        printer.MarkOnEditors();
    }

    wxCommandEvent endEvent(wxEVT_COMMAND_ENTER,idParseEnd);
    endEvent.SetInt(0); //Parse Indicator
    endEvent.SetClientData(file);
    m_CC.AddPendingEvent(endEvent);
    return ast;
}
ASTUnit* TranslationUnitManager::GetASTUnitForProjectFile(ProjectFile* file)
{
    cbAssert (file && file->file.FileExists() && "File not exists");

    cbProject* proj = file->GetParentProject();
    bool isHeader = IsHeaderFile(file->file.GetFullName());
    boost::lock_guard<boost::mutex> lock(m_ProjectsMapMutex);
    auto it = m_ProjectTranslationUnits.find(proj);
    if (it != m_ProjectTranslationUnits.end())
    {
        auto& fileMap =(*it).second;
        auto it = fileMap.find(file);
        if ( it != fileMap.end())
            return (*it).second.get();
        // if it's a header file check whether a AST exists for sourcefile with the same name.
        if (isHeader)
        {
            ProjectFile* sourceFile = GetProjectFilePair(file);
            if (sourceFile)
            {
                it = fileMap.find(sourceFile);
                if (it != fileMap.end())
                    return (*it).second.get();
            }
        }
    }
    return nullptr;
}
ASTUnit* TranslationUnitManager::ReparseProjectFile(ProjectFile* file)
{
    wxCommandEvent startEvent(wxEVT_COMMAND_ENTER,idParseStart);
    startEvent.SetInt(1); //Reparse Indicator
    startEvent.SetClientData(file);
    m_CC.AddPendingEvent(startEvent);
#ifdef CLANGCC_TIMING
    wxStopWatch watch;
#endif // CLANGCC_TIMING
    wxString fileName = file->file.GetFullPath();
    cbStyledTextCtrl* control = Manager::Get()->GetEditorManager()->GetBuiltinEditor(fileName)->GetControl();
    SmallVector<ASTUnit::RemappedFile,1> remappedFiles;
    if (control->GetModify())
    {
        unsigned length = control->GetLength();
        llvm::MemoryBuffer* membuf = llvm::MemoryBuffer::getNewUninitMemBuffer(length+1,wx2std(fileName));
        control->SendMsg(SCI_GETTEXT, length+1, (wxUIntPtr)membuf->getBufferStart());
        ASTUnit::RemappedFile remap = std::make_pair(wx2std(fileName),membuf);
        remappedFiles.push_back(remap);
    }

    ASTUnit* tu = GetASTUnitForProjectFile(file);
    if (!tu)
    {
        tu = CreateASTUnitForProjectFile(file);
    }
    if (!tu || tu->Reparse(remappedFiles.data(),remappedFiles.size()))
    {
         ClangCCLogger::Get()->Log(_("\t Reparsing Failed : ")+ file->file.GetFullName());
    }
    else //Show diagnostics if any
    {
        DiagnosticPrinter printer(tu);
        printer.MarkOnEditors();
    }
#ifdef CLANGCC_TIMING
    ClangCCLogger::Get()->Log(wxString::Format(_("Reparsing completed in %ldms"), watch.Time()),Logger::info);
#endif // CLANGCC_TIMING
    wxCommandEvent endEvent(wxEVT_COMMAND_ENTER,idParseEnd);
    endEvent.SetInt(1); //Reparse Indicator
    endEvent.SetClientData(file);
    m_CC.AddPendingEvent(endEvent);
    return tu;

}
std::vector<ASTMemoryUsage> TranslationUnitManager::GetMemoryUsageForProjectFile(ProjectFile* file)
{
    ASTUnit* tu = GetASTUnitForProjectFile(file);
    std::vector<ASTMemoryUsage> usages;
    if (tu)
    {
        //AST Context
        ASTContext& ctx = tu->getASTContext();
        usages.push_back(ASTMemoryUsage(AST_Nodes,ctx.getASTAllocatedMemory()));
        usages.push_back(ASTMemoryUsage(AST_Identifiers,ctx.Idents.getAllocator().getTotalMemory()));
        usages.push_back(ASTMemoryUsage(AST_Selectors,ctx.Selectors.getTotalMemory()));
        usages.push_back(ASTMemoryUsage(AST_SideTables,ctx.getSideTableAllocatedMemory()));

        //Source Manager
        usages.push_back(ASTMemoryUsage(SM_ContentCache,ctx.getSourceManager().getContentCacheSize()));
        const SourceManager::MemoryBufferSizes& srcBufs = tu->getSourceManager().getMemoryBufferSizes();
        usages.push_back(ASTMemoryUsage(SM_Malloc,srcBufs.malloc_bytes));
        usages.push_back(ASTMemoryUsage(SM_Mmap,srcBufs.mmap_bytes));
        usages.push_back(ASTMemoryUsage(SM_DataStructures,tu->getSourceManager().getDataStructureSizes()));
        // Preprocessor
        Preprocessor& PP = tu->getPreprocessor();
        usages.push_back(ASTMemoryUsage(PP_Total,PP.getTotalMemory()));
        usages.push_back(ASTMemoryUsage(PP_HeaderSearch,PP.getHeaderSearchInfo().getTotalMemory()));
        if (PP.getPreprocessorOpts().DetailedRecord)
            usages.push_back(ASTMemoryUsage(PP_PreprocessingRecord,PP.getPreprocessingRecord()->getTotalMemory()));
    }
    return usages;
}
void TranslationUnitManager::Clear()
{
    m_ProjectTranslationUnits.clear();
}


