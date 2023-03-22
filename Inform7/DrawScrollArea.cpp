#include "stdafx.h"
#include "DrawScrollArea.h"
#include "Inform.h"

#include "Dib.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(DrawScrollArea, DrawScrollWindow)
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_MOUSEWHEEL()
  ON_MESSAGE(WM_MBUTTONDOWN, OnMButtonDown)
END_MESSAGE_MAP()

void DrawScrollArea::SetScrollSizes(int mapMode, SIZE sizeTotal, const SIZE& sizePage, const SIZE& sizeLine)
{
  ASSERT((sizeTotal.cx >= 0) && (sizeTotal.cy >= 0));
  ASSERT(mapMode == MM_TEXT);

  m_total = sizeTotal;
  m_page = sizePage;
  m_line = sizeLine;

  ASSERT((m_total.cx >= 0) && (m_total.cy >= 0));
  if (m_page.cx == 0)
    m_page.cx = m_total.cx / 10;
  if (m_page.cy == 0)
    m_page.cy = m_total.cy / 10;
  if (m_line.cx == 0)
    m_line.cx = m_page.cx / 10;
  if (m_line.cy == 0)
    m_line.cy = m_page.cy / 10;

  if (GetSafeHwnd() != 0)
    UpdateBars();
}

CPoint DrawScrollArea::GetDeviceScrollPosition()
{
  CPoint pos(GetScrollPos(SB_HORZ),GetScrollPos(SB_VERT));
  ASSERT((pos.x >= 0) && (pos.y >= 0));
  return pos;
}

void DrawScrollArea::ScrollToDevicePosition(POINT point)
{
  ASSERT(point.x >= 0);
  ASSERT(point.y >= 0);

  int xOrig = GetScrollPos(SB_HORZ);
  SetScrollPos(SB_HORZ,point.x);
  int yOrig = GetScrollPos(SB_VERT);
  SetScrollPos(SB_VERT,point.y);
  ScrollWindow(xOrig - point.x,yOrig - point.y);
}

void DrawScrollArea::UpdateBars()
{
  if (m_insideUpdate)
    return;
  m_insideUpdate = true;

  ASSERT((m_total.cx >= 0) && (m_total.cy >= 0));

  CSize sizeClient, sizeBars;
  if (!GetTrueClientSize(sizeClient,sizeBars))
  {
    CRect r;
    GetClientRect(&r);
    if ((r.right > 0) && (r.bottom > 0))
    {
      EnableScrollBarCtrl(SB_BOTH,FALSE);
    }
    m_insideUpdate = false;
    return;
  }

  CSize sizeRange, needBars;
  CPoint moveTo;
  GetScrollBarState(sizeClient,needBars,sizeRange,moveTo,TRUE);
  if (needBars.cx)
    sizeClient.cy -= sizeBars.cy;
  if (needBars.cy)
    sizeClient.cx -= sizeBars.cx;

  ScrollToDevicePosition(moveTo);

  SCROLLINFO info;
  info.fMask = SIF_PAGE|SIF_RANGE;
  info.nMin = 0;

  EnableScrollBarCtrl(SB_HORZ,needBars.cx);
  if (needBars.cx)
  {
    info.nPage = sizeClient.cx;
    info.nMax = m_total.cx-1;
    SetScrollInfo(SB_HORZ,&info,TRUE);
  }

  EnableScrollBarCtrl(SB_VERT,needBars.cy);
  if (needBars.cy)
  {
    info.nPage = sizeClient.cy;
    info.nMax = m_total.cy-1;
    SetScrollInfo(SB_VERT,&info,TRUE);
  }

  m_insideUpdate = false;
}

BOOL DrawScrollArea::GetTrueClientSize(CSize& size, CSize& sizeBars)
{
  CRect rc;
  GetClientRect(&rc);

  size.cx = rc.Width();
  size.cy = rc.Height();
  GetScrollBarSizes(sizeBars);

  if ((sizeBars.cx != 0) && m_v.visible)
    size.cx += sizeBars.cx;
  if ((sizeBars.cy != 0) && m_h.visible)
    size.cy += sizeBars.cy;
  return ((size.cx > sizeBars.cx) && (size.cy > sizeBars.cy));
}

