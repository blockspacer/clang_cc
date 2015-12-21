#pragma once

#include <wx/listctrl.h>
#include <vector>
constexpr int MAX_HEIGHT = 250;
constexpr int MAX_WIDTH  = 400;
constexpr int ROW_COUNT = 15;

class AutoCompList :public wxListView
{
public:
	AutoCompList(wxWindow* parent, wxWindowID id = wxID_ANY,
		         const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
				 long style = wxLC_VIRTUAL|wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_NO_HEADER);
	wxString OnGetItemText(long item, long column) const;
	int OnGetItemImage(long item) const;
	void SetItems(std::vector<wxString> items);
	~AutoCompList();
	void DoLayout();
private:
	std::vector<wxString> m_FilteredItems;
};


