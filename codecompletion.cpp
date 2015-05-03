#include "codecompletion.h"
#include <clang/AST/Decl.h>
#include "codecompletepopup.h"
#include "options.h"
#include "clangcclogger.h"


using boost::bind;
using namespace clang;
EditorCodeCompleteConsumer::EditorCodeCompleteConsumer(const CodeCompleteOptions& ccOpts,CodeCompletePopupWindow* ccPopup):
    CodeCompleteConsumer(ccOpts,false),
    m_TUInfo(new GlobalCodeCompletionAllocator),
    m_SortType(Priority),
    m_CcPopup(ccPopup)
{}

void EditorCodeCompleteConsumer::ProcessCodeCompleteResults(Sema &S,
                                                            CodeCompletionContext Context,
                                                            CodeCompletionResult *Results,
                                                            unsigned NumResults)
{
#ifdef CLANGCC_TIMING
    wxStopWatch watch;
#endif // CLANGCC_TIMING
//    std::stable_sort(Results, Results + NumResults); //Sort By name
//    LoggerAccess::Get()->Log(wxString::Format(_("ProcessCodeCompleteResults Sort by name executed in %ldms"), watch.Time()),Logger::info);
    std::vector<CodeCompleteResultHolder> out;
    switch(m_SortType)
    {
        case Priority:
            std::stable_sort(Results, Results + NumResults ,[] (const CodeCompletionResult& lhs, const CodeCompletionResult& rhs)
                                                                { return lhs.Priority < rhs.Priority;});
            break;
    }
#ifdef CLANGCC_TIMING
    LoggerAccess::Get()->Log(wxString::Format(_("ProcessCodeCompleteResults Sort by Priority executed in %ldms"), watch.Time()),Logger::info);
#endif // CLANGCC_TIMING
    std::vector<wxString> stringResults;
    CCListedResultTypes shownTypes = Options::Get().GetListedResultTypes();
    for (unsigned i=0; i < NumResults; ++i)
    {
        AccessSpecifier access = AS_none;
        if (Results[i].Kind == CodeCompletionResult::RK_Declaration)
        {
            //Skip the overloaded operators,destructors and conversion ops
            if (const FunctionDecl* funcDecl = dyn_cast<FunctionDecl>(Results[i].getDeclaration()))
            {
                if (funcDecl->isOverloadedOperator())
                     continue;
                switch (funcDecl->getDeclKind())
                {
                    case Decl::CXXDestructor:
                    case Decl::CXXConversion:
                        continue;
                    default:
                        break;
                }
            }
            access = Results[i].getDeclaration()->getAccess();
        }

        CodeCompletionString *ccs
            = Results[i].CreateCodeCompletionString(S, getAllocator(), m_TUInfo, includeBriefComments());
        //FIXME If cached results are used this fails miserably
//        switch (Results[i].Kind)
//        {
//            case CodeCompletionResult::RK_Macro :   if (!shownTypes.m_IncludeMacros)
//                                                        continue;
//                                                    break;
//            case CodeCompletionResult::RK_Pattern : if (!shownTypes.m_IncludePatterns)
//                                                      continue;
//                                                    break;
//            case CodeCompletionResult::RK_Keyword : if (!shownTypes.m_IncludeKeywords)
//                                                      continue;
//                                                    break;
//        }
        out.emplace_back(Results[i], ccs, access);
    }
    m_CcPopup->SetItems(std::move(out));
#ifdef CLANGCC_TIMING
    LoggerAccess::Get()->Log(wxString::Format(_("ProcessCodeCompleteResults processed %d results in %ldms"), NumResults, watch.Time()),Logger::info);
#endif // CLANGCC_TIMING
}

void EditorCodeCompleteConsumer::ProcessOverloadCandidates(Sema &S, unsigned CurrentArg,
                                                           OverloadCandidate *Candidates,
                                                           unsigned NumCandidates)
{
    LoggerAccess::Get()->Log(_("Strange!! ProcessOverloadCandidates never gets called.See to it."));
    std::ostringstream os;
    for (unsigned i=0 ; i < NumCandidates ; ++i)
    {
        CodeCompletionString *ccs
            = Candidates[i].CreateSignatureString(CurrentArg, S, getAllocator(), m_TUInfo,true);
        os << ccs->getAsString() << "\n";
    }

}
CodeCompletionAllocator& EditorCodeCompleteConsumer::getAllocator()
{
    return m_TUInfo.getAllocator();
}

CodeCompletionTUInfo& EditorCodeCompleteConsumer::getCodeCompletionTUInfo()
{
    return m_TUInfo;
}