void DrawScrollArea::GetScrollBarSizes(CSize& sizeBars)
{
  int dpi = DPI::getWindowDPI(this);
  sizeBars.cx = DPI::getSystemMetrics(SM_CXVSCROLL,dpi);
  sizeBars.cy = DPI::getSystemMetrics(SM_CYHSCROLL,dpi);
}

void DrawScrollArea::GetScrollBarState(CSize sizeClient, CSize& needBars, CSize& sizeRange, CPoint& moveTo, BOOL insideClient)
{
  CSize sizeBars;
  GetScrollBarSizes(sizeBars);

  sizeRange = m_total - sizeClient;
  moveTo = GetDeviceScrollPosition();

  bool needH = (sizeRange.cx > 0);
  if (!needH)
    moveTo.x = 0;
  else if (insideClient)
    sizeRange.cy += sizeBars.cy;

  bool needV = (sizeRange.cy > 0);
  if (!needV)
    moveTo.y = 0;
  else if (insideClient)
    sizeRange.cx += sizeBars.cx;

  if (needV && !needH && (sizeRange.cx > 0))
  {
    ASSERT(insideClient);

    needH = true;
    sizeRange.cy += sizeBars.cy;
  }

  if ((sizeRange.cx > 0) && (moveTo.x >= sizeRange.cx))
    moveTo.x = sizeRange.cx;
  if ((sizeRange.cy > 0) && (moveTo.y >= sizeRange.cy))
    moveTo.y = sizeRange.cy;

  needBars.cx = needH;
  needBars.cy = needV;
}

void DrawScrollArea::OnPrepareDC(CDC* dc)
{
  ASSERT((m_total.cx >= 0) && (m_total.cy >= 0));

  dc->SetMapMode(MM_TEXT);

  CPoint vpOrg(0,0);
  if (!dc->IsPrinting())
    vpOrg = -GetDeviceScrollPosition();
  dc->SetViewportOrg(vpOrg);
}

BOOL DrawScrollArea::OnScroll(UINT code, UINT pos, BOOL doScroll)
{
  int x = GetScrollPos(SB_HORZ);
  int xOrig = x;
  switch (LOBYTE(code))
  {
  case SB_TOP:
    x = 0;
    break;
  case SB_BOTTOM:
    x = INT_MAX;
    break;
  case SB_LINEUP:
    x -= m_line.cx;
    break;
  case SB_LINEDOWN:
    x += m_line.cx;
    break;
  case SB_PAGEUP:
    x -= m_page.cx;
    break;
  case SB_PAGEDOWN:
    x += m_page.cx;
    break;
  case SB_THUMBTRACK:
    x = pos;
    break;
  }

  int y = GetScrollPos(SB_VERT);
  int yOrig = y;
  switch (HIBYTE(code))
  {
  case SB_TOP:
    y = 0;
    break;
  case SB_BOTTOM:
    y = INT_MAX;
    break;
  case SB_LINEUP:
    y -= m_line.cy;
    break;
  case SB_LINEDOWN:
    y += m_line.cy;
    break;
  case SB_PAGEUP:
    y -= m_page.cy;
    break;
  case SB_PAGEDOWN:
    y += m_page.cy;
    break;
  case SB_THUMBTRACK:
    y = pos;
    break;
  }

  return OnScrollBy(CSize(x - xOrig, y - yOrig),doScroll);
}

BOOL DrawScrollArea::OnScrollBy(CSize sizeScroll, BOOL doScroll)
{
  int xOrig, x;
  int yOrig, y;

  if (!m_v.visible)
    sizeScroll.cy = 0;
  if (!m_h.visible)
    sizeScroll.cx = 0;

  xOrig = x = GetScrollPos(SB_HORZ);
  int xMax = GetScrollLimit(SB_HORZ);
  x += sizeScroll.cx;
  if (x < 0)
    x = 0;
  else if (x > xMax)
    x = xMax;

  yOrig = y = GetScrollPos(SB_VERT);
  int yMax = GetScrollLimit(SB_VERT);
  y += sizeScroll.cy;
  if (y < 0)
    y = 0;
  else if (y > yMax)
    y = yMax;

  if ((x == xOrig) && (y == yOrig))
    return FALSE;

  if (doScroll)
  {
    ScrollWindow(-(x-xOrig),-(y-yOrig));
    if (x != xOrig)
      SetScrollPos(SB_HORZ,x);
    if (y != yOrig)
      SetScrollPos(SB_VERT,y);
  }
  return TRUE;
}

