#include "stdafx.h"
#include "FlatTab.h"
#include "Inform.h"
#include "Panel.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(FlatTab, CTabCtrl)

BEGIN_MESSAGE_MAP(FlatTab, CTabCtrl)
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_NOTIFY_REFLECT(TCN_SELCHANGING, OnSelChanging)
END_MESSAGE_MAP()

FlatTab::FlatTab(bool buttons) : m_buttons(buttons), m_controller(NULL)
{
}

BOOL FlatTab::PreTranslateMessage(MSG* pMsg)
{
  if ((pMsg->hwnd == GetSafeHwnd()) && (pMsg->message == WM_KEYDOWN))
  {
    if ((pMsg->wParam == VK_LEFT) || (pMsg->wParam == VK_RIGHT))
    {
      int tab = (pMsg->wParam == VK_LEFT) ?
        PrevEnabledTab(GetCurSel(),false) : NextEnabledTab(GetCurSel(),false);
      if (tab >= 0)
        SetActiveTab(tab);
      return TRUE;
    }
  }
  return CTabCtrl::PreTranslateMessage(pMsg);
}

int FlatTab::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CTabCtrl::OnCreate(lpCreateStruct) == -1)
    return -1;
  SetFont(theApp.GetFont(this,InformApp::FontPanel));
  return 0;
}

BOOL FlatTab::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void FlatTab::OnPaint()
{
  CRect client;
  GetClientRect(client);
  CRect below(client);
  AdjustRect(FALSE,below);
  client.bottom = below.top;

  CPaintDC dcPaint(this);
  CDC dc;
  dc.CreateCompatibleDC(&dcPaint);

  CDibSection bitmap;
  if (bitmap.CreateBitmap(dc.GetSafeHdc(),client.Width(),client.Height()) == FALSE)
    return;
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&bitmap);

  // Get the colour for the lines around tabs: use the theme, if possible
  COLORREF lineColour = ::GetSysColor(COLOR_BTNSHADOW);
  if (!m_buttons)
  {
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
  }
  CPen linePen(PS_SOLID,0,lineColour);

  CFont* oldFont = dc.SelectObject(GetFont());
  CPen* oldPen = dc.SelectObject(&linePen);
  dc.SetBkMode(TRANSPARENT);

  int sel = GetCurSel();

  if (m_buttons)
  {
    dc.FillSolidRect(client,::GetSysColor(COLOR_BTNFACE));

    for (int i = 0; i < GetItemCount(); i++)
    {
      TCITEM item;
      ::ZeroMemory(&item,sizeof item);
      item.mask = TCIF_TEXT;

      CString text;
      item.pszText = text.GetBufferSetLength(256);
      item.cchTextMax = 256;
      GetItem(i,&item);
      text.ReleaseBuffer();

      CRect itemRect;
      GetItemRect(i,itemRect);
      itemRect.top = client.top;
      itemRect.bottom = client.bottom;

      if (i == sel)
      {
        // Get the bitmap to indicate a selected button
        CBitmap selectBitmap;
        selectBitmap.LoadBitmap(IDR_FLAT_SELECT);
        CDC selectDC;
        selectDC.CreateCompatibleDC(&dc);
        CBitmap* oldBitmap = selectDC.SelectObject(&selectBitmap);

        // Get the bitmap's dimensions
        BITMAP bitmapInfo;
        selectBitmap.GetBitmap(&bitmapInfo);

        // Stretch the bitmap into the selected item's background
        dc.StretchBlt(itemRect.left,itemRect.top,itemRect.Width(),itemRect.Height(),
          &selectDC,0,0,bitmapInfo.bmWidth,bitmapInfo.bmHeight,SRCCOPY);
        selectDC.SelectObject(oldBitmap);
      }
      else
      {
        if (i == 0)
        {
          int gap = itemRect.Height()/6;
          dc.MoveTo(itemRect.left,itemRect.top+gap);
          dc.LineTo(itemRect.left,itemRect.bottom-gap);
        }
        if ((i != sel-1) && (i != GetItemCount()-1))
        {
          int gap = itemRect.Height()/6;
          dc.MoveTo(itemRect.right,itemRect.top+gap);
          dc.LineTo(itemRect.right,itemRect.bottom-gap);
        }
      }

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
  }
  else
  {
    int base = client.bottom-1;
    dc.FillSolidRect(client,theApp.GetColour(InformApp::ColourTabBack));
    dc.MoveTo(0,base);
    dc.LineTo(client.right,base);

    for (int i = 0; i < GetItemCount(); i++)
    {
      TCITEM item;
      ::ZeroMemory(&item,sizeof item);
      item.mask = TCIF_TEXT;

      CString text;
      item.pszText = text.GetBufferSetLength(256);
      item.cchTextMax = 256;
      GetItem(i,&item);
      text.ReleaseBuffer();

      CRect itemRect;
      GetItemRect(i,itemRect);
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
  dc.SelectObject(oldFont);
  dc.SelectObject(oldPen);

  dcPaint.BitBlt(0,0,client.Width(),client.Height(),&dc,0,0,SRCCOPY);
  dc.SelectObject(oldBitmap);
}

void FlatTab::OnSelChanging(NMHDR*, LRESULT* pResult)
{
  TC_HITTESTINFO hit;
  ::GetCursorPos(&hit.pt);
  ScreenToClient(&hit.pt);
  int tab = HitTest(&hit);
  *pResult = ((tab >= 0) && !IsTabEnabled(tab));
}

BOOL FlatTab::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
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

void FlatTab::UpdateDPI(void)
{
  SetFont(theApp.GetFont(this,InformApp::FontPanel));
}

void FlatTab::SetTabController(TabController* controller)
{
  m_controller = controller;
}

void FlatTab::SelectNextTab(bool forward)
{
  int tab = forward ?
    NextEnabledTab(GetCurSel(),true) : PrevEnabledTab(GetCurSel(),true);
  if (tab >= 0)
    SetActiveTab(tab);
}

bool FlatTab::SetActiveTab(int tab)
{
  // Send TCN_SELCHANGING to the parent
  NMHDR hdr;
  hdr.hwndFrom = GetSafeHwnd();
  hdr.idFrom = GetDlgCtrlID();
  hdr.code = TCN_SELCHANGING;
  if (GetParent()->SendMessage(WM_NOTIFY,hdr.idFrom,(LPARAM)&hdr) >= 0)
  {
    SetCurSel(tab);

    // Send TCN_SELCHANGE to the parent
    hdr.code = TCN_SELCHANGE;
    GetParent()->SendMessage(WM_NOTIFY,hdr.idFrom,(LPARAM)&hdr);
    return TRUE;
  }
  return FALSE;
}

bool FlatTab::IsTabEnabled(int tab)
{
  if (m_controller != NULL)
    return m_controller->IsTabEnabled(tab);
  return true;
}

int FlatTab::NextEnabledTab(int currentTab, bool wrap)
{
  int tabs = GetItemCount();
  for (int tab = currentTab+1; tab != currentTab; tab++)
  {
    if (tab >= tabs)
    {
      if (!wrap)
        return -1;
      tab = 0;
    }
    if (IsTabEnabled(tab))
      return tab;
  }
  return -1;
}

int FlatTab::PrevEnabledTab(int currentTab, bool wrap)
{
  for (int tab = currentTab-1; tab != currentTab; tab--)
  {
    if (tab < 0)
    {
      if (!wrap)
        return -1;
      tab = GetItemCount()-1;
    }
    if (IsTabEnabled(tab))
      return tab;
  }
  return -1;
}

CDibSection* FlatTab::GetImage(const char* name, const CSize& size)
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
