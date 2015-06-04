#include "options.h"
#include "configmanager.h"

Options& Options::Get()
{
    static Options singleton;
    return singleton;
}
Options::Options()
{
    Populate();
}
void Options::Populate()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager("clang_cc");
    //Code Completion Options
    m_CacheCompletionResults = cfg->ReadBool("/cc_cache_cc_results", true);
    m_ListedResultTypes.m_IncludeMacros = cfg->ReadBool("/cc_inc_macros", true);
    m_ListedResultTypes.m_IncludePatterns = cfg->ReadBool("/cc_inc_patterns", true);
    m_ListedResultTypes.m_IncludeKeywords = cfg->ReadBool("/cc_inc_keywords", true);
    m_ListedResultTypes.m_IncludeBriefComments = cfg->ReadBool("/cc_inc_comments", true);
    //Code Complete Popup Options
    m_StringFilter = (StringFilterType) cfg->ReadInt("/pop_string_filter_type", 0);
    m_CaseInsensitiveFilter = cfg->ReadBool("/pop_case_insensitive_filter", 0);
    //Translation Unit Options
    m_SkipFunctionBodies = cfg->ReadBool("/tu_skip_function_bodies");
    m_SpellCheck = cfg->ReadBool("/tu_spell_check");
    wxArrayString clangOptions;
    cfg->Read("/tu_clang_options", &clangOptions);
    m_ClangOptions.clear();
    for (const auto& option : clangOptions)
    {
        m_ClangOptions.push_back(wx2std(option));
    }
    m_CommitCharacters = R"({}[]().,:;+-*/%&|^!=<>?@#\)";


}
CCListedResultTypes Options::GetListedResultTypes()
{
    return m_ListedResultTypes;
}
Options::FilterPredicateType Options::MakeStringFilter(const wxString& filter)
{
    switch (m_StringFilter)
    {
        case ShortHand:
            if (m_CaseInsensitiveFilter)
                return StringFilterCaseInsensitive(filter);
            return StringFilter(filter);
        case Acronym:
            if (m_CaseInsensitiveFilter)
                return AcronymFilterCaseInsensitive(filter);
            return AcronymFilter(filter);
    }
}