CSize DrawScrollArea::GetWheelScrollDistance(CSize distance, BOOL horz, BOOL vert)
{
  CSize result;
  result.cx = horz ? distance.cx / 10 : 0;
  result.cy = vert ? distance.cy / 10 : 0;
  return result;
}

void DrawScrollArea::OnPaint()
{
  CPaintDC dc(this);
  OnPrepareDC(&dc);
  OnDraw(&dc);
}

void DrawScrollArea::OnSize(UINT type, int cx, int cy)
{
  DrawScrollWindow::OnSize(type,cx,cy);
  UpdateBars();
}

void DrawScrollArea::OnHScroll(UINT code, UINT pos, CScrollBar* bar)
{
  if ((bar != NULL) && bar->SendChildNotifyLastMsg())
    return;
  if (bar == NULL)
  {
    if ((code == SB_THUMBPOSITION) || (code == SB_THUMBTRACK))
    {
      SCROLLINFO si = { sizeof (SCROLLINFO), 0 };
      GetScrollInfo(SB_HORZ,&si);
      pos = si.nTrackPos;
    }
    OnScroll(MAKEWORD(code,0xFF),pos);
  }
}

void DrawScrollArea::OnVScroll(UINT code, UINT pos, CScrollBar* bar)
{
  if ((bar != NULL) && bar->SendChildNotifyLastMsg())
    return;
  if (bar == NULL)
  {
    if ((code == SB_THUMBPOSITION) || (code == SB_THUMBTRACK))
    {
      SCROLLINFO si = { sizeof (SCROLLINFO), 0 };
      GetScrollInfo(SB_VERT,&si);
      pos = si.nTrackPos;
    }
    OnScroll(MAKEWORD(0xFF,code),pos);
  }
}

BOOL DrawScrollArea::OnMouseWheel(UINT flags, short delta, CPoint point)
{
  if (flags & (MK_SHIFT|MK_CONTROL))
    return FALSE;

  if (!m_v.visible && !m_h.visible)
    return FALSE;

  static UINT scrollLines = 0;
  if (scrollLines == 0)
  {
    scrollLines = 3;
    ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,&scrollLines,0);
  }

  BOOL result = FALSE;
  int toScroll = ::MulDiv(-delta,scrollLines,WHEEL_DELTA);
  int displace;

  if (m_v.visible)
  {
    if (scrollLines == WHEEL_PAGESCROLL)
    {
      displace = m_page.cy;
      if (delta > 0)
        displace = -displace;
    }
    else
    {
      displace = toScroll * m_line.cy;
      displace = min(displace,m_page.cy);
    }
    result = OnScrollBy(CSize(0,displace),TRUE);
  }
  else if (m_h.visible)
  {
    if (scrollLines == WHEEL_PAGESCROLL)
    {
      displace = m_page.cx;
      if (delta > 0)
        displace = -displace;
    }
    else
    {
      displace = toScroll * m_line.cx;
      displace = min(displace,m_page.cx);
    }
    result = OnScrollBy(CSize(displace,0),TRUE);
  }

  if (result)
    UpdateWindow();
  return result;
}

LRESULT DrawScrollArea::OnMButtonDown(WPARAM wp, LPARAM lp)
{
  if (wp & (MK_SHIFT|MK_CONTROL))
    return FALSE;

  if (::GetSystemMetrics(SM_MOUSEWHEELPRESENT) == 0)
    return TRUE;

  if (m_anchorWindow == NULL)
  {
    UINT bitmapID = 0;
    if (m_v.visible)
    {
      if (m_h.visible)
        bitmapID = AFX_IDC_MOUSE_ORG_HV;
      else
        bitmapID = AFX_IDC_MOUSE_ORG_VERT;
    }
    else if (m_h.visible)
      bitmapID = AFX_IDC_MOUSE_ORG_HORZ;

    if (bitmapID == 0)
      return FALSE;

    m_anchorWindow = new DrawScrollMouseAnchorWnd(CPoint(lp));
    m_anchorWindow->SetBitmap(bitmapID);
    m_anchorWindow->Create(this);
    m_anchorWindow->ShowWindow(SW_SHOWNA);
  }
  else
  {
    m_anchorWindow->DestroyWindow();
    delete m_anchorWindow;
    m_anchorWindow = NULL;
  }
  return TRUE;
}

