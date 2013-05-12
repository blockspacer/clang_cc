#ifndef TRANSLATIONUNITMANAGER_H_
#define TRANSLATIONUNITMANAGER_H_

#include <map>
#include <wx/string.h>
#include <manager.h>
#include <logmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <boost/thread.hpp>
// interface is macroed somewhere in mingw
// not exactly portable but works for gcc,MSVC and clang
#pragma push_macro("interface")
#undef interface
#include <clang/Frontend/ASTUnit.h>
#pragma pop_macro("interface")


class ClangCC;
extern int idParseStart;
extern int idParseEnd;

typedef std::map<ProjectFile*,std::shared_ptr<clang::ASTUnit> > ParserMapType;

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

class TranslationUnitManager
{
public:
    TranslationUnitManager(ClangCC& CC);
    TranslationUnitManager(const TranslationUnitManager&) = delete;
    TranslationUnitManager& operator = (const TranslationUnitManager&) = delete;
    void ParseFilesInProject(cbProject* proj);
    ASTUnit* GetASTUnitForProjectFile(ProjectFile* file);
    ASTUnit* CreateASTUnitForProjectFile(ProjectFile* file,bool add = true);
    bool     AddASTUnitForProjectFile(ProjectFile* file, ASTUnit* tu);
    ASTUnit* ReparseProjectFile(ProjectFile* file);
    std::vector<ASTMemoryUsage> GetMemoryUsageForProjectFile(ProjectFile* file);
    void Clear();
private:
    std::map<cbProject*,ParserMapType> m_ProjectTranslationUnits;
    ClangCC& m_CC;
    boost::mutex m_ProjectsMapMutex;
};

#endif // TRANSLATIONUNITMANAGER_H_
