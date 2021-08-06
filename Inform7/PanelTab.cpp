#include "stdafx.h"
#include "PanelTab.h"
#include "Inform.h"
#include "Panel.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(PanelTab, CTabCtrl)

BEGIN_MESSAGE_MAP(PanelTab, CTabCtrl)
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_NOTIFY_REFLECT(TCN_SELCHANGING, OnSelChanging)
END_MESSAGE_MAP()

PanelTab::PanelTab() : m_controller(NULL)
{
}

BOOL PanelTab::PreTranslateMessage(MSG* pMsg)
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

int PanelTab::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CTabCtrl::OnCreate(lpCreateStruct) == -1)
    return -1;
  SetFont(theApp.GetFont(this,InformApp::FontPanel));
  return 0;
}

BOOL PanelTab::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void PanelTab::OnPaint()
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

  CFont* oldFont = dc.SelectObject(GetFont());
  CPen* oldPen = dc.SelectObject(&linePen);
  dc.SetBkMode(TRANSPARENT);

  int base = client.bottom-1;
  dc.FillSolidRect(client,theApp.GetColour(InformApp::ColourTabBack));
  dc.MoveTo(0,base);
  dc.LineTo(client.right,base);

  int sel = GetCurSel();
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
  dc.SelectObject(oldFont);
  dc.SelectObject(oldPen);

  dcPaint.BitBlt(0,0,client.Width(),client.Height(),&dc,0,0,SRCCOPY);
  dc.SelectObject(oldBitmap);
}

void PanelTab::OnSelChanging(NMHDR*, LRESULT* pResult)
{
  TC_HITTESTINFO hit;
  ::GetCursorPos(&hit.pt);
  ScreenToClient(&hit.pt);
  int tab = HitTest(&hit);
  *pResult = ((tab >= 0) && !IsTabEnabled(tab));
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

void PanelTab::UpdateDPI(void)
{
  SetFont(theApp.GetFont(this,InformApp::FontPanel));
}

void PanelTab::SetTabController(TabController* controller)
{
  m_controller = controller;
}

bool PanelTab::SetActiveTab(int tab)
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

bool PanelTab::IsTabEnabled(int tab)
{
  if (m_controller != NULL)
    return m_controller->IsTabEnabled(tab);
  return true;
}

int PanelTab::NextEnabledTab(int currentTab, bool wrap)
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

int PanelTab::PrevEnabledTab(int currentTab, bool wrap)
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
