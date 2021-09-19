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
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_MOUSEMOVE()
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

PanelTab::PanelTab() : m_vertical(true), m_currentItem(-1), m_controller(NULL), m_mouseOverItem(-1), m_mouseTrack(false)
{
  EnableActiveAccessibility();

  // Get the orientation immediately so that the panel is laid out correctly
  CRegKey regKey;
  if (regKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_READ) == ERROR_SUCCESS)
    UpdateOrientation(regKey);
}

bool PanelTab::UpdateOrientation(CRegKey& key)
{
  DWORD horiz = 0;
  if (key.QueryDWORDValue("Tabs Horizontal",horiz) == ERROR_SUCCESS)
  {
    if (m_vertical != (horiz == 0))
    {
      m_vertical = (horiz == 0);
      if (GetSafeHwnd() != 0)
        Invalidate();
      return true;
    }
  }
  return false;
}

CFont* PanelTab::GetFont(void)
{
  return theApp.GetFont(this,m_vertical ? InformApp::FontVertical : InformApp::FontSystem);
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
    CRect client;
    GetClientRect(client);

    CSize fontSize = theApp.MeasureFont(this,GetFont());
    CDC* dc = GetDC();
    CFont* oldFont = dc->SelectObject(GetFont());

    if (m_vertical)
    {
      r.right = client.right;
      r.left = r.right - GetTabHeaderSize().cx;

      const double spaceFactor = 2.5;

      // Get the total size of all the tabs
      std::vector<int> sizes;
      sizes.resize(m_items.size());
      int totalSize = 0;
      for (int i = 0; i < m_items.size(); i++)
      {
        sizes[i] = dc->GetTextExtent(GetItem(i)).cx;
        totalSize += sizes[i];
        totalSize += (int)(spaceFactor*fontSize.cx);
      }

      // Get the rectangle for the tab so that the overall set of tabs are centred vertically
      r.top = (client.Height() - totalSize)/2;
      for (int i = 0; i < item; i++)
        r.top += sizes[i] + (int)(spaceFactor*fontSize.cx);
      r.bottom = r.top + sizes[item] + (int)(spaceFactor*fontSize.cx);
    }
    else
    {
      // Get the total size of all the tabs, without spacing
      std::vector<int> sizes;
      sizes.resize(m_items.size());
      int totalSize = 0;
      for (int i = 0; i < m_items.size(); i++)
      {
        sizes[i] = dc->GetTextExtent(GetItem(i)).cx;
        totalSize += sizes[i];
      }

      // Work out a suitable spacing factor, depending on how much space there is
      double spaceFactor = (double)(client.Width()-totalSize-4) / (double)(m_items.size()*fontSize.cx);
      if (spaceFactor > 2.5)
        spaceFactor = 2.5;
      if (spaceFactor < 1.0)
        spaceFactor = 1.0;

      // Get the rectangle for the tab
      r.bottom = GetTabHeaderSize().cy;
      for (int i = 0; i <= item; i++)
      {
        r.left = r.right;
        r.right += sizes[i];
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
  if (m_currentItem != item)
  {
    m_currentItem = item;
    if (GetSafeHwnd() != 0)
      Invalidate();
  }
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
        theApp.DrawSelectRect(dc,itemRect,false);
      else if ((i == m_mouseOverItem) && IsTabEnabled(i))
        theApp.DrawSelectRect(dc,itemRect,true);
      else
        dc.FillSolidRect(itemRect,::GetSysColor(COLOR_BTNFACE));

      dc.SetTextColor(::GetSysColor(IsTabEnabled(i) ? COLOR_BTNTEXT : COLOR_GRAYTEXT));
      CSize size = dc.GetTextExtent(text);
      dc.TextOut((itemRect.left+itemRect.right-size.cy)/2,(itemRect.top+itemRect.bottom-size.cx)/2,text);
    }
  }
  else
  {
    dc.SetTextAlign(TA_TOP|TA_LEFT);

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
      else if ((i == m_mouseOverItem) && IsTabEnabled(i))
      {
        itemRect.bottom--;
        dc.FillSolidRect(itemRect,::GetSysColor(COLOR_BTNHIGHLIGHT));

        dc.MoveTo(itemRect.right,itemRect.bottom-1);
        dc.LineTo(itemRect.right,itemRect.top);
        dc.LineTo(itemRect.left,itemRect.top);
        dc.LineTo(itemRect.left,itemRect.bottom);
        itemRect.bottom++;
      }

      dc.SetTextColor(::GetSysColor(IsTabEnabled(i) ? COLOR_BTNTEXT : COLOR_GRAYTEXT));
      dc.DrawText(text,itemRect,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
    }
    dc.SelectObject(oldPen);
  }

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

void PanelTab::OnMouseMove(UINT nFlags, CPoint point)
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

LRESULT PanelTab::OnMouseLeave(WPARAM, LPARAM)
{
  if (m_mouseOverItem >= 0)
  {
    m_mouseOverItem = -1;
    m_mouseTrack = false;
    Invalidate();
  }
  return Default();
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

HRESULT PanelTab::get_accChildCount(long* count)
{
  *count = GetItemCount();
  return S_OK;
}

HRESULT PanelTab::get_accChild(VARIANT child, IDispatch** disp)
{
  if (child.vt != VT_I4)
    return E_INVALIDARG;

  return S_FALSE;
}

HRESULT PanelTab::get_accName(VARIANT child, BSTR* accName)
{
  if (child.vt != VT_I4)
    return E_INVALIDARG;

  CString name;
  if (child.lVal == CHILDID_SELF)
    name = "Panels";
  else
    name = GetItem(child.lVal-1);

  *accName = name.AllocSysString();
  return S_OK;
}

HRESULT PanelTab::get_accRole(VARIANT child, VARIANT* role)
{
  if (child.vt != VT_I4)
    return E_INVALIDARG;

  role->vt = VT_I4;
  role->lVal = (child.lVal == CHILDID_SELF) ?
    ROLE_SYSTEM_PAGETABLIST : ROLE_SYSTEM_PAGETAB;
  return S_OK;
}

HRESULT PanelTab::get_accState(VARIANT child, VARIANT* state)
{
  if (child.vt != VT_I4)
    return E_INVALIDARG;

  state->vt = VT_I4;
  state->lVal = 0;

  if ((child.lVal > 0) && (m_currentItem == child.lVal-1))
    state->lVal = STATE_SYSTEM_SELECTED;
  return S_OK;
}

HRESULT PanelTab::accDoDefaultAction(VARIANT child)
{
  if (child.vt != VT_I4)
    return E_INVALIDARG;

  if (child.lVal != CHILDID_SELF)
  {
    if (SetActiveTab(child.lVal-1))
      return S_OK;
  }
  return S_FALSE;
}

HRESULT PanelTab::accHitTest(long left, long top, VARIANT* child)
{
  child->vt = VT_I4;
  child->lVal = CHILDID_SELF;

  CPoint point(left,top);
  ScreenToClient(&point);

  for (int i = 0; i < GetItemCount(); i++)
  {
    if (GetItemRect(i).PtInRect(point))
      child->lVal = i+1;
  }
  return S_OK;
}

HRESULT PanelTab::accLocation(long* left, long* top, long* width, long* height, VARIANT child)
{
  if (child.vt != VT_I4)
    return E_INVALIDARG;

  CRect rect;
  if (child.lVal == CHILDID_SELF)
    GetWindowRect(rect);
  else
  {
    CRect winRect;
    GetWindowRect(winRect);
    rect = GetItemRect(child.lVal-1);
    rect.OffsetRect(winRect.TopLeft());
  }

  *left = rect.left;
  *top = rect.top;
  *width = rect.Width();
  *height = rect.Height();
  return S_OK;
}
