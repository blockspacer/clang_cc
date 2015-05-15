#pragma once



#include "clangcommon.h"

#include <functional>
#include <boost/bind.hpp>
#define BOOST_RANGE_ENABLE_CONCEPT_ASSERT 0
#include <iterator>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>

class CodeCompletePopupWindow;
enum CodeCompleteSortType
{
    Priority
};
using namespace llvm;
// Helper class for passing parameters to ASTUnit.CodeComplete
struct CodeCompleteResultHelper
{
    CodeCompleteResultHelper(const clang::FileSystemOptions& fsOpts):
        m_DiagEngine (new clang::DiagnosticsEngine(IntrusiveRefCntPtr<clang::DiagnosticIDs>(new clang::DiagnosticIDs),
                                                new clang::DiagnosticOptions,
                                                0)),
        m_FileMgr(new clang::FileManager(fsOpts)),
        m_SourceMgr(new clang::SourceManager(*m_DiagEngine,*m_FileMgr))
    {}
    IntrusiveRefCntPtr<clang::DiagnosticsEngine> m_DiagEngine;
    clang::LangOptions m_LangOpts;
    IntrusiveRefCntPtr<clang::FileManager> m_FileMgr;
    IntrusiveRefCntPtr<clang::SourceManager> m_SourceMgr;
    SmallVector<clang::StoredDiagnostic,10> m_StoredDiags;
    SmallVector<const llvm::MemoryBuffer *, 1> m_OwnedBuffers;
};
struct CodeCompleteResultHolder
{
    CodeCompleteResultHolder(clang::CodeCompletionResult result,clang::CodeCompletionString* str,
                             clang::AccessSpecifier access = clang::AS_none):
        m_Ccr(result),
        m_Ccs(str),
        m_Access(access)
    {}
    clang::CodeCompletionResult m_Ccr;
    clang::CodeCompletionString* m_Ccs;
    clang::AccessSpecifier m_Access;
};
class EditorCodeCompleteConsumer : public clang::CodeCompleteConsumer
{
public:
    EditorCodeCompleteConsumer(const CodeCompleteOptions& ccOpts,CodeCompletePopupWindow* ccPopup);
    virtual void ProcessCodeCompleteResults(clang::Sema &S,
                                            clang::CodeCompletionContext Context,
                                            clang::CodeCompletionResult* Results,
                                            unsigned NumResults);
    virtual void ProcessOverloadCandidates(clang::Sema &S, unsigned CurrentArg,
                                           OverloadCandidate *Candidates,
                                           unsigned NumCandidates);
    virtual clang::CodeCompletionAllocator &getAllocator();
    virtual clang::CodeCompletionTUInfo &getCodeCompletionTUInfo();
    virtual ~EditorCodeCompleteConsumer() {}
private:
    clang::CodeCompletionTUInfo m_TUInfo;
    CodeCompleteSortType m_SortType;
    CodeCompletePopupWindow* m_CcPopup;
};


