#include "stdafx.h"
#include "ButtonTab.h"
#include "Inform.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ButtonTab, CWnd)

BEGIN_MESSAGE_MAP(ButtonTab, CWnd)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_MOUSEMOVE()
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

ButtonTab::ButtonTab() : m_currentItem(-1), m_mouseOverItem(-1), m_mouseTrack(false)
{
}

int ButtonTab::GetDefaultHeight(void)
{
  CSize fontSize = theApp.MeasureFont(this,GetFont());
  return (int)(1.4*fontSize.cy);
}

CFont* ButtonTab::GetFont(void)
{
  return theApp.GetFont(this,InformApp::FontSystem);
}

int ButtonTab::GetItemCount(void) const
{
  return (int)m_items.size();
}

CString ButtonTab::GetItem(int item) const
{
  if ((item >= 0) && (item < m_items.size()))
    return m_items[item];
  return "";
}

CRect ButtonTab::GetItemRect(int item)
{
  CRect r(0,0,0,0);
  if ((item >= 0) && (item < m_items.size()))
  {
    CRect client;
    GetClientRect(client);
    r.bottom = client.Height();

    CSize fontSize = theApp.MeasureFont(this,GetFont());
    CDC* dc = GetDC();
    CFont* oldFont = dc->SelectObject(GetFont());

    for (int i = 0; i <= item; i++)
    {
      r.left = r.right;
      r.right += dc->GetTextExtent(GetItem(i)).cx;
      r.right += (int)(1.5*fontSize.cx);
    }

    dc->SelectObject(oldFont);
    ReleaseDC(dc);
  }
  return r;
}

void ButtonTab::InsertItem(int item, LPCSTR name)
{
  if (item >= m_items.size())
    m_items.resize(item+1);
  m_items[item] = name;
}

int ButtonTab::GetCurSel(void) const
{
  return m_currentItem;
}

void ButtonTab::SetCurSel(int item)
{
  if (m_currentItem != item)
  {
    m_currentItem = item;
    if (GetSafeHwnd() != 0)
      Invalidate();
  }
}

BOOL ButtonTab::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void ButtonTab::OnPaint()
{
  CRect client;
  GetClientRect(client);

  CPaintDC dcPaint(this);
  CDC dc;
  dc.CreateCompatibleDC(&dcPaint);

  CDibSection bitmap;
  if (bitmap.CreateBitmap(dc.GetSafeHdc(),client.Width(),client.Height()) == FALSE)
    return;
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&bitmap);
  dc.FillSolidRect(client,::GetSysColor(COLOR_BTNFACE));

  CFont* oldFont = dc.SelectObject(GetFont());
  dc.SetBkMode(TRANSPARENT);

  int sel = GetCurSel();
  for (int i = 0; i < GetItemCount(); i++)
  {
    CString text = GetItem(i);
    CRect itemRect = GetItemRect(i);
    itemRect.top = client.top;
    itemRect.bottom = client.bottom;

    if (i == sel)
      theApp.DrawSelectRect(dc,itemRect,false);
    else if (i == m_mouseOverItem)
      theApp.DrawSelectRect(dc,itemRect,true);

    if (text == "?H")
    {
      if (itemRect.Width() > itemRect.Height())
        itemRect.DeflateRect((itemRect.Width() - itemRect.Height())/2,0);
      itemRect.DeflateRect(itemRect.Height()/6,itemRect.Height()/6);
      CDibSection* dib = GetImage("Home",itemRect.Size());
      bitmap.AlphaBlend(dib,itemRect.left,itemRect.top,FALSE);
    }
    else
    {
      dc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
      dc.DrawText(text,itemRect,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
    }
  }

  dc.SelectObject(oldFont);
  dcPaint.BitBlt(0,0,client.Width(),client.Height(),&dc,0,0,SRCCOPY);
  dc.SelectObject(oldBitmap);
}

void ButtonTab::OnLButtonDown(UINT nFlags, CPoint point)
{
  CWnd::OnLButtonDown(nFlags,point);

  CPoint cursor = GetCurrentMessage()->pt;
  ScreenToClient(&cursor);

  int tab = -1;
  for (int i = 0; i < GetItemCount(); i++)
  {
    if (GetItemRect(i).PtInRect(cursor))
      tab = i;
  }
  if (tab >= 0)
    SetActiveTab(tab);
}

void ButtonTab::OnMouseMove(UINT nFlags, CPoint point)
{
  int hotTab = -1;
  for (int i = 0; i < GetItemCount(); i++)
  {
    if (GetItemRect(i).PtInRect(point))
      hotTab = i;
  }

  if (m_mouseOverItem != hotTab)
  {
    m_mouseOverItem = hotTab;
    Invalidate();

    if (!m_mouseTrack)
    {
      // Listen for the mouse leaving this control
      TRACKMOUSEEVENT tme;
      ::ZeroMemory(&tme,sizeof tme);
      tme.cbSize = sizeof tme;
      tme.dwFlags = TME_LEAVE;
      tme.hwndTrack = GetSafeHwnd();
      ::TrackMouseEvent(&tme);
      m_mouseTrack = true;
    }
  }
  CWnd::OnMouseMove(nFlags,point);
}

LRESULT ButtonTab::OnMouseLeave(WPARAM, LPARAM)
{
  if (m_mouseOverItem >= 0)
  {
    m_mouseOverItem = -1;
    m_mouseTrack = false;
    Invalidate();
  }
  return Default();
}

void ButtonTab::SetActiveTab(int tab)
{
  SetCurSel(tab);

  // Send TCN_SELCHANGE to the parent
  NMHDR hdr;
  hdr.hwndFrom = GetSafeHwnd();
  hdr.idFrom = GetDlgCtrlID();
  hdr.code = TCN_SELCHANGE;
  GetParent()->SendMessage(WM_NOTIFY,hdr.idFrom,(LPARAM)&hdr);
}

CDibSection* ButtonTab::GetImage(const char* name, const CSize& size)
{
  // Is the image in the cache?
  CString scaleName;
  scaleName.Format("%s-scaled-%ddpi",name,DPI::getWindowDPI(this));
  CDibSection* dib = theApp.GetCachedImage(scaleName);
  if (dib != NULL)
    return dib;

  // Create the scaled image
  CDibSection* original_dib = theApp.GetCachedImage(name);
  CSize original_size = original_dib->GetSize();
  double scaleX = (double)size.cx / (double)original_size.cx;
  double scaleY = (double)size.cy / (double)original_size.cy;
  dib = theApp.CreateScaledImage(original_dib,scaleX,scaleY);
  theApp.CacheImage(scaleName,dib);
  return dib;
}
