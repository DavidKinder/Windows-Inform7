#include "stdafx.h"
#include "PanelTab.h"
#include "Inform.h"
#include "Panel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(PanelTab, CWnd)

BEGIN_MESSAGE_MAP(PanelTab, CWnd)
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONDOWN()
  ON_WM_PAINT()
END_MESSAGE_MAP()

PanelTab::PanelTab() : m_vertical(false), m_currentItem(-1), m_controller(NULL)
{
}

CFont* PanelTab::GetFont(void)
{
  return theApp.GetFont(this,m_vertical ? InformApp::FontVertical : InformApp::FontPanel);
}

int PanelTab::GetItemCount(void) const
{
  return (int)m_items.size();
}

CString PanelTab::GetItem(int item) const
{
  if ((item >= 0) && (item < m_items.size()))
    return m_items[item];
  return "";
}

CRect PanelTab::GetItemRect(int item)
{
  CRect r(0,0,0,0);
  if ((item >= 0) && (item < m_items.size()))
  {
    CSize fontSize = theApp.MeasureFont(this,GetFont());
    CDC* dc = GetDC();
    CFont* oldFont = dc->SelectObject(GetFont());

    if (m_vertical)
    {
      CRect client;
      GetClientRect(client);
      r.right = client.right;
      r.left = r.right - GetTabHeaderSize().cx;

      const double spaceFactor = 2.5;

      std::vector<int> sizes;
      sizes.resize(m_items.size());
      int totalSize = 0;
      for (int i = 0; i < m_items.size(); i++)
      {
        sizes[i] = dc->GetTextExtent(GetItem(i)).cx;
        totalSize += sizes[i];
        totalSize += (int)(spaceFactor*fontSize.cx);
      }

      r.top = (client.Height() - totalSize)/2;
      for (int i = 0; i < item; i++)
        r.top += sizes[i] + (int)(spaceFactor*fontSize.cx);
      r.bottom = r.top + sizes[item] + (int)(spaceFactor*fontSize.cx);
    }
    else
    {
      const double spaceFactor = 1.8;

      r.bottom = GetTabHeaderSize().cy;
      for (int i = 0; i <= item; i++)
      {
        r.left = r.right;
        r.right += dc->GetTextExtent(GetItem(i)).cx;
        r.right += (int)(spaceFactor*fontSize.cx);
      }
    }

    dc->SelectObject(oldFont);
    ReleaseDC(dc);
  }
  return r;
}

void PanelTab::InsertItem(int item, LPCSTR name)
{
  if (item >= m_items.size())
    m_items.resize(item+1);
  m_items[item] = name;
}

int PanelTab::GetCurSel(void) const
{
  return m_currentItem;
}

void PanelTab::SetCurSel(int item)
{
  m_currentItem = item;
}

