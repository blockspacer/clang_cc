#include "autocomplist.h"
#include "clangcclogger.h"
#include "Codecompletepopup.h"
#include "wx/imaglist.h"
#include "wx/xrc/xh_all.h"
#include <algorithm>
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/bind.hpp>

AutoCompList::AutoCompList(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style):
	wxListView(parent,id,pos,size,style)
{
	InsertColumn(0, _("TypedText"));
    wxImageList* imglist = new wxImageList(16,16,true,0);
	imglist->Add(wxXmlResource::Get()->LoadBitmap(_("browser_images")));
	AssignImageList(imglist, wxIMAGE_LIST_SMALL);
}

wxString AutoCompList::OnGetItemText(long item, long column) const
{
	return m_FilteredItems[item];
}

int AutoCompList::OnGetItemImage(long item) const
{
   CodeCompletePopupWindow* grandPa = static_cast<CodeCompletePopupWindow*>(GetGrandParent());
   return grandPa->GetFilteredImageIndex(item);
}

void AutoCompList::SetItems(std::vector<wxString> items)
{
   m_FilteredItems = items;
   SetItemCount(m_FilteredItems.size());
   ClangCCLogger::Get()->Log(wxString::Format(_("%d items for CodeCompleteList."),m_FilteredItems.size()));
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
        int last = std::min<unsigned long>(m_FilteredItems.size() - firstVisible, 20);
		wxSize widest = *boost::max_element(m_FilteredItems | sliced(firstVisible,firstVisible + last)  //Should be quick enough
                                                            | transformed(boost::bind(&AutoCompList::GetTextExtent, this, _1)),
                                              boost::bind(std::less<int>(), boost::bind(&wxSize::GetWidth, _1), boost::bind(&wxSize::GetWidth, _2)));

        int maxWidth = std::min(widest.GetWidth() + 30, MAX_WIDTH); //some padding to count the images in
		SetColumnWidth(0,maxWidth);
		wxRect rect;
		GetItemRect(0,rect);
		int height = rect.GetHeight() * m_FilteredItems.size();
		SetClientSize(maxWidth, std::min(height,MAX_HEIGHT));
	}
	Refresh();
}
#if !wxCHECK_VERSION(2, 9, 4)
wxSize AutoCompList::GetTextExtent(const wxString& string)
{
    wxCoord w,h;
    wxWindow::GetTextExtent(string, &w, &h);
    return wxSize(w, h);
}

#endif
