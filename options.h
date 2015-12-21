
#pragma once
#include <functional>
#include "stringio.h"
#include "codecompletepopup.h"

struct CCListedResultTypes
{
    bool m_IncludeMacros = true;
    bool m_IncludePatterns = true;
    bool m_IncludeKeywords = true;
    bool m_IncludeBriefComments = true;
    bool m_IncludeGlobals = true;
};
///
struct ci_char_traits : public std::char_traits<char>
{
    static bool eq(char p, char q)
    {
        return std::tolower(p) == std::tolower(q);
    }
    static bool lt(char p, char q)
    {
        return std::tolower(p) < std::tolower(q);
    }
    static int compare(const char* p, const char* q, size_t n)
    {
        while(n--)
        {
            if (lt(*p,*q))
                return -1;
            if (lt(*q,*p))
                return 1;
            p++; q++;
        }
        return 0;
    }
};
typedef std::basic_string<char, ci_char_traits> ci_string;
//TODO consider matching from the start
struct StringFilter
{
	StringFilter(const wxString& filter):
	    m_FilterString(wx2std(filter))
	{}
	StringFilter(const std::string& filter):
	    m_FilterString(filter)
    {}
	bool operator() (const CodeCompleteResultHolder& item) const
	{
	    if (m_FilterString.empty())
            return true;
        std::string typedText = item.m_Ccs->getTypedText();
        return typedText.find(m_FilterString) != std::string::npos;
	}
	std::string m_FilterString;
};

struct StringFilterCaseInsensitive
{
	StringFilterCaseInsensitive(const wxString& filter):
	    m_FilterString(wx2std(filter).c_str())
	{}
	StringFilterCaseInsensitive(const std::string& filter):
	    m_FilterString(filter.c_str())
    {}
	bool operator() (const CodeCompleteResultHolder& item) const
	{
	    if (m_FilterString.empty())
            return true;
        ci_string typedText = item.m_Ccs->getTypedText();
        return typedText.find(m_FilterString) != std::string::npos;
	}
	ci_string m_FilterString;
};
//Case Insensitive
struct AcronymFilter
{
    AcronymFilter(wxString filter):
        m_FilterString(filter)
    {}
    bool operator() (const CodeCompleteResultHolder& item) const
    {
        if (m_FilterString.IsEmpty())
            return true;
        auto it = m_FilterString.begin();
        const char* typedText = item.m_Ccs->getTypedText();
        while(*typedText)
	    {
            if ((*it == *typedText++))
            {
                ++it;
                if (it == m_FilterString.end())
                    break;
            }
	    }
        return it == m_FilterString.end();
    }
    wxString m_FilterString;
};
struct AcronymFilterCaseInsensitive
{
    AcronymFilterCaseInsensitive(wxString filter):
        m_FilterString(filter)
    {}
    bool operator() (const CodeCompleteResultHolder& item) const
    {
        if (m_FilterString.IsEmpty())
            return true;
        auto it = m_FilterString.begin();
        const char* typedText = item.m_Ccs->getTypedText();
        while(*typedText)
	    {
            if (wxToupper(*it) == wxToupper(*typedText++))
            {
                ++it;
                if (it == m_FilterString.end())
                    break;
            }
	    }
        return it == m_FilterString.end();
    }
    wxString m_FilterString;
};

class Options :public wxEvtHandler
{
public:
    typedef std::function<bool (const CodeCompleteResultHolder&)> FilterPredicateType;

    enum StringFilterType {ShortHand = 0,Acronym};
    static Options& Get();
    void Populate();
    CCListedResultTypes GetListedResultTypes();
    FilterPredicateType MakeStringFilter(const wxString& filter);
    bool ShouldSkipFunctionBodies() { return m_SkipFunctionBodies; }
    bool ShouldSpellCheck() { return m_SpellCheck; }
    bool ShouldCacheCompletionResults() { return m_CacheCompletionResults;}
    std::vector<std::string> GetCompilerOptions() {return m_ClangOptions;}
    std::string GetMemberCommitCharacters() {return m_CommitCharacters;}

private:
    Options();
    Options(const Options&) = delete;
    Options& operator = (const Options&) = delete;
    ~Options() = default;
private:
    CCListedResultTypes m_ListedResultTypes;
    StringFilterType m_StringFilter;
    bool m_CaseInsensitiveFilter;
    bool m_SkipFunctionBodies;
    bool m_SpellCheck;
    bool m_CacheCompletionResults;
    std::vector<std::string> m_ClangOptions;
    std::string m_CommitCharacters;

};

