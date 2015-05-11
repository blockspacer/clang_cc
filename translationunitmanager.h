#ifndef TRANSLATIONUNITMANAGER_H_
#define TRANSLATIONUNITMANAGER_H_

#include <map>
#include <vector>
#include <wx/string.h>
#include <thread>
#include <mutex>

#include "clangcommon.h"

#include <manager.h>
#include <logmanager.h>
#include <editormanager.h>
#include <cbeditor.h>


class ClangCC;



typedef std::map<wxString,std::shared_ptr<clang::ASTUnit> > ParserMapType;

enum ASTUnitMemoryUsageKind
{
    AST_Nodes,
    AST_Identifiers,
    AST_Selectors,
    AST_SideTables,
    SM_ContentCache,
    SM_Malloc,
    SM_Mmap,
    SM_DataStructures,
    PP_Total,
    PP_PreprocessingRecord,
    PP_HeaderSearch
};
struct ASTMemoryUsage
{
    ASTMemoryUsage(ASTUnitMemoryUsageKind kind,std::size_t amount):
         m_Kind(kind),
         m_Amount(amount)
    {}
    ASTUnitMemoryUsageKind m_Kind;
    unsigned long m_Amount;
};

using clang::ASTUnit;

class TranslationUnitManager:public wxEvtHandler
{
public:
    TranslationUnitManager(ClangCC& CC);
    TranslationUnitManager(const TranslationUnitManager&) = delete;
    TranslationUnitManager& operator = (const TranslationUnitManager&) = delete;
    ASTUnit* GetASTUnitForProjectFile(ProjectFile* file);
    ASTUnit* ParseProjectFile(ProjectFile* file,bool add = true);
    bool     AddASTUnitForProjectFile(ProjectFile* file, ASTUnit* tu);
    ASTUnit* ReparseProjectFile(ProjectFile* file);
    bool IsFileBeingParsed(ProjectFile* file);
    bool CreateCompilationDatabase(cbProject* proj);
    std::vector<ASTMemoryUsage> GetMemoryUsageForProjectFile(ProjectFile* file);
    void RemoveProject(cbProject* project);
    void RemoveFile(cbProject* project,const wxString& fileName);
    void OnProjectOpened(CodeBlocksEvent& event);
    void Clear();
private:
    std::map<cbProject*,ParserMapType> m_ProjectTranslationUnits;
    std::vector<ProjectFile*> m_FilesBeingParsed;
    ClangCC& m_CC;
    std::mutex m_ProjectsMapMutex;
    std::mutex m_FilesBeingParsedMutex;
    std::map<cbProject*,std::unique_ptr<clang::tooling::CompilationDatabase>> m_CompilationDatabases;
};

#endif // TRANSLATIONUNITMANAGER_H_
