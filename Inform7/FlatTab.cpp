#include "stdafx.h"
#include "FlatTab.h"
#include "Inform.h"
#include "OSLayer.h"
#include "Panel.h"

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
  m_font.CreatePointFont(theApp.GetDialogFontSize(),theApp.GetFontName());
  SetFont(&m_font);
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

  int sel = GetCurSel();
  CPaintDC dc(this);

  CPen darkPen(PS_SOLID,0,::GetSysColor(COLOR_BTNTEXT));
  CPen shadowPen(PS_SOLID,0,::GetSysColor(COLOR_BTNSHADOW));

  CFont* oldFont = dc.SelectObject(GetFont());
  CPen* oldPen = dc.SelectObject(&darkPen);
  dc.SetBkMode(TRANSPARENT);

  if (m_buttons)
  {
    CPen highPen(PS_SOLID,0,::GetSysColor(COLOR_BTNHILIGHT));
    dc.FillSolidRect(client,::GetSysColor(COLOR_BTNFACE));

    // Draw the buttons the same size as any real buttons on the tab heading
    int off = theApp.MeasureFont(GetFont()).cy/5;
    client.InflateRect(0,2-off);

    if (theOS.IsAppThemed())
    {
      HTHEME theme = theOS.OpenThemeData(this,L"Button");
      if (theme)
      {
        theOS.DrawThemeBackground(theme,&dc,BP_PUSHBUTTON,PBS_NORMAL,client);
        theOS.GetThemeBackgroundContentRect(theme,&dc,BP_PUSHBUTTON,PBS_NORMAL,client);
        theOS.CloseThemeData(theme);
      }
    }
    else
      dc.DrawFrameControl(client,DFC_BUTTON,DFCS_BUTTONPUSH|DFCS_ADJUSTRECT);

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
        selectBitmap.LoadBitmap(IDR_TAB_SELECT);
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

      if ((i != sel) && (i != sel-1) && (i != GetItemCount()-1))
      {
        dc.SelectObject(shadowPen);
        int gap = itemRect.Height()/6;
        dc.MoveTo(itemRect.right,itemRect.top+gap);
        dc.LineTo(itemRect.right,itemRect.bottom-gap);
      }

      dc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
      dc.DrawText(text,itemRect,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
    }
  }
  else
  {
    COLORREF highColour = ::GetSysColor(COLOR_BTNHILIGHT);
    if (m_controller != NULL)
    {
      if (highColour == m_controller->GetTabColour(sel))
        highColour = ::GetSysColor(COLOR_BTNFACE);
    }
    CPen highPen(PS_SOLID,0,highColour);

    int base = client.bottom-1;
    dc.FillSolidRect(client,theApp.GetColour(InformApp::ColourTabBack));
    dc.SelectObject(&highPen);
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
        COLORREF high = ::GetSysColor(COLOR_BTNFACE);
        if (m_controller != NULL)
          high = m_controller->GetTabColour(i);
        dc.FillSolidRect(itemRect,high);

        dc.SelectObject(&darkPen);
        dc.MoveTo(itemRect.right,itemRect.bottom-1);
        dc.LineTo(itemRect.right,itemRect.top);
        dc.SelectObject(&highPen);
        dc.LineTo(itemRect.left,itemRect.top);
        dc.LineTo(itemRect.left,itemRect.bottom);
      }

      if ((i != sel) && (i != sel-1))
      {
        dc.SelectObject(shadowPen);
        int gap = itemRect.Height()/5;
        dc.MoveTo(itemRect.right,gap);
        dc.LineTo(itemRect.right,base+1-gap);
      }

      dc.SetTextColor(::GetSysColor(IsTabEnabled(i) ? COLOR_BTNTEXT : COLOR_GRAYTEXT));
      dc.DrawText(text,itemRect,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
    }
  }
  dc.SelectObject(oldFont);
  dc.SelectObject(oldPen);
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