#define ID_TIMER_TRACKING 0xE000

BEGIN_MESSAGE_MAP(DrawScrollMouseAnchorWnd, CWnd)
  ON_WM_TIMER()
END_MESSAGE_MAP()

DrawScrollMouseAnchorWnd::DrawScrollMouseAnchorWnd(CPoint anchor)
  : m_size(32,33), m_anchor(anchor), m_quitTracking(false)
{
}

BOOL DrawScrollMouseAnchorWnd::Create(DrawScrollArea* parent)
{
  ASSERT(parent != NULL);

  parent->ClientToScreen(&m_anchor);

  int dpi = DPI::getWindowDPI(parent);
  m_drag.top = m_anchor.y - DPI::getSystemMetrics(SM_CYDOUBLECLK,dpi);
  m_drag.bottom = m_anchor.y + DPI::getSystemMetrics(SM_CYDOUBLECLK,dpi);
  m_drag.left = m_anchor.x - DPI::getSystemMetrics(SM_CXDOUBLECLK,dpi);
  m_drag.right = m_anchor.x + DPI::getSystemMetrics(SM_CXDOUBLECLK,dpi);

  BOOL created = CreateEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_LAYERED,
    AfxRegisterWndClass(CS_SAVEBITS),NULL,WS_POPUP,
    m_anchor.x - m_size.cx/2,m_anchor.y - m_size.cy/2,m_size.cx,m_size.cy,NULL,NULL);
  SetOwner(parent);

  if (created)
  {
    HDC dc = ::GetDC(NULL);
    CDC dcMem;
    dcMem.CreateCompatibleDC(CDC::FromHandle(dc));
    CDibSection bitmap;
    if (bitmap.CreateBitmap(dc,m_size.cx,m_size.cy))
    {
      CBitmap* oldBitmap = CDibSection::SelectDibSection(dcMem,&bitmap);

      // Copy the circle image
      CDibSection* circle = NULL;
      switch (m_anchorID)
      {
      case AFX_IDC_MOUSE_ORG_HV:
        circle = theApp.GetCachedImage("Anchor-pan");
        break;
      case AFX_IDC_MOUSE_ORG_HORZ:
        circle = theApp.GetCachedImage("Anchor-pan-horizontal");
        break;
      case AFX_IDC_MOUSE_ORG_VERT:
        circle = theApp.GetCachedImage("Anchor-pan-vertical");
        break;
      }
      ASSERT(circle != NULL);
      if (circle != NULL)
      {
        ASSERT(circle->GetSize().cx == m_size.cx);
        ASSERT(circle->GetSize().cy == m_size.cy);
        memcpy(bitmap.GetBits(),circle->GetBits(),m_size.cx * m_size.cy * sizeof (DWORD));
      }

      // Use the bitmap as the alpha blended image for this window
      CPoint layerTopLeft(0,0);
      BLENDFUNCTION layerBlend = { AC_SRC_OVER,0,0xFF,AC_SRC_ALPHA };
      ::UpdateLayeredWindow(GetSafeHwnd(),dc,
        NULL,&m_size,dcMem.GetSafeHdc(),&layerTopLeft,0,&layerBlend,ULW_ALPHA);

      dcMem.SelectObject(&oldBitmap);
    }
    ::ReleaseDC(NULL,dc);

    SetCapture();
    SetTimer(ID_TIMER_TRACKING,50,NULL);
  }
  return created;
}

void DrawScrollMouseAnchorWnd::SetBitmap(UINT id)
{
  HINSTANCE inst = AfxFindResourceHandle(MAKEINTRESOURCE(id),RT_GROUP_CURSOR);
  ASSERT(inst != NULL);
  m_anchorCursor = ::LoadCursor(inst,MAKEINTRESOURCE(id));
  m_anchorID = id;
}

