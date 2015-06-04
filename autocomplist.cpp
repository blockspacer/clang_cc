#include "autocomplist.h"
#include "clangcclogger.h"
#include "codecompletepopup.h"
#include "wx/imaglist.h"
#include "wx/xrc/xh_all.h"
#include <wx/settings.h>
#include <algorithm>
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/bind.hpp>

AutoCompList::AutoCompList(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style):
	wxListView(parent,id,pos,size,style)
{
	InsertColumn(0, "TypedText");
    wxImageList* imglist = new wxImageList(16,16,true,0);
    SetBackgroundColour(*wxGREEN);
	imglist->Add(wxXmlResource::Get()->LoadBitmap("browser_images"));
	AssignImageList(imglist, wxIMAGE_LIST_SMALL);
}

wxString AutoCompList::OnGetItemText(long item, long column) const
{
	return m_FilteredItems[item];
}

int AutoCompList::OnGetItemImage(long item) const
{
   CodeCompletePopupWindow* grandPa = static_cast<CodeCompletePopupWindow*>(GetParent());
   return grandPa->GetFilteredImageIndex(item);
}

void AutoCompList::SetItems(std::vector<wxString> items)
{
   m_FilteredItems = items;

   SetItemCount(m_FilteredItems.size());
   LoggerAccess::Get()->Log(wxString::Format("%u items for CodeCompleteList.",m_FilteredItems.size()));
   DoLayout();
   if (!m_FilteredItems.empty())
   {
      Select(0, true);
      EnsureVisible(0);
   }
}

AutoCompList::~AutoCompList()
{
}

void AutoCompList::DoLayout()
{
	using boost::bind;
	using boost::adaptors::transformed;
	using boost::adaptors::sliced;
	std::vector<wxSize> textSizes;

	if (!m_FilteredItems.empty())
	{
        long firstVisible = GetTopItem();
        int last = std::min<unsigned long>(m_FilteredItems.size() - firstVisible, 100);
		wxString widest = *boost::max_element(m_FilteredItems | sliced(firstVisible,firstVisible + last),  //Should be quick enough
                                                              [this] (const wxString& lhs, const wxString& rhs)
                                                              {
                                                                    return GetTextExtent(lhs).GetWidth() < GetTextExtent(rhs).GetWidth();
                                                              });

        int scrollBarWidth = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
        int maxWidth = std::min(GetTextExtent(widest).GetWidth() + 30, MAX_WIDTH); //some padding to count the images in
		SetColumnWidth(0 ,maxWidth);
		wxRect rect;
		GetItemRect(0, rect);
		int height = rect.GetHeight() * m_FilteredItems.size();
		int maxHeight = rect.GetHeight() * ROW_COUNT;
		SetClientSize(maxWidth, std::min(height, maxHeight));
	}
	Refresh();
}
