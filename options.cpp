#include "options.h"
#include "configmanager.h"

Options& Options::Get()
{
    //This is safe in C++11. I've begun to like it.
    static Options singleton;
    return singleton;
}
Options::Options()
{
    Populate();
}
void Options::Populate()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_("clang_cc"));
    //Code Completion Options
    m_ListedResultTypes.m_IncludeMacros = cfg->ReadBool(_T("/cc_inc_macros"), false);
    m_ListedResultTypes.m_IncludePatterns = cfg->ReadBool(_T("/cc_inc_patterns"), false);
    m_ListedResultTypes.m_IncludeKeywords = cfg->ReadBool(_T("/cc_inc_keywords"), false);
    m_ListedResultTypes.m_IncludeBriefComments = cfg->ReadBool(_T("/cc_inc_comments"), false);
    //Code Complete Popup Options
    m_StringFilter = (StringFilterType) cfg->ReadInt(_T("//string_filter_type"),0);
    //Translation Unit Options
    m_SkipFunctionBodies = cfg->ReadBool(_T("/tu_skip_function_bodies"));
    m_SpellCheck = cfg->ReadBool(_T("/tu_spell_check"));

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
            return StringFilter(filter);
        case Acronym:
            return AcronymFilter(filter);
    }
}