BOOL PanelTab::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void PanelTab::OnPaint()
{
  CRect client;
  GetClientRect(client);
  int bitmapX = 0;

  if (m_vertical)
  {
    int w = GetTabHeaderSize().cx;
    bitmapX = client.Width() - w;
    client.right = client.left + w;
  }
  else
    client.bottom = client.top + GetTabHeaderSize().cy;

  CPaintDC dcPaint(this);
  CDC dc;
  dc.CreateCompatibleDC(&dcPaint);
  dc.SetBkMode(TRANSPARENT);

  CDibSection bitmap;
  if (bitmap.CreateBitmap(dc.GetSafeHdc(),client.Width(),client.Height()) == FALSE)
    return;
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&bitmap);
  CFont* oldFont = dc.SelectObject(GetFont());

  // Fill the background colour
  dc.FillSolidRect(client,theApp.GetColour(InformApp::ColourTabBack));

  // Get the colour for the lines around tabs: use the theme, if possible
  COLORREF lineColour = ::GetSysColor(COLOR_BTNSHADOW);
  if (::IsAppThemed())
  {
    HTHEME theme = ::OpenThemeData(GetSafeHwnd(),L"TAB");
    if (theme != 0)
    {
      COLORREF themeColour = 0;
      if (SUCCEEDED(::GetThemeColor(theme,TABP_TABITEM,TIBES_NORMAL,TMT_EDGEFILLCOLOR,&themeColour)))
      {
        if (themeColour != 0)
          lineColour = themeColour;
      }
      ::CloseThemeData(theme);
    }
  }
  CPen linePen(PS_SOLID,0,lineColour);
  CPen* oldPen = dc.SelectObject(&linePen);

  if (m_vertical)
  {
    dc.SetTextAlign(TA_BOTTOM|TA_LEFT);

    // Draw each tab
    int sel = GetCurSel();
    for (int i = 0; i < GetItemCount(); i++)
    {
      CString text = GetItem(i);
      CRect itemRect = GetItemRect(i);
      itemRect.OffsetRect(CPoint(-bitmapX,0));

      if (i == sel)
      {
        bool drawn = false;
        if (::IsAppThemed())
        {
          HTHEME theme = ::OpenThemeData(GetSafeHwnd(),L"Menu");
          if (theme)
          {
            ::DrawThemeBackground(theme,dc.GetSafeHdc(),MENU_BARITEM,MBI_PUSHED,itemRect,NULL);
            ::CloseThemeData(theme);
            drawn = true;
          }
        }
        if (!drawn)
          dc.FillSolidRect(itemRect,::GetSysColor(COLOR_HIGHLIGHT));
      }
      else
      {
        dc.FillSolidRect(itemRect,::GetSysColor(COLOR_BTNFACE));

        dc.MoveTo(itemRect.left,itemRect.bottom);
        dc.LineTo(itemRect.left,itemRect.top);
        dc.LineTo(itemRect.right-1,itemRect.top);
        dc.LineTo(itemRect.right-1,itemRect.bottom);
        if (i == GetItemCount()-1)
          dc.LineTo(itemRect.left,itemRect.bottom);
      }

      dc.SetTextColor(::GetSysColor(IsTabEnabled(i) ? COLOR_BTNTEXT : COLOR_GRAYTEXT));
      CSize size = dc.GetTextExtent(text);
      dc.TextOut((itemRect.left+itemRect.right-size.cy)/2,(itemRect.top+itemRect.bottom-size.cx)/2,text);
    }
  }
  else
  {
    int base = client.bottom-1;
    dc.MoveTo(0,base);
    dc.LineTo(client.right,base);

    // Draw each tab
    int sel = GetCurSel();
    for (int i = 0; i < GetItemCount(); i++)
    {
      CString text = GetItem(i);
      CRect itemRect = GetItemRect(i);
      itemRect.bottom = base+1;

      if (i == sel)
      {
        dc.FillSolidRect(itemRect,::GetSysColor(COLOR_BTNFACE));

        dc.MoveTo(itemRect.right,itemRect.bottom-1);
        dc.LineTo(itemRect.right,itemRect.top);
        dc.LineTo(itemRect.left,itemRect.top);
        dc.LineTo(itemRect.left,itemRect.bottom);
      }

      dc.SetTextColor(::GetSysColor(IsTabEnabled(i) ? COLOR_BTNTEXT : COLOR_GRAYTEXT));
      dc.DrawText(text,itemRect,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
    }
  }

  dc.SelectObject(oldPen);
  dc.SelectObject(oldFont);
  dcPaint.BitBlt(bitmapX,0,client.Width(),client.Height(),&dc,0,0,SRCCOPY);
  dc.SelectObject(oldBitmap);
}

void PanelTab::OnLButtonDown(UINT nFlags, CPoint point)
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

BOOL PanelTab::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  // Get the parent panel
  CWnd* parent = GetParent();
  if (parent->IsKindOf(RUNTIME_CLASS(Panel)))
  {
    Panel* panel = (Panel*)parent;

    // Let the active tab process the command first
    Panel::Tabs tab = panel->GetActiveTab();
    if (tab != Panel::No_Tab)
    {
      if (panel->GetTab(tab)->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
        return TRUE;
    }
  }
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void PanelTab::SetTabController(TabController* controller)
{
  m_controller = controller;
}

CSize PanelTab::GetTabHeaderSize(void)
{
  CSize fontSize = theApp.MeasureFont(this,GetFont());
  if (m_vertical)
    return CSize((int)(1.2*fontSize.cy),0);
  else
    return CSize(0,(int)(1.4*fontSize.cy));
}

bool PanelTab::SetActiveTab(int tab)
{
  if (IsTabEnabled(tab))
  {
    SetCurSel(tab);

    // Send TCN_SELCHANGE to the parent
    NMHDR hdr;
    hdr.hwndFrom = GetSafeHwnd();
    hdr.idFrom = GetDlgCtrlID();
    hdr.code = TCN_SELCHANGE;
    GetParent()->SendMessage(WM_NOTIFY,hdr.idFrom,(LPARAM)&hdr);
    return TRUE;
  }
  return FALSE;
}

bool PanelTab::IsTabEnabled(int tab)
{
  if (m_controller != NULL)
    return m_controller->IsTabEnabled(tab);
  return true;
}
