
#ifndef OPTIONS_H_
#define OPTIONS_H_
#include <functional>
#include "util.h"
#include "codecompletepopup.h"
#include <boost/foreach.hpp>
struct CCListedResultTypes
{
    CCListedResultTypes():
        m_IncludeMacros(true),
        m_IncludePatterns(true),
        m_IncludeKeywords(true),
        m_IncludeBriefComments(true)
    {}
    bool m_IncludeMacros;
    bool m_IncludePatterns;
    bool m_IncludeKeywords;
    bool m_IncludeBriefComments;
};
struct StringFilter
{
	StringFilter(const wxString& filter):
	    m_FilterString(filter)
	{}
	bool operator() (const CodeCompleteResultHolder& item) const
	{
	    wxString toMatch = std2wx(item.m_Ccs->getTypedText());
		return m_FilterString.IsEmpty() || toMatch.Contains(m_FilterString);
	}
	wxString m_FilterString;
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
        wxString::const_iterator it = m_FilterString.begin();
	    BOOST_FOREACH(char i, item.m_Ccs->getTypedText())
	    {
            if (wxToupper(*it) == wxToupper(i))
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

private:
    Options();
    Options(const Options&) = delete;
    Options& operator = (const Options&) = delete;
    ~Options() = default;
private:
    CCListedResultTypes m_ListedResultTypes;
    StringFilterType m_StringFilter;
    bool m_SkipFunctionBodies;
    bool m_SpellCheck;

};
#endif // OPTIONS_H_