BOOL DrawScrollMouseAnchorWnd::PreTranslateMessage(MSG* msg)
{
  BOOL result = FALSE;

  switch (msg->message)
  {
  case WM_MOUSEWHEEL:
  case WM_KEYDOWN:
  case WM_CHAR:
  case WM_KEYUP:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_MBUTTONDOWN:
    m_quitTracking = TRUE;
    result = TRUE;
    break;

  case WM_MBUTTONUP:
    {
      CPoint pt(msg->lParam);
      ClientToScreen(&pt);
      if (!PtInRect(&m_drag,pt))
        m_quitTracking = TRUE;
      result = TRUE;
    }
    break;
  }
  return result;
}

void DrawScrollMouseAnchorWnd::OnTimer(UINT_PTR id)
{
  ASSERT(id == ID_TIMER_TRACKING);

  CPoint ptNow;
  GetCursorPos(&ptNow);

  CRect rc;
  GetWindowRect(&rc);

  int cursor = 0;
  if ((m_anchorID == AFX_IDC_MOUSE_ORG_HV) || (m_anchorID == AFX_IDC_MOUSE_ORG_VERT))
  {
    if (ptNow.y < rc.top)
      cursor = AFX_IDC_MOUSE_PAN_N;
    else if (ptNow.y > rc.bottom)
      cursor = AFX_IDC_MOUSE_PAN_S;
  }
  if ((m_anchorID == AFX_IDC_MOUSE_ORG_HV) || (m_anchorID == AFX_IDC_MOUSE_ORG_HORZ))
  {
    if (ptNow.x < rc.left)
    {
      if (cursor == 0)
        cursor = AFX_IDC_MOUSE_PAN_W;
      else if (m_anchorID == AFX_IDC_MOUSE_ORG_HV)
        cursor--;
    }
    else if (ptNow.x > rc.right)
    {
      if (cursor == 0)
        cursor = AFX_IDC_MOUSE_PAN_E;
      else if (m_anchorID == AFX_IDC_MOUSE_ORG_HV)
        cursor++;
    }
  }

  if (m_quitTracking)
  {
    KillTimer(ID_TIMER_TRACKING);
    ReleaseCapture();
    SetCursor(NULL);
    DrawScrollArea* area = (DrawScrollArea*)GetOwner();
    DestroyWindow();
    delete area->m_anchorWindow;
    area->m_anchorWindow = NULL;
  }
  else if (cursor == 0)
    SetCursor(m_anchorCursor);
  else
  {
    HINSTANCE inst = AfxFindResourceHandle(MAKEINTRESOURCE(cursor),RT_GROUP_CURSOR);
    HICON hc = ::LoadCursor(inst,MAKEINTRESOURCE(cursor));
    ASSERT(hc != 0);
    SetCursor(hc);

    CSize distance;
    if (ptNow.x > rc.right)
      distance.cx = ptNow.x - rc.right;
    else if (ptNow.x < rc.left)
      distance.cx = ptNow.x - rc.left;
    else
      distance.cx = 0;

    if (ptNow.y > rc.bottom)
      distance.cy = ptNow.y - rc.bottom;
    else if (ptNow.y < rc.top)
      distance.cy = ptNow.y - rc.top;
    else
      distance.cy = 0;

    DrawScrollArea* area = (DrawScrollArea*)GetOwner();

    CSize sizeToScroll = area->GetWheelScrollDistance(distance,
      (m_anchorID == AFX_IDC_MOUSE_ORG_HV) || (m_anchorID == AFX_IDC_MOUSE_ORG_HORZ),
      (m_anchorID == AFX_IDC_MOUSE_ORG_HV) || (m_anchorID == AFX_IDC_MOUSE_ORG_VERT));

    area->OnScrollBy(sizeToScroll,TRUE);
    UpdateWindow();
    SetWindowPos(&CWnd::wndTop,m_anchor.x - m_size.cx/2,m_anchor.y - m_size.cy/2,0,0,
      SWP_NOACTIVATE|SWP_NOSIZE|SWP_SHOWWINDOW);
  }
}
